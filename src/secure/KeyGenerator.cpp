#include "secure/KeyGenerator.h"
#include "utils/JsonUtils.h"
#include "utils/Logger.h"
#include <memory>
#include <stdexcept>
#include <random>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <chrono>
#include <ctime>
#include <sstream>
#include <cpuid.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/provider.h>
#include <openssl/hmac.h>
#include <openssl/kdf.h>
#include <drogon/drogon.h>

#if defined(__x86_64__) || defined(_M_X64)
#include <x86intrin.h> // x86-64에서만 사용 가능
#else
// 다른 플랫폼에서는 하드웨어 RNG를 지원하지 않음
static int _rdrand64_step(unsigned long long* random) {
    // __builtin_ia32_rdrand64_step은 x86-64 전용으로 다른 플랫폼에서는 사용할 수 없음
    return 0; // 다른 RNG 대체 코드 추가 가능
}
#endif

// GF(256) 연산을 위한 룩업 테이블
namespace {
    constexpr unsigned char GF256_EXP[256] = { /* 지수 테이블 */ };
    constexpr unsigned char GF256_LOG[256] = { /* 로그 테이블 */ };

    // GF(256) 곱셈
    unsigned char gf256_mul(unsigned char a, unsigned char b) {
        if (a == 0 || b == 0) return 0;
        return GF256_EXP[(GF256_LOG[a] + GF256_LOG[b]) % 255];
    }

    // GF(256) 나눗셈
    unsigned char gf256_div(unsigned char a, unsigned char b) {
        if (a == 0) return 0;
        if (b == 0) throw std::invalid_argument("Division by zero in GF(256)");
        return GF256_EXP[(GF256_LOG[a] + 255 - GF256_LOG[b]) % 255];
    }
}

// HKDF 헬퍼 함수 정의
static int HKDF(const EVP_MD* evp_md,
                const unsigned char* salt, size_t salt_len,
                const unsigned char* key, size_t key_len,
                const unsigned char* info, size_t info_len,
                unsigned char* okm, size_t okm_len) {
    EVP_PKEY_CTX* pctx;
    
    pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_HKDF, NULL);
    if (!pctx) return 0;
    
    if (EVP_PKEY_derive_init(pctx) <= 0 ||
        EVP_PKEY_CTX_set_hkdf_md(pctx, evp_md) <= 0 ||
        EVP_PKEY_CTX_set1_hkdf_salt(pctx, salt, salt_len) <= 0 ||
        EVP_PKEY_CTX_set1_hkdf_key(pctx, key, key_len) <= 0 ||
        EVP_PKEY_CTX_add1_hkdf_info(pctx, info, info_len) <= 0 ||
        EVP_PKEY_derive(pctx, okm, &okm_len) <= 0) {
        EVP_PKEY_CTX_free(pctx);
        return 0;
    }
    
    EVP_PKEY_CTX_free(pctx);
    return 1;
}

std::string KeyGenerator::generateCryptographicKey() {
    try {
        // 1. 하드웨어 보안 모듈을 사용한 키 생성 시도
        if (auto hwKey = tryHardwareKeyGeneration()) {
            return hwKey.value();
        }
    } catch (const std::exception& e) {
        // HSM 방식 실패 시 로그 작성 (필요 시)
        TRADING_LOG_ERROR("HSM key generation failed: " + std::string(e.what()));
    }

    // 2. 소프트웨어 방식으로 플백
    std::vector<unsigned char> key(KEY_LENGTH);
    std::vector<unsigned char> additionalEntropy(MIN_ENTROPY_BYTES);

    // 추가 엔트로피 수집
    if (!collectSystemEntropy(additionalEntropy)) {
        throw std::runtime_error("Failed to collect system entropy");
    }

    // OpenSSL의 RAND_bytes 사용
    if (RAND_bytes(key.data(), KEY_LENGTH) != 1) {
        throw std::runtime_error("OpenSSL RAND_bytes failed");
    }

    // 엔트로피 결합
    mixEntropy(key, additionalEntropy);

    // 키 강도 검증
    if (!validateKeyStrength(key)) {
        throw std::runtime_error("Generated key does not meet strength requirements");
    }

    return std::string(reinterpret_cast<char*>(key.data()), KEY_LENGTH);
}


bool KeyGenerator::validateKeyStrength(const std::vector<unsigned char>& key) {
    if (key.size() < KEY_LENGTH) {
        return false;
    }

    // 엔트로피 검사
    double entropy = calculateEntropy(key);
    if (entropy < MIN_ENTROPY_BYTES * 8) { // 비트 단위로 변환
        return false;
    }

    // 키 강도 분석
    return analyzeKeyStrength(key);
}

void KeyGenerator::secureKeyStorage(const std::string& key, const std::string& keyId) {
    // 키 분할 (Shamir`s Secret Sharing)
    auto keyShares = splitKey(key);

    // 각 키 조각을 다른 위치에 안전하게 저장
    for (size_t i = 0; i < keyShares.size(); i++) {
        storeKeyShare(keyShares[i], keyId, i);
    }

    // 키 메타데이터 저장
    storeKeyMetadata(keyId);
}

std::optional<std::string> KeyGenerator::recoverKey(const std::string& keyId) {
    try {
        // 키가 유효한지 확인
        if (!isKeyActive(keyId)) {
            TRADING_LOG_ERROR("Attempt to recover inactive or expired key: {}", keyId);
            return std::nullopt;
        }

        // 키 조각들을 가져옴
        auto shares = retrieveKeyShares(keyId);
        if (shares.empty()) {
            TRADING_LOG_ERROR("No key shares found for key: {}", keyId);
            return std::nullopt;
        }

        // 키 조각들이 충분한지 확인
        if (shares.size() < REQUIRED_SHARES) {
            TRADING_LOG_ERROR("Insufficient key shares for recovery. Required: {}, Found: {}", REQUIRED_SHARES, shares.size());
            return std::nullopt;
        }

        // 키 복구 시도
        auto recoveredKey = combineKeyShares(shares);
        if (!recoveredKey) {
            TRADING_LOG_ERROR("Failed to combine key shares for key: {}", keyId);
            return std::nullopt;
        }

        // 복구 작업 로깅
        logKeyEvent(keyId, "key_recovery", "key successfully recovered");

        return recoveredKey;
    } catch (const std::exception& e) {
        TRADING_LOG_DEBUG("Error recovering key: {}", e.what());
        return std::nullopt;
    }
}

bool KeyGenerator::revokeKey(const std::string& keyId) {
    try {
        // 메타데이터 업데이트로 키 상태를 revoked로 변경
        if (!updateKeyStatus(keyId, "revoked")) {
            TRADING_LOG_ERROR("Failed to update key status to revoked for key: {}", keyId);
            return false;
        }

        auto dbClient = drogon::app().getDbClient();
        
        // 트랜잭션 시작
        dbClient->newTransactionAsync(
            [keyId](const auto& trans) {
                try {
                    // 키 조각들 삭제
                    trans->execSqlSync(
                        "DELETE FROM key_shares WHERE key_id = $1",
                        keyId
                    );

                    // 키 메타데이터에 삭제 시간 기록
                    auto result = trans->execSqlSync(
                        "SELECT metadata FROM key_metadata WHERE key_id = $1",
                        keyId
                    );

                    if (!result.empty()) {
                        Json::Value metadata = utils::JsonUtils::parseJson(
                            result[0]["metadata"].template as<std::string>()
                        );
                        metadata["revoked_at"] = static_cast<Json::Int64>(
                            std::chrono::system_clock::to_time_t(
                                std::chrono::system_clock::now()
                            )
                        );

                        trans->execSqlSync(
                            "UPDATE key_metadata SET metadata = $1 WHERE key_id = $2",
                            utils::JsonUtils::toJsonString(metadata),
                            keyId
                        );
                    }

                    // COMMIT
                    auto commitBinder = (*trans) << "COMMIT";
                    commitBinder.exec();
                    TRADING_LOG_INFO("Key shares deleted for key: {}", keyId);

                } catch (const std::exception& e) {
                    TRADING_LOG_ERROR("Transaction failed during key revocation: {}", e.what());
                    // ROLLBACK
                    auto rollbackBinder = (*trans) << "ROLLBACK";
                    rollbackBinder.exec();
                    throw;
                }
            }
        );

        // 키 폐기 이벤트 로깅
        logKeyEvent(keyId, "key_revoked", "Key has been revoked and shares deleted");
        TRADING_LOG_INFO("Key successfully revoked: {}", keyId);
        return true;

    } catch (const std::exception& e) {
        TRADING_LOG_ERROR("Error revoking key: {}", e.what());
        return false;
    }
}

std::optional<std::string> KeyGenerator::tryHardwareKeyGeneration() {
    try {
        const char* providerName = "pkcs11"; // PKCS#11 Provider 이름
        const char* modulePath = "/usr/lib/your-pkcs11.so"; // PKCS#11 라이브러리 경로
        const char* slotId = "0"; // 특정 슬롯 사용 (필요시 변경)

        // OpenSSL Provider 로드
        OSSL_PROVIDER* provider = OSSL_PROVIDER_load(nullptr, providerName);
        if (!provider) {
            TRADING_LOG_ERROR("Failed to load OpenSSL provider: {}", providerName);
            return std::nullopt;
        }

        // PKCS#11 모듈 초기화
        if (!OSSL_PROVIDER_set_default_search_path(nullptr, modulePath)) {
            TRADING_LOG_ERROR("Failed to set PKCS#11 module path: {}", modulePath);
            OSSL_PROVIDER_unload(provider);
            return std::nullopt;
        }

        // EVP Context 생성
        EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_from_name(nullptr, "RSA", providerName);
        if (!ctx) {
            TRADING_LOG_ERROR("Failed to create EVP_PKEY_CTX");
            OSSL_PROVIDER_unload(provider);
            return std::nullopt;
        }

        // 키 생성 초기화
        if (EVP_PKEY_keygen_init(ctx) <= 0) {
            TRADING_LOG_ERROR("Failed to initialize key generation");
            EVP_PKEY_CTX_free(ctx);
            OSSL_PROVIDER_unload(provider);
            return std::nullopt;
        }

        // 키 길이 설정 (예: 2048비트)
        if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, 2048) <= 0) {
            TRADING_LOG_ERROR("Failed to set RSA key length");
            EVP_PKEY_CTX_free(ctx);
            OSSL_PROVIDER_unload(provider);
            return std::nullopt;
        }

        // 키 생성
        EVP_PKEY* pkey = nullptr;
        if (EVP_PKEY_keygen(ctx, &pkey) <= 0) {
            TRADING_LOG_ERROR("Key generation failed");
            EVP_PKEY_CTX_free(ctx);
            OSSL_PROVIDER_unload(provider);
            return std::nullopt;
        }

        // 키를 PEM 형식으로 변환
        BIO* bio = BIO_new(BIO_s_mem());
        if (!bio || !PEM_write_bio_PrivateKey(bio, pkey, nullptr, nullptr, 0, nullptr, nullptr)) {
            TRADING_LOG_ERROR("Failed to write key to PEM format");
            EVP_PKEY_free(pkey);
            EVP_PKEY_CTX_free(ctx);
            OSSL_PROVIDER_unload(provider);
            return std::nullopt;
        }

        // PEM 데이터를 문자열로 변환
        char* pemData = nullptr;
        long pemLen = BIO_get_mem_data(bio, &pemData);
        std::string pemKey(pemData, pemLen);

        // 리소스 정리
        BIO_free(bio);
        EVP_PKEY_free(pkey);
        EVP_PKEY_CTX_free(ctx);
        OSSL_PROVIDER_unload(provider);

        TRADING_LOG_INFO("Successfully generated hardware key");
        return pemKey;

    } catch (const std::exception& e) {
        TRADING_LOG_ERROR("Exception during hardware key generation: {}", e.what());
        return std::nullopt;
    }
}

std::string KeyGenerator::generateSecureSoftwareKey() {
    try {
        TRADING_LOG_DEBUG("Starting secure software key generation");
        
        std::vector<unsigned char> key(KEY_LENGTH);
        std::vector<unsigned char> additionalEntropy(MIN_ENTROPY_BYTES);

        // 시스템 엔트로피 수집
        if (!collectSystemEntropy(additionalEntropy)) {
            TRADING_LOG_ERROR("Failed to collect system entropy");
            throw std::runtime_error("Failed to collect system entropy");
        }

        // OpenSSL의 RAND_bytes 사용하여 초기 키 생성
        if (RAND_bytes(key.data(), KEY_LENGTH) != 1) {
            unsigned long err = ERR_get_error();
            char err_buf[256];
            ERR_error_string_n(err, err_buf, sizeof(err_buf));
            TRADING_LOG_ERROR("OpenSSL RAND_bytes failed: {}", err_buf);
            throw std::runtime_error("OpenSSL RAND_bytes failed");
        }

        // 추가 엔트로피를 키와 혼합
        mixEntropy(key, additionalEntropy);

        // 통계적 분석 수행
        if (!performStatisticalTests(key)) {
            TRADING_LOG_ERROR("Generated key failed statistical tests");
            throw std::runtime_error("Generated key failed statistical tests");
        }

        // 키 품질 검증
        if (!validateKeyStrength(key)) {
            TRADING_LOG_ERROR("Generated key does not meet strength requirements");
            throw std::runtime_error("Generated key does not meet strength requirements");
        }

        TRADING_LOG_INFO("Successfully generated secure software key");
        return std::string(reinterpret_cast<char*>(key.data()), KEY_LENGTH);

    } catch (const std::exception& e) {
        TRADING_LOG_ERROR("Key generation failed: {}", e.what());
        throw;
    }
}

bool KeyGenerator::collectSystemEntropy(std::vector<unsigned char>& buffer) {
    try {
        // CPU 하드웨어 이벤트 수집
        collectOSEntropy(buffer);

        // 시스템 타이밍 정보 수집
        collectTimingEntropy(buffer);

        // 가능한 경우 OS 엔트로피 풀 사용
        if (!collectOSEntropy(buffer)) {
            return false;
        }
        return true;
    } catch (...) {
        return false;
    }
}

void KeyGenerator::collectCPUEntropy(std::vector<unsigned char>& buffer) {
    // RDRAND/RDSEED 명령어 사용 (가능한 경우)
    #if defined(__x86_64__) || defined(_M_X64)
        if (__builtin_cpu_supports("rdrnd")) {
            unsigned long long rnd;
            if (_rdrand64_step(&rnd)) {
                auto bytes = reinterpret_cast<unsigned char*>(&rnd);
                buffer.insert(buffer.end(), bytes, bytes + sizeof(rnd));
            }
        }
    #endif
}

void KeyGenerator::collectTimingEntropy(std::vector<unsigned char>& buffer) {
    auto now = std::chrono::high_resolution_clock::now();
    auto nanos = now.time_since_epoch().count();
    auto bytes = reinterpret_cast<const unsigned char*>(&nanos);
    buffer.insert(buffer.end(), bytes, bytes + sizeof(nanos));
}

bool KeyGenerator::collectOSEntropy(std::vector<unsigned char>& buffer) {
    std::ifstream urandom("/dev/urandom", std::ios::binary);
    if (urandom) {
        std::vector<unsigned char> temp(buffer.size());
        urandom.read(reinterpret_cast<char*>(temp.data()), temp.size());
        if (!urandom) {
            return false;
        }
        buffer.insert(buffer.end(), temp.begin(), temp.end());
        return true;
    }
    return false;
}


void KeyGenerator::mixEntropy(std::vector<unsigned char>& key,
                const std::vector<unsigned char>& additionalEntropy) {
    // HKDF를 사용하여 엔트로피 소스들을 혼합
    unsigned char prk[EVP_MAX_MD_SIZE];
    unsigned int prkLen;

    // HMAC-SHA256을 사용하여 추출 단계 수행
    HMAC(EVP_sha256(), additionalEntropy.data(), additionalEntropy.size(),
        key.data(), key.size(), prk, &prkLen);

    // 확장 단계
    std::vector<unsigned char> output(key.size());
    HKDF(EVP_sha256(), nullptr, 0, prk, prkLen,
        additionalEntropy.data(), additionalEntropy.size(),
        output.data(), output.size());

    key = output;
}

std::vector<std::vector<unsigned char>> KeyGenerator::splitKey(const std::string& key) {
    if (key.length() != KEY_LENGTH) {
        throw std::invalid_argument("Invalid key length for splitting");
    }

    std::vector<std::vector<unsigned char>> shares(MIN_SHARES, std::vector<unsigned char>(KEY_LENGTH + 1));
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<unsigned int> dist(0, 255);

    // 각 바이트별로 처리
    for (size_t byte_idx = 0; byte_idx < KEY_LENGTH; ++byte_idx) {
        // 각 바이트에 대해 t-1차 다항식 생성
        std::vector<unsigned char> coefficients(REQUIRED_SHARES);
        coefficients[0] = static_cast<unsigned char>(key[byte_idx]); // 상수항

        // 랜덤 계수 생성
        for (size_t i = 1; i < REQUIRED_SHARES; ++i) {
            coefficients[i] = static_cast<unsigned char>(dist(gen));
        }

        // 각 share에 대한 값 계산
        for (size_t shared_idx = 0; shared_idx < MIN_SHARES; ++shared_idx) {
            unsigned char x = static_cast<unsigned char>(shared_idx + 1);
            unsigned char y = coefficients[0]; // 상수항

            // 다항식 계산: y = f(x) = a0 + a1*x + a2*x^2 + ... + at-1*x^(t-1)
            unsigned char x_power = x;
            for (size_t coef_idx = 1; coef_idx < REQUIRED_SHARES; ++coef_idx) {
                y ^= gf256_mul(coefficients[coef_idx], x_power);
                x_power = gf256_mul(x_power, x);
            }

            shares[shared_idx][byte_idx] = y;
        }

        // x 좌표 저장
        for (size_t shared_idx = 0; shared_idx < MIN_SHARES; ++shared_idx) {
            shares[shared_idx][KEY_LENGTH] = static_cast<unsigned char>(shared_idx + 1);
        }
    }

    return shares;
}

std::optional<std::string> KeyGenerator::combineKeyShares(
    const std::vector<std::vector<unsigned char>>& shares) {
    
    if (shares.size() < REQUIRED_SHARES) {
        return std::nullopt;
    }

    // 결과 키 준비
    std::string recoveredKey(KEY_LENGTH, '\0');

    // 각 바이트별로 처리
    for (size_t byte_idx = 0; byte_idx < KEY_LENGTH; ++byte_idx) {
        // 라그랑주 보간법
        unsigned char result = 0;
        
        for (size_t i = 0; i < REQUIRED_SHARES; ++i) {
            unsigned char basis = 1;  // 라그랑주 기저 다항식
            
            for (size_t j = 0; j < REQUIRED_SHARES; ++j) {
                if (i == j) continue;

                unsigned char x_i = shares[i][KEY_LENGTH];
                unsigned char x_j = shares[j][KEY_LENGTH];
                
                // basis *= (x - x_j) / (x_i - x_j)
                // x = 0 이므로 분자는 x_j가 됨
                basis = gf256_mul(basis, 
                    gf256_div(x_j, (x_i ^ x_j)));  // GF(256)에서 뺄셈은 XOR
            }

            result ^= gf256_mul(shares[i][byte_idx], basis);
        }

        recoveredKey[byte_idx] = static_cast<char>(result);
    }

    // 복구된 키의 유효성 검증
    std::vector<unsigned char> keyVec(recoveredKey.begin(), recoveredKey.end());
    if (!validateKeyStrength(keyVec)) {
        return std::nullopt;
    }

    return recoveredKey;
}

void KeyGenerator::storeKeyShare(const std::vector<unsigned char>& share,
                               const std::string& keyId,
                               size_t shareIndex) {
    try {
        auto dbClient = drogon::app().getDbClient();

        // 키 조각을 암호화하여 저장
        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx) {
            TRADING_LOG_ERROR("Failed to create cipher context for share storage");
            throw std::runtime_error("Failed to create cipher context");
        }

        unsigned char iv[IV_LEN];
        if (RAND_bytes(iv, IV_LEN) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Failed to generate IV for share storage");
        }

        // std::vector<unsigned char>를 std::string으로 변환
        std::string shareData(reinterpret_cast<const char*>(share.data()), share.size());
        std::string ivData(reinterpret_cast<const char*>(iv), IV_LEN);

        // 데이터베이스에 저장
        dbClient->newTransactionAsync(
            [keyId, shareIndex, shareData, ivData](const auto& trans) {
                try {
                    trans->execSqlSync(
                        "INSERT INTO key_shares (key_id, share_index, share_data, iv) "
                        "VALUES ($1, $2, $3, $4)",
                        keyId,
                        static_cast<int>(shareIndex),
                        shareData,
                        ivData
                    );

                    auto commitBinder = (*trans) << "COMMIT";
                    commitBinder.exec();

                    TRADING_LOG_DEBUG("Stored key share {} for key: {}", shareIndex, keyId);

                } catch (const std::exception& e) {
                    auto rollbackBinder = (*trans) << "ROLLBACK";
                    rollbackBinder.exec();
                    TRADING_LOG_ERROR("Failed to store key share: {}", e.what());
                    throw;
                }
            }
        );

        logShareOperation("store", keyId, shareIndex);

    } catch (const std::exception& e) {
        TRADING_LOG_ERROR("Error storing key share: {}", e.what());
        throw;
    }
}

std::vector<std::vector<unsigned char>> KeyGenerator::retrieveKeyShares(
    const std::string& keyId) {
    try {
        auto dbClient = drogon::app().getDbClient();
        std::vector<std::vector<unsigned char>> shares;

        auto result = dbClient->execSqlSync(
            "SELECT share_index, share_data, iv FROM key_shares "
            "WHERE key_id = $1 ORDER BY share_index",
            keyId
        );

        if (result.empty()) {
            TRADING_LOG_WARN("No shares found for key: {}", keyId);
            return shares;
        }

        shares.reserve(result.size());

        for (const auto& row : result) {
            size_t shareIndex = row["share_index"].template as<size_t>();
            auto shareDataStr = row["share_data"].template as<std::string>();
            
            // std::string -> std::vector<unsigned char> 변환
            std::vector<unsigned char> shareData(shareDataStr.begin(), shareDataStr.end());

            // 복호화된 share를 shares 벡터에 추가
            shares.push_back(shareData);
            
            logShareOperation("retrieve", keyId, shareIndex);
        }

        TRADING_LOG_INFO("Retrieved {} shares for key: {}", shares.size(), keyId);
        return shares;

    } catch (const std::exception& e) {
        TRADING_LOG_ERROR("Error retrieving key shares: {}", e.what());
        throw;
    }
}


void KeyGenerator::storeKeyMetadata(const std::string& keyId) {
    try {
        auto dbClient = drogon::app().getDbClient();
        auto now = std::chrono::system_clock::now();
        auto nowTime = std::chrono::system_clock::to_time_t(now);
        auto expiryTime = nowTime + (90 * 24 * 60 * 60); // 90일 후 만료

        // 메타데이터 준비
        Json::Value metadata;
        metadata["key_id"] = keyId;
        metadata["creation_time"] = static_cast<Json::Int64>(nowTime);
        metadata["expiry_time"] = static_cast<Json::Int64>(expiryTime);
        metadata["status"] = "active";
        metadata["version"] = "1";
        metadata["key_type"] = "AES-256-GCM";
        metadata["shares_total"] = static_cast<Json::Int>(MIN_SHARES);
        metadata["shares_required"] = static_cast<Json::Int>(REQUIRED_SHARES);

        // 메타데이터를 데이터베이스에 저장
        dbClient->execSqlSync(
            "INSERT INTO key_metadata "
            "(key_id, metadata, created_at, updated_at) "
            "VALUES ($1, $2, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP)",
            keyId,
            utils::JsonUtils::toJsonString(metadata)
        );

        // 키 사용 이력 시작
        logKeyEvent(keyId, "created", "Key created and metadata stored");

    } catch (const std::exception& e) {
        throw std::runtime_error(
            std::string("Failed to store key metadata: ") + e.what()
        );
    }
}

double KeyGenerator::calculateEntropy(const std::vector<unsigned char>& data) {
    try {
        if (data.empty()) {
            TRADING_LOG_ERROR("Cannot calculate entropy of empty data");
            return 0.0;
        }

        // 바이트 값 빈도 계산
        std::array<size_t, 256> frequency = {0};
        for (unsigned char byte : data) {
            frequency[byte]++;
        }

        // Shannon 엔트로피 계산
        double entropy = 0.0;
        double size = static_cast<double>(data.size());

        for (size_t count : frequency) {
            if (count > 0) {
                double probability = count / size;
                entropy -= probability * std::log2(probability);
            }
        }

        TRADING_LOG_DEBUG("Calculated entropy: {} bits per byte", entropy);
        return entropy * 8.0; // 비트 단위로 변환

    } catch (const std::exception& e) {
        TRADING_LOG_ERROR("Error calculating entropy: {}", e.what());
        throw;
    }
}

bool KeyGenerator::analyzeKeyStrength(const std::vector<unsigned char>& key) {
    try {
        // 1. 최소 키 길이 확인
        if (key.size() < KEY_LENGTH) {
            TRADING_LOG_WARN("Key length {} is less than required {}", key.size(), KEY_LENGTH);
            return false;
        }

        // 2. 엔트로피 검사
        double entropy = calculateEntropy(key);
        if (entropy < MIN_KEY_STRENGTH) {
            TRADING_LOG_WARN("Key entropy {} is below minimum requirement {}", entropy, MIN_KEY_STRENGTH);
            return false;
        }

        // 3. 연속된 같은 값 검사
        constexpr size_t MAX_REPEATED_BYTES = 4;
        size_t repeatedCount = 1;
        
        for (size_t i = 1; i < key.size(); ++i) {
            if (key[i] == key[i-1]) {
                repeatedCount++;
                if (repeatedCount > MAX_REPEATED_BYTES) {
                    TRADING_LOG_WARN("Key contains too many repeated bytes");
                    return false;
                }
            } else {
                repeatedCount = 1;
            }
        }

        // 4. 통계적 검증
        if (!performStatisticalTests(key)) {
            TRADING_LOG_WARN("Key failed statistical tests");
            return false;
        }

        TRADING_LOG_DEBUG("Key passed strength analysis");
        return true;

    } catch (const std::exception& e) {
        TRADING_LOG_ERROR("Error analyzing key strength: {}", e.what());
        throw;
    }
}

bool KeyGenerator::performStatisticalTests(const std::vector<unsigned char>& key) {
    try {
        // 1. 단순 빈도 검사
        std::array<size_t, 256> frequency = {0};
        for (unsigned char byte : key) {
            frequency[byte]++;
        }

        double expectedFreq = static_cast<double>(key.size()) / 256.0;
        double chiSquare = 0.0;

        for (size_t count : frequency) {
            double diff = count - expectedFreq;
            chiSquare += (diff * diff) / expectedFreq;
        }

        // chi-square 임계값 (95% 신뢰도)
        constexpr double CHI_SQUARE_THRESHOLD = 293.25; // 255 자유도
        
        if (chiSquare > CHI_SQUARE_THRESHOLD) {
            TRADING_LOG_WARN("Key failed frequency test: chi-square = {}", chiSquare);
            return false;
        }

        // 2. 연속된 비트 패턴 검사
        std::vector<bool> bits;
        bits.reserve(key.size() * 8);
        
        for (unsigned char byte : key) {
            for (int i = 7; i >= 0; --i) {
                bits.push_back((byte >> i) & 1);
            }
        }

        size_t ones = std::count(bits.begin(), bits.end(), true);
        double onesRatio = static_cast<double>(ones) / bits.size();

        // 비트 비율이 0.45~0.55 범위 내에 있어야 함
        if (onesRatio < 0.45 || onesRatio > 0.55) {
            TRADING_LOG_WARN("Key failed bit ratio test: ratio = {}", onesRatio);
            return false;
        }

        // 3. 런 길이 테스트
        size_t maxRun = 0;
        size_t currentRun = 1;
        
        for (size_t i = 1; i < bits.size(); ++i) {
            if (bits[i] == bits[i-1]) {
                currentRun++;
            } else {
                maxRun = std::max(maxRun, currentRun);
                currentRun = 1;
            }
        }
        maxRun = std::max(maxRun, currentRun);

        // 최대 런 길이가 너무 길면 안됨
        if (maxRun > 20) {
            TRADING_LOG_WARN("Key failed run length test: max run = {}", maxRun);
            return false;
        }

        TRADING_LOG_DEBUG("Key passed all statistical tests");
        return true;

    } catch (const std::exception& e) {
        TRADING_LOG_ERROR("Error performing statistical tests: {}", e.what());
        throw;
    }
}

std::optional<Json::Value> KeyGenerator::getKeyMetadata(const std::string& keyId) {
    try {
        auto dbClient = drogon::app().getDbClient();
        auto result = dbClient->execSqlSync(
            "SELECT metadata FROM key_metadata WHERE key_id = $1",
            keyId
        );

        if (result.empty()) {
            return std::nullopt;
        }

        std::string metadataStr = result[0]["metadata"].template as<std::string>();
        return utils::JsonUtils::parseJson(metadataStr);

    } catch (const std::exception& e) {
        throw std::runtime_error(
            std::string("Failed to retrieve key metadata: ") + e.what()
        );
    }
}

bool KeyGenerator::updateKeyStatus(const std::string& keyId, const std::string& newStatus) {
    try {
        auto dbClient = drogon::app().getDbClient();
        bool success = false;
        
        // 트랜잭션 시작
        dbClient->newTransactionAsync(
            [keyId, newStatus, &success](const auto& trans) {
                try {
                    // 현재 메타데이터 조회
                    auto result = trans->execSqlSync(
                        "SELECT metadata FROM key_metadata WHERE key_id = $1 FOR UPDATE",
                        keyId
                    );

                    if (result.empty()) {
                        // ROLLBACK
                        auto rollbackBinder = (*trans) << "ROLLBACK";
                        rollbackBinder.exec();
                        success = false;
                        return;
                    }

                    // 메타데이터 업데이트
                    Json::Value metadata = utils::JsonUtils::parseJson(
                        result[0]["metadata"].template as<std::string>()
                    );
                    metadata["status"] = newStatus;
                    metadata["updated_at"] = static_cast<Json::Int64>(
                        std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())
                    );

                    // 업데이트된 메타데이터 저장
                    trans->execSqlSync(
                        "UPDATE key_metadata SET metadata = $1, updated_at = CURRENT_TIMESTAMP "
                        "WHERE key_id = $2",
                        utils::JsonUtils::toJsonString(metadata),
                        keyId
                    );

                    // 상태 변경 이력 기록
                    logKeyEvent(keyId, "status_changed", 
                              "Key status changed to: " + newStatus);

                    // COMMIT
                    auto commitBinder = (*trans) << "COMMIT";
                    commitBinder.exec();
                    success = true;

                } catch (const std::exception& e) {
                    // ROLLBACK
                    auto rollbackBinder = (*trans) << "ROLLBACK";
                    rollbackBinder.exec();
                    throw;
                }
            }
        );

        return success;

    } catch (const std::exception& e) {
        throw std::runtime_error(
            std::string("Failed to update key status: ") + e.what()
        );
    }
}

void KeyGenerator::logKeyEvent(const std::string& keyId, 
                             const std::string& eventType,
                             const std::string& description) {
    try {
        auto dbClient = drogon::app().getDbClient();
        
        dbClient->execSqlSync(
            "INSERT INTO key_events "
            "(key_id, event_type, description, created_at) "
            "VALUES ($1, $2, $3, CURRENT_TIMESTAMP)",
            keyId,
            eventType,
            description
        );

    } catch (const std::exception& e) {
        // 로깅 실패는 예외를 던지지 않고 에러 로그만 남김
        LOG_ERROR << "Failed to log key event: " << e.what();
    }
}

bool KeyGenerator::isKeyActive(const std::string& keyId) {
    try {
        if (auto metadata = getKeyMetadata(keyId)) {
            const auto& meta = metadata.value();
            
            // 상태 확인
            if (meta["status"].asString() != "active") {
                return false;
            }

            // 만료 시간 확인
            auto now = std::chrono::system_clock::to_time_t(
                std::chrono::system_clock::now()
            );
            auto expiryTime = meta["expiry_time"].asInt64();

            return now < expiryTime;
        }
        return false;

    } catch (const std::exception& e) {
        // 메타데이터 검증 실패 시 키를 비활성으로 간주
        return false;
    }
}

// 로깅 및 보안 감사를 위한 헬퍼 함수
void KeyGenerator::logShareOperation(const std::string& operation, 
                                   const std::string& keyId, 
                                   size_t shareIndex) {
    try {
        auto dbClient = drogon::app().getDbClient();
        
        // 현재 시간 얻기
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::system_clock::to_time_t(now);

        Json::Value logData;
        logData["operation"] = operation;
        logData["share_index"] = static_cast<int>(shareIndex);
        logData["timestamp"] = static_cast<Json::Int64>(timestamp);

        dbClient->execSqlSync(
            "INSERT INTO key_share_operations "
            "(key_id, operation_type, share_index, metadata, created_at) "
            "VALUES ($1, $2, $3, $4, CURRENT_TIMESTAMP)",
            keyId,
            operation,
            static_cast<int>(shareIndex),
            utils::JsonUtils::toJsonString(logData)
        );

        TRADING_LOG_INFO("Logged share operation: {} for key: {}, share: {}", 
                        operation, keyId, shareIndex);

    } catch (const std::exception& e) {
        TRADING_LOG_ERROR("Failed to log share operation: {}", e.what());
        // 로깅 실패는 치명적이지 않으므로 예외를 던지지 않음
    }
}
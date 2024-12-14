#include "secure/EncryptionManager.h"
#include <cstdlib>
#include <cstring>
#include <vector>
#include <stdexcept>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <openssl/crypto.h>
#include <openssl/core_names.h>
#include <openssl/params.h>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <ctime>
#include <uuid/uuid.h>
#include <drogon/drogon.h>
#include "utils/JsonUtils.h"

EncryptionManager::EncryptionManager() {
    const char* envKey = std::getenv("ENCRYPTION_KEY");
    if(!envKey) {
        throw std::runtime_error("ENCRYPTION_KEY not set in environment variables");
    }
    std::string rawKey(envKey);
    if (rawKey.size() != KEY_LEN) {
        throw std::runtime_error("ENCRYPTION_KEY must be 32 bytes for AES-256");
    }
    key_ = rawKey;
}

EncryptionManager::~EncryptionManager() {}

std::string EncryptionManager::encrypt(const std::string &plaintext) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if(!ctx) throw std::runtime_error("Failed to create EVP_CIPHER_CTX");

    if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL)) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_EncryptInit_ex failed");
    }

    unsigned char iv[IV_LEN];
    if(1 != RAND_bytes(iv, IV_LEN)) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("IV generation failed");
    }

    if(1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, IV_LEN, NULL)) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("IV length set failed");
    }

    if(1 != EVP_EncryptInit_ex(ctx, NULL, NULL,
                               reinterpret_cast<const unsigned char*>(key_.data()),
                               iv)) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_EncryptInit_ex key/iv failed");
    }

    int outlen;
    std::vector<unsigned char> outBuf(plaintext.size() + 16); // GCM may not expand much
    if(1 != EVP_EncryptUpdate(ctx, outBuf.data(), &outlen,
                              reinterpret_cast<const unsigned char*>(plaintext.data()),
                              (int)plaintext.size())) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_EncryptUpdate failed");
    }
    int ciphertext_len = outlen;

    // Finalize
    if(1 != EVP_EncryptFinal_ex(ctx, outBuf.data() + outlen, &outlen)) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_EncryptFinal_ex failed");
    }
    ciphertext_len += outlen;

    unsigned char tag[TAG_LEN];
    if(1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, TAG_LEN, tag)) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Get tag failed");
    }

    EVP_CIPHER_CTX_free(ctx);

    // 최종: IV + ciphertext + tag 합쳐서 base64 인코딩
    std::vector<unsigned char> finalData;
    finalData.reserve(IV_LEN + ciphertext_len + TAG_LEN);
    finalData.insert(finalData.end(), iv, iv + IV_LEN);
    finalData.insert(finalData.end(), outBuf.begin(), outBuf.begin() + ciphertext_len);
    finalData.insert(finalData.end(), tag, tag + TAG_LEN);

    return base64Encode(finalData.data(), finalData.size());
}

std::string EncryptionManager::decrypt(const std::string &ciphertextBase64) {
    std::vector<unsigned char> encData = base64Decode(ciphertextBase64);
    if(encData.size() < IV_LEN + TAG_LEN) {
        throw std::runtime_error("Invalid ciphertext format");
    }

    unsigned char iv[IV_LEN];
    std::memcpy(iv, encData.data(), IV_LEN);

    size_t dataSize = encData.size() - IV_LEN - TAG_LEN;
    std::vector<unsigned char> ciphertext(dataSize);
    std::memcpy(ciphertext.data(), encData.data() + IV_LEN, dataSize);

    unsigned char tag[TAG_LEN];
    std::memcpy(tag, encData.data() + IV_LEN + dataSize, TAG_LEN);

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if(!ctx) throw std::runtime_error("Failed to create EVP_CIPHER_CTX decrypt");

    if(1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL)) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_DecryptInit_ex failed");
    }

    if(1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, IV_LEN, NULL)) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Set IV length failed");
    }

    if(1 != EVP_DecryptInit_ex(ctx, NULL, NULL,
                               reinterpret_cast<const unsigned char*>(key_.data()),
                               iv)) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_DecryptInit_ex with key/iv failed");
    }

    int outlen;
    std::vector<unsigned char> outBuf(dataSize);
    if(1 != EVP_DecryptUpdate(ctx, outBuf.data(), &outlen,
                              ciphertext.data(), (int)dataSize)) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_DecryptUpdate failed");
    }
    int plaintext_len = outlen;

    if(1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, TAG_LEN, tag)) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Set tag failed");
    }

    int ret = EVP_DecryptFinal_ex(ctx, outBuf.data() + outlen, &outlen);
    EVP_CIPHER_CTX_free(ctx);

    if(ret <= 0) {
        throw std::runtime_error("Decryption failed (invalid tag)");
    }
    plaintext_len += outlen;

    return std::string((char*)outBuf.data(), plaintext_len);
}

std::string EncryptionManager::base64Encode(const unsigned char* data, size_t len) {
    // Base64 인코딩 후 길이: 4 * ceil(len/3)
    int encodedLen = 4 * ((int)len + 2) / 3;
    std::vector<unsigned char> outBuf(encodedLen + 1); // +1 for null terminator
    int ret = EVP_EncodeBlock(outBuf.data(), data, (int)len);
    if (ret < 0) {
        throw std::runtime_error("EVP_EncodeBlock failed");
    }
    // EVP_EncodeBlock는 마지막에 '\0'을 붙이므로 별도 처리 불필요
    return std::string((char*)outBuf.data());
}

std::vector<unsigned char> EncryptionManager::base64Decode(const std::string& b64) {
    // base64 문자열 길이를 4의 배수로 맞추기 위해 '=' 패딩을 확인
    std::string paddedB64 = b64;
    // base64 길이가 4의 배수가 아니면 '='를 추가
    size_t remainder = paddedB64.size() % 4;
    if (remainder != 0) {
        paddedB64.append(4 - remainder, '=');
    }

    // 디코딩용 버퍼: 디코딩된 바이트 수는 인코딩 길이보다 작거나 같음
    std::vector<unsigned char> outBuf(paddedB64.size());
    int ret = EVP_DecodeBlock(outBuf.data(),
                              (const unsigned char*)paddedB64.data(),
                              (int)paddedB64.size());
    if (ret < 0) {
        throw std::runtime_error("EVP_DecodeBlock failed");
    }

    // ret는 디코딩된 바이트 수, 하지만 '=' 패딩으로 인한 실제 데이터 길이 조정 필요
    // 원래 base64 끝의 '=' 개수 세기
    size_t eqCount = 0;
    for (int i = (int)b64.size() - 1; i >= 0 && b64[i] == '='; i--) {
        eqCount++;
    }

    // '=' 개수에 따라 실제 데이터 크기:
    // '='가 2개면 마지막 2바이트는 실제 데이터가 아님.
    // '='가 1개면 마지막 1바이트는 실제 데이터가 아님.
    // '='가 0개면 ret 그대로 사용
    if (eqCount > 2) eqCount = 2; // base64 표준상 최대 2개
    ret -= (int)eqCount;
    if (ret < 0) ret = 0; // 혹시나 음수가 될 경우 0 처리

    outBuf.resize((size_t)ret);
    return outBuf;
}

std::string EncryptionManager::hashPassword(const std::string& password) {
    std::vector<unsigned char> salt = generateSalt();
    
    unsigned char hash[EVP_MAX_MD_SIZE];

    // PBKDF2를 사용한 패스워드 해싱
    PKCS5_PBKDF2_HMAC(
        password.c_str(), password.length(),
        salt.data(), salt.size(),
        HASH_ITERATIONS,
        EVP_sha256(),
        sizeof(hash),
        hash
    );

    // salt + hash를 합쳐서 base64로 인코딩
    std::vector<unsigned char> combined;
    combined.insert(combined.end(), salt.begin(), salt.end());
    combined.insert(combined.end(), hash, hash + sizeof(hash));
    
    return base64Encode(combined.data(), combined.size());
}

bool EncryptionManager::verifyPassword(const std::string& password, const std::string& hashStr) {
    auto combined = base64Decode(hashStr);
    if (combined.size() < SALT_LEN + EVP_MAX_MD_SIZE) {
        return false;
    }

    std::vector<unsigned char> salt(combined.begin(), combined.begin() + SALT_LEN);
    std::vector<unsigned char> storedHash(combined.begin() + SALT_LEN, combined.end());

    unsigned char newHash[EVP_MAX_MD_SIZE];
    unsigned int hashLen;

    PKCS5_PBKDF2_HMAC(
        password.c_str(), password.length(),
        salt.data(), salt.size(),
        HASH_ITERATIONS,
        EVP_sha256(),
        sizeof(newHash),
        newHash
    );

    return std::memcmp(newHash, storedHash.data(), storedHash.size()) == 0;
}

void EncryptionManager::rotateKey() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!isKeyExpired()) {
        return;
    }

    std::string newKey = generateNewKey();
    std::string oldKey = key_;

    // 데이터 재암호화
    reEncryptData(oldKey, newKey);

    key_ = newKey;
    keyId_ = keyGenerator.generateCryptographicKey();
    keyCreationTime_ = std::time(nullptr);
}

std::string EncryptionManager::generateNewKey() {
    std::vector<unsigned char> keyData(KEY_LEN);
    if (RAND_bytes(keyData.data(), KEY_LEN) != 1) {
        throw std::runtime_error("Failed to generate new key");
    }
    return std::string(reinterpret_cast<char*>(keyData.data()), KEY_LEN);
}

bool EncryptionManager::isKeyExpired() const {
    auto currentTime = std::time(nullptr);
    auto elapsedTime = static_cast<size_t>(currentTime - keyCreationTime_);
    return elapsedTime >= KEY_ROTATION_INTERVAL;
}

bool EncryptionManager::isKeyValid() const {
    return !key_.empty() && !isKeyExpired();
}

std::vector<unsigned char> EncryptionManager::generateSalt() const {
    std::vector<unsigned char> salt(SALT_LEN);
    if (RAND_bytes(salt.data(), SALT_LEN) != 1) {
        throw std::runtime_error("Failed to generate salt");
    }
    return salt;
}

EncryptionManager::EncryptionMetadata EncryptionManager::getMetadata() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return EncryptionMetadata{
        .algorithm = "AES-256-GCM",
        .keyId = keyId_,
        .timestamp = keyCreationTime_,
        .needsRotation = isKeyExpired()
    };
}

std::string EncryptionManager::exportEncryptedKey(const std::string& masterKey) {
    std::lock_guard<std::mutex> lock(mutex_);

    // 1. 마스터 키로부터 키 유도
    std::vector<unsigned char> derivedKey(KEY_LEN);
    std::vector<unsigned char> salt = generateSalt();
    
    if (PKCS5_PBKDF2_HMAC(
        masterKey.c_str(), masterKey.length(),
        salt.data(), salt.size(),
        HASH_ITERATIONS * 2,  // 키 유도에는 더 많은 반복 사용
        EVP_sha256(),
        KEY_LEN,
        derivedKey.data()
    ) != 1) {
        throw std::runtime_error("Failed to derive key from master key");
    }

    // 2. 현재 시간 기반의 nonce 생성
    unsigned char nonce[IV_LEN];
    auto timestamp = std::chrono::system_clock::now().time_since_epoch().count();
    std::memcpy(nonce, &timestamp, std::min(sizeof(timestamp), size_t(IV_LEN)));
    
    if (RAND_bytes(nonce + std::min(sizeof(timestamp), size_t(IV_LEN)), 
                   IV_LEN - std::min(sizeof(timestamp), size_t(IV_LEN))) != 1) {
        throw std::runtime_error("Failed to generate nonce");
    }

    // 3. 키 메타데이터 준비
    Json::Value keyMetadata;
    keyMetadata["version"] = "1";  // 암호화 버전
    keyMetadata["keyId"] = keyId_;
    keyMetadata["timestamp"] = static_cast<Json::UInt64>(keyCreationTime_);
    keyMetadata["algorithm"] = "AES-256-GCM";

    // 4. 키와 메타데이터를 함께 암호화
    std::string metadataStr = utils::JsonUtils::toJsonString(keyMetadata);
    std::string dataToEncrypt = key_ + "|" + metadataStr;

    // 5. HMAC 생성
    EVP_MAC *mac = EVP_MAC_fetch(NULL, "HMAC", NULL);
    if (!mac) {
        throw std::runtime_error("Failed to create MAC");
    }

    EVP_MAC_CTX *mac_ctx = EVP_MAC_CTX_new(mac);
    if (!mac_ctx) {
        EVP_MAC_free(mac);
        throw std::runtime_error("Failed to create MAC context");
    }

    OSSL_PARAM params[2];
    const char* digest = "SHA256";
    params[0] = OSSL_PARAM_construct_utf8_string("digest", const_cast<char*>(digest), 0);
    params[1] = OSSL_PARAM_construct_end();

    if (!EVP_MAC_init(mac_ctx, derivedKey.data(), KEY_LEN, params)) {
        EVP_MAC_CTX_free(mac_ctx);
        EVP_MAC_free(mac);
        throw std::runtime_error("Failed to initialize MAC");
    }

    if (!EVP_MAC_update(mac_ctx, (const unsigned char*)dataToEncrypt.c_str(), dataToEncrypt.length())) {
        EVP_MAC_CTX_free(mac_ctx);
        EVP_MAC_free(mac);
        throw std::runtime_error("Failed to update MAC");
    }

    size_t hmacLength;
    unsigned char hmac[EVP_MAX_MD_SIZE];
    if (!EVP_MAC_final(mac_ctx, hmac, &hmacLength, sizeof(hmac))) {
        EVP_MAC_CTX_free(mac_ctx);
        EVP_MAC_free(mac);
        throw std::runtime_error("Failed to finalize MAC");
    }

    EVP_MAC_CTX_free(mac_ctx);
    EVP_MAC_free(mac);

    // 6. 암호화 컨텍스트 설정
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        throw std::runtime_error("Failed to create encryption context");
    }

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to initialize encryption");
    }

    // 7. IV(nonce) 설정
    if (EVP_EncryptInit_ex(ctx, NULL, NULL, derivedKey.data(), nonce) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to set key and IV");
    }

    // 8. 데이터 암호화
    std::vector<unsigned char> ciphertext(dataToEncrypt.length() + EVP_MAX_BLOCK_LENGTH);
    int ciphertextLen;

    if (EVP_EncryptUpdate(ctx, ciphertext.data(), &ciphertextLen,
                         (const unsigned char*)dataToEncrypt.c_str(),
                         dataToEncrypt.length()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Encryption failed");
    }

    int finalLen;
    if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + ciphertextLen, &finalLen) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Encryption finalization failed");
    }

    ciphertextLen += finalLen;

    // 9. GCM 태그 얻기
    unsigned char tag[TAG_LEN];
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, TAG_LEN, tag) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to get tag");
    }

    EVP_CIPHER_CTX_free(ctx);

    // 10. 최종 데이터 구조 생성
    std::vector<unsigned char> finalData;
    finalData.reserve(1 + SALT_LEN + IV_LEN + hmacLength + ciphertextLen + TAG_LEN);
    
    finalData.push_back(1);  // 버전 식별자
    finalData.insert(finalData.end(), salt.begin(), salt.end());
    finalData.insert(finalData.end(), nonce, nonce + IV_LEN);
    finalData.insert(finalData.end(), hmac, hmac + hmacLength);
    finalData.insert(finalData.end(), ciphertext.begin(), ciphertext.begin() + ciphertextLen);
    finalData.insert(finalData.end(), tag, tag + TAG_LEN);

    // 11. Base64 인코딩하여 반환
    return base64Encode(finalData.data(), finalData.size());
}

void EncryptionManager::importEncryptedKey(const std::string& encryptedKey, const std::string& masterKey) {
    std::lock_guard<std::mutex> lock(mutex_);

    try {
        // 1. Base64 디코딩
        std::vector<unsigned char> encryptedData = base64Decode(encryptedKey);
        if (encryptedData.size() < 1 + SALT_LEN + IV_LEN + EVP_MAX_MD_SIZE + TAG_LEN) {
            throw std::runtime_error("Invalid encrypted key format");
        }

        // 2. 버전 확인
        uint8_t version = encryptedData[0];
        if (version != 1) {
            throw std::runtime_error("Unsupported key encryption version");
        }

        // 3. 데이터 구조 파싱
        size_t offset = 1;  // 버전 바이트 이후부터

        // Salt 추출
        std::vector<unsigned char> salt(encryptedData.begin() + offset, 
                                      encryptedData.begin() + offset + SALT_LEN);
        offset += SALT_LEN;

        // Nonce(IV) 추출
        unsigned char nonce[IV_LEN];
        std::memcpy(nonce, encryptedData.data() + offset, IV_LEN);
        offset += IV_LEN;

        // HMAC 추출
        unsigned char storedHmac[EVP_MAX_MD_SIZE];
        std::memcpy(storedHmac, encryptedData.data() + offset, EVP_MAX_MD_SIZE);
        offset += EVP_MAX_MD_SIZE;

        // 암호문 및 태그 추출
        size_t ciphertextLen = encryptedData.size() - offset - TAG_LEN;
        std::vector<unsigned char> ciphertext(encryptedData.begin() + offset,
                                            encryptedData.begin() + offset + ciphertextLen);
        offset += ciphertextLen;

        unsigned char tag[TAG_LEN];
        std::memcpy(tag, encryptedData.data() + offset, TAG_LEN);

        // 4. 마스터 키로부터 키 유도
        std::vector<unsigned char> derivedKey(KEY_LEN);
        if (PKCS5_PBKDF2_HMAC(
            masterKey.c_str(), masterKey.length(),
            salt.data(), salt.size(),
            HASH_ITERATIONS * 2,
            EVP_sha256(),
            KEY_LEN,
            derivedKey.data()
        ) != 1) {
            throw std::runtime_error("Failed to derive key from master key");
        }

        // 5. 복호화 컨텍스트 설정
        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx) {
            throw std::runtime_error("Failed to create decryption context");
        }

        if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Failed to initialize decryption");
        }

        if (EVP_DecryptInit_ex(ctx, NULL, NULL, derivedKey.data(), nonce) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Failed to set key and IV");
        }

        // 6. 복호화 수행
        std::vector<unsigned char> plaintext(ciphertextLen);
        int plaintextLen;

        if (EVP_DecryptUpdate(ctx, plaintext.data(), &plaintextLen,
                             ciphertext.data(), ciphertextLen) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Decryption failed");
        }

        // 7. GCM 태그 설정 및 검증
        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, TAG_LEN, tag) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Failed to set tag");
        }

        int finalLen;
        if (EVP_DecryptFinal_ex(ctx, plaintext.data() + plaintextLen, &finalLen) <= 0) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Decryption verification failed");
        }

        EVP_CIPHER_CTX_free(ctx);
        plaintextLen += finalLen;

        // 8. 평문을 문자열로 변환
        std::string decryptedData(reinterpret_cast<char*>(plaintext.data()), plaintextLen);

        // 9. 키와 메타데이터 분리
        size_t separatorPos = decryptedData.find('|');
        if (separatorPos == std::string::npos) {
            throw std::runtime_error("Invalid key format: missing separator");
        }

        std::string decryptedKey = decryptedData.substr(0, separatorPos);
        std::string metadataStr = decryptedData.substr(separatorPos + 1);

        // 10. HMAC 검증
        EVP_MAC *mac = EVP_MAC_fetch(NULL, "HMAC", NULL);
        if (!mac) {
            throw std::runtime_error("Failed to create MAC");
        }

        EVP_MAC_CTX *mac_ctx = EVP_MAC_CTX_new(mac);
        if (!mac_ctx) {
            EVP_MAC_free(mac);
            throw std::runtime_error("Failed to create MAC context");
        }

        OSSL_PARAM params[2];
        const char* digest = "SHA256";
        params[0] = OSSL_PARAM_construct_utf8_string("digest", const_cast<char*>(digest), 0);
        params[1] = OSSL_PARAM_construct_end();

        if (!EVP_MAC_init(mac_ctx, derivedKey.data(), KEY_LEN, params)) {
            EVP_MAC_CTX_free(mac_ctx);
            EVP_MAC_free(mac);
            throw std::runtime_error("Failed to initialize MAC");
        }

        if (!EVP_MAC_update(mac_ctx, (const unsigned char*)decryptedData.c_str(), decryptedData.length())) {
            EVP_MAC_CTX_free(mac_ctx);
            EVP_MAC_free(mac);
            throw std::runtime_error("Failed to update MAC");
        }

        unsigned char computedHmac[EVP_MAX_MD_SIZE];
        size_t hmacLength;
        if (!EVP_MAC_final(mac_ctx, computedHmac, &hmacLength, sizeof(computedHmac))) {
            EVP_MAC_CTX_free(mac_ctx);
            EVP_MAC_free(mac);
            throw std::runtime_error("Failed to finalize MAC");
        }

        EVP_MAC_CTX_free(mac_ctx);
        EVP_MAC_free(mac);

        if (CRYPTO_memcmp(storedHmac, computedHmac, hmacLength) != 0) {
            throw std::runtime_error("HMAC verification failed: data integrity compromised");
        }

        // 11. 메타데이터 파싱 및 검증
        Json::Value metadata = utils::JsonUtils::parseJson(metadataStr);
        
        if (metadata["version"].asString() != "1" ||
            metadata["algorithm"].asString() != "AES-256-GCM") {
            throw std::runtime_error("Invalid metadata");
        }

        // 12. 새 키 설정
        key_ = decryptedKey;
        keyId_ = metadata["keyId"].asString();
        keyCreationTime_ = metadata["timestamp"].asUInt64();

        // 13. 키 유효성 검증
        if (key_.length() != KEY_LEN) {
            throw std::runtime_error("Invalid key length after import");
        }

    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Key import failed: ") + e.what());
    }
}

void EncryptionManager::reEncryptData(const std::string& oldKey, const std::string& newKey) {
    std::lock_guard<std::mutex> lock(reEncryptMutex_);
    
    if (isReEncryptionInProgress()) {
        throw std::runtime_error("Reencryption already in progress");
    }

    try {
        auto dbClient = drogon::app().getDbClient();

        // 재암호화할 테이블과 컬럼 정보 정의
        std::vector<std::pair<std::string, std::string>> tablesToReEncrypt = {
            {"user_settings", "api_credentials"},
            {"user_settings", "strategy_params"},
            {"key_metadata", "metadata"}
        };

        // 총 처리해야 할 레코드 수 계산
        size_t totalRecords = 0;
        for (const auto& table : tablesToReEncrypt) {
            auto result = dbClient->execSqlSync(
                "SELECT COUNT(*) as count FROM " + table.first
            );
            totalRecords += result[0]["count"].as<size_t>();
        }

        updateReEncryptionStatus(true, totalRecords, 0);

        // 각 테이블에 대해 청크 단위로 데이터 재암호화
        const size_t CHUNK_SIZE = 100; // 한 번에 처리할 레코드 수
        size_t processedCount = 0;

        for (const auto& table : tablesToReEncrypt) {
            size_t tableRecordCount = 0;
            auto result = dbClient->execSqlSync(
                "SELECT COUNT(*) as count FROM " + table.first
            );
            tableRecordCount = result[0]["count"].as<size_t>();

            size_t tableProcessedCount = 0;

            while (tableProcessedCount < tableRecordCount) {
                reEncryptChunk(
                    oldKey, 
                    newKey, 
                    tableProcessedCount, 
                    CHUNK_SIZE, 
                    table.first, 
                    table.second
                );
                tableProcessedCount += CHUNK_SIZE;
                processedCount += CHUNK_SIZE;

                updateReEncryptionStatus(true, totalRecords, processedCount);
            }
        }

        // 재암호화 완료 표시
        updateReEncryptionStatus(false);

    } catch (const std::exception& e) {
        updateReEncryptionStatus(false);
        throw std::runtime_error(std::string("Reencryption failed: ") + e.what());
    }
}


void EncryptionManager::reEncryptChunk(
    const std::string& oldKey, 
    const std::string& newKey, 
    size_t offset, 
    size_t limit, 
    const std::string& tableName, 
    const std::string& columnName
) {
    auto dbClient = drogon::app().getDbClient();

    // 트랜잭션 시작
    dbClient->newTransactionAsync([=](const auto& transPtr) {
        try {
            // 테이블 및 컬럼 기반 데이터 조회
            auto result = transPtr->execSqlSync(
                "SELECT id, " + columnName + " FROM " + tableName + " LIMIT $1 OFFSET $2",
                limit, offset
            );

            for (const auto& row : result) {
                auto id = row.template at("id").template as<int64_t>();
                auto encryptedData = row.template at(columnName).template as<std::string>();

                // 기존 키로 복호화
                std::string tempKey = key_;
                key_ = oldKey;
                auto decrypted = decrypt(encryptedData);

                // 새 키로 암호화
                key_ = newKey;
                auto reEncrypted = encrypt(decrypted);
                key_ = tempKey;  // 원래 키 복구

                // 업데이트
                transPtr->execSqlSync(
                    "UPDATE " + tableName + " SET " + columnName + " = $1 WHERE id = $2",
                    reEncrypted, id
                );
            }

            transPtr->execSqlSync("COMMIT");

        } catch (const std::exception& e) {
            transPtr->execSqlSync("ROLLBACK");
            throw;
        }
    });
}

void EncryptionManager::updateReEncryptionStatus(
    bool inProgress, 
    size_t total, 
    size_t processed, 
    const std::string& errorMessage
) {
    std::lock_guard<std::mutex> lock(reEncryptMutex_);
    reEncryptionStatus_.inProgress = inProgress;
    if (inProgress) {
        if (total > 0) reEncryptionStatus_.totalItems = total;
        reEncryptionStatus_.processedItems = processed;
        if (processed == 0) reEncryptionStatus_.startTime = std::time(nullptr);
    } else {
        reEncryptionStatus_.totalItems = 0;
        reEncryptionStatus_.processedItems = 0;
        reEncryptionStatus_.startTime = 0;
    }
    reEncryptionStatus_.errorMessage = errorMessage;
}


bool EncryptionManager::isReEncryptionInProgress() const {
    std::lock_guard<std::mutex> lock(reEncryptMutex_);
    return reEncryptionStatus_.inProgress;
}

EncryptionManager::ReEncryptionProgress EncryptionManager::getReEncryptionProgress() const {
    std::lock_guard<std::mutex> lock(reEncryptMutex_);
    
    ReEncryptionProgress progress;
    progress.inProgress = reEncryptionStatus_.inProgress;
    
    if (!progress.inProgress) {
        progress.percentComplete = 100.0;
        progress.elapsedTime = std::chrono::seconds(0);
        progress.estimatedTimeRemaining = std::chrono::seconds(0);
        return progress;
    }
    
    // 진행률 계산
    progress.percentComplete = (reEncryptionStatus_.totalItems > 0)
        ? (static_cast<double>(reEncryptionStatus_.processedItems) / reEncryptionStatus_.totalItems * 100.0)
        : 0.0;
    
    // 경과 시간 계산
    auto now = std::time(nullptr);
    progress.elapsedTime = std::chrono::seconds(now - reEncryptionStatus_.startTime);
    
    // 남은 시간 추정
    if (progress.percentComplete > 0) {
        auto totalEstimatedSeconds = static_cast<int64_t>(
            progress.elapsedTime.count() * (100.0 / progress.percentComplete)
        );
        progress.estimatedTimeRemaining = std::chrono::seconds(
            totalEstimatedSeconds - progress.elapsedTime.count()
        );
    } else {
        progress.estimatedTimeRemaining = std::chrono::seconds(0);
    }
    
    return progress;
}
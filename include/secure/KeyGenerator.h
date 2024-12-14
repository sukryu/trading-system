#pragma once

#include <string>
#include <vector>
#include <optional>
#include <chrono>
#include <json/json.h>

class KeyGenerator {
public:
    static constexpr size_t MIN_ENTROPY_BYTES = 64;
    static constexpr size_t KEY_LENGTH = 32;
    static constexpr size_t MIN_KEY_STRENGTH = 256;
    static constexpr size_t MIN_SHARES = 3;
    static constexpr size_t REQUIRED_SHARES = 2;
    static constexpr size_t IV_LEN = 12;
    static constexpr size_t TAG_LEN = 16;

    // 주요 public 메서드들
    static std::string generateCryptographicKey();
    static bool validateKeyStrength(const std::vector<unsigned char>& key);
    static void secureKeyStorage(const std::string& key, const std::string& keyId);
    static std::optional<std::string> recoverKey(const std::string& keyId);
    static bool revokeKey(const std::string& keyId);

private:
    // 키 생성 관련 private 메서드들
    static std::optional<std::string> tryHardwareKeyGeneration();
    static std::string generateSecureSoftwareKey();
    static bool collectSystemEntropy(std::vector<unsigned char>& buffer);
    static void collectCPUEntropy(std::vector<unsigned char>& buffer);
    static void collectTimingEntropy(std::vector<unsigned char>& buffer);
    static bool collectOSEntropy(std::vector<unsigned char>& buffer);
    static void mixEntropy(std::vector<unsigned char>& key, 
                          const std::vector<unsigned char>& additionalEntropy);

    // 키 분할 및 복구 관련 private 메서드들
    static std::vector<std::vector<unsigned char>> splitKey(const std::string& key);
    static std::optional<std::string> combineKeyShares(
        const std::vector<std::vector<unsigned char>>& shares);
    
    // 키 저장 관련 private 메서드들
    static void storeKeyShare(const std::vector<unsigned char>& share, 
                            const std::string& keyId, 
                            size_t shareIndex);
    static std::vector<std::vector<unsigned char>> retrieveKeyShares(
        const std::string& keyId);
    static void storeKeyMetadata(const std::string& keyId);
    
    // 보안 분석 관련 private 메서드들
    static double calculateEntropy(const std::vector<unsigned char>& data);
    static bool analyzeKeyStrength(const std::vector<unsigned char>& key);
    static bool performStatisticalTests(const std::vector<unsigned char>& key);

    // 키 메타데이터 관리를 위한 private 메서드들
    static std::optional<Json::Value> getKeyMetadata(const std::string& keyId);
    static bool updateKeyStatus(const std::string& keyId, const std::string& newStatus);
    static bool isKeyActive(const std::string& keyId);

    // 로깅 관련 private 메서드
    static void logShareOperation(const std::string& operation, 
                                const std::string& keyId, 
                                size_t shareIndex);
    static void logKeyEvent(const std::string& keyId,
                        const std::string& eventType,
                        const std::string& description);
};
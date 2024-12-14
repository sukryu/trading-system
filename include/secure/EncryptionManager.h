#pragma once
#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <mutex>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include "KeyGenerator.h"

class EncryptionManager {
public:
    static EncryptionManager& getInstance() {
        static EncryptionManager instance;
        return instance;
    }

    // 기존 메서드들
    std::string encrypt(const std::string& plaintext);
    std::string decrypt(const std::string& ciphertextBase64);

    // 새로운 메서드들 - 해싱
    std::string hashPassword(const std::string& password);
    bool verifyPassword(const std::string& password, const std::string& hash);
    
    // 키 관리
    void rotateKey();
    bool isKeyValid() const;
    std::string exportEncryptedKey(const std::string& masterKey);
    void importEncryptedKey(const std::string& encryptedKey, const std::string& masterKey);

    // 메타데이터
    struct EncryptionMetadata {
        std::string algorithm;  // 사용 중인 암호화 알고리즘
        std::string keyId;      // 현재 키의 ID
        time_t timestamp;       // 키 생성 시간
        bool needsRotation;     // 키 교체 필요 여부
    };
    EncryptionMetadata getMetadata() const;

    // 재암호화 모니터링을 위한 public 메서드
    struct ReEncryptionProgress {
        bool inProgress;
        double percentComplete;
        std::chrono::seconds elapsedTime;
        std::chrono::seconds estimatedTimeRemaining;
    };
    
    ReEncryptionProgress getReEncryptionProgress() const;

private:
    EncryptionManager();
    ~EncryptionManager();
    EncryptionManager(const EncryptionManager&) = delete;
    EncryptionManager& operator=(const EncryptionManager&) = delete;

    // 기존 멤버들
    std::string key_;
    std::string keyId_;
    time_t keyCreationTime_;
    mutable std::mutex mutex_;

    // 재암호화 관련 새 멤버들
    struct ReEncryptionStatus {
        bool inProgress{false};
        size_t totalItems{0};
        size_t processedItems{0};
        time_t startTime{0};
        std::string errorMessage{""};
    };

    ReEncryptionStatus reEncryptionStatus_;
    mutable std::mutex reEncryptMutex_;
    
    // 재암호화 상태 관리
    void updateReEncryptionStatus(bool inProgress, size_t total = 0, size_t processed = 0, const std::string& errorMessage = "");
    bool isReEncryptionInProgress() const;
    ReEncryptionStatus getReEncryptionStatus() const;
    
    // 청크 단위 재암호화 지원
    void reEncryptChunk(
        const std::string& oldKey, 
        const std::string& newKey, 
        size_t offset, 
        size_t limit,
        const std::string& tableName,
        const std::string& columnName
    );

    // 키 관리용 추가 멤버들
    static constexpr size_t KEY_LEN = 32;
    static constexpr size_t IV_LEN = 12;
    static constexpr size_t TAG_LEN = 16;
    static constexpr size_t SALT_LEN = 16;
    static constexpr size_t KEY_ROTATION_INTERVAL = 30 * 24 * 60 * 60;  // 30일
    static constexpr size_t HASH_ITERATIONS = 10000;
    KeyGenerator keyGenerator;

    // 내부 유틸리티 메서드들
    std::string base64Encode(const unsigned char* data, size_t len);
    std::vector<unsigned char> base64Decode(const std::string& b64);
    
    // 새로운 내부 메서드들
    std::string generateNewKey();
    std::vector<unsigned char> generateSalt() const;
    bool isKeyExpired() const;
    void reEncryptData(const std::string& oldKey, const std::string& newKey);
};
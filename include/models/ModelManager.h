#pragma once

#include "models/ModelFactory.h"
#include <string>
#include <memory>
#include <unordered_map>

namespace models {

    class ModelManager {
    public:
        // Singleton 인스턴스 얻기
        static ModelManager& getInstance();

        // 팩토리 등록
        void registerFactory(const std::string& modelName, std::shared_ptr<ModelFactory> factory);

        // 팩토리 얻기
        std::shared_ptr<ModelFactory> getFactory(const std::string& modelName) const;

    private:
        ModelManager() = default;
        ~ModelManager() = default;
        ModelManager(const ModelManager&) = delete;
        ModelManager& operator=(const ModelManager&) = delete;

        std::unordered_map<std::string, std::shared_ptr<ModelFactory>> factories_;
    };

}  // namespace models
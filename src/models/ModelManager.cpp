#include "models/ModelManager.h"
#include <stdexcept>

namespace models {

    ModelManager& ModelManager::getInstance() {
        static ModelManager instance;
        return instance;
    }

    void ModelManager::registerFactory(const std::string& modelName, std::shared_ptr<ModelFactory> factory) {
        if (!factory) {
            throw std::invalid_argument("Null factory provided for model: " + modelName);
        }
        factories_[modelName] = factory;
    }

    std::shared_ptr<ModelFactory> ModelManager::getFactory(const std::string& modelName) const {
        auto it = factories_.find(modelName);
        if (it == factories_.end()) {
            throw std::runtime_error("Factory not found for model: " + modelName);
        }
        return it->second;
    }

}  // namespace models
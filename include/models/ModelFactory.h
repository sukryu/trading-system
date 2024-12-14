#pragma once

#include "models/BaseModel.h"
#include <drogon/orm/Result.h>
#include <drogon/orm/Row.h>
#include <memory>
#include <vector>
#include <json/json.h>

namespace models {

    class ModelFactory {
    public:
        virtual ~ModelFactory() = default;
        
        // DB row에서 모델 생성
        virtual std::shared_ptr<BaseModel> createFromDbRow(const drogon::orm::Row& row) const = 0;
        
        // DB result에서 모델 벡터 생성
        virtual std::vector<std::shared_ptr<BaseModel>> createFromDbResult(const drogon::orm::Result& result) const = 0;
        
        // JSON에서 모델 생성
        virtual std::shared_ptr<BaseModel> createFromJson(const Json::Value& json) const = 0;
        
        // 빈 모델 생성
        virtual std::shared_ptr<BaseModel> create() const = 0;
    };

}
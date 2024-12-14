#pragma once

#include <drogon/orm/Result.h>
#include <drogon/orm/Row.h>
#include <drogon/orm/Field.h>
#include <drogon/orm/SqlBinder.h>
#include <drogon/orm/Mapper.h>
#include <json/json.h>
#include <memory>

namespace models {

    class BaseModel {
    public:
        virtual ~BaseModel() = default;
        virtual Json::Value toJson() const = 0;
        virtual void fromJson(const Json::Value& json) = 0;
    };

}
#pragma once

#include <drogon/drogon.h>
#include <vector>
#include <memory>
#include <string>

namespace models {
    namespace mappers {

        template<typename T>
        class BaseMapper {
        public:
            // CRUD 기본 연산
            virtual T insert(const T& model) = 0;
            virtual T findById(int64_t id) = 0;
            virtual std::vector<T> findAll() = 0;
            virtual std::vector<T> findByCriteria(const std::string& whereClause) = 0;
            virtual void update(const T& model) = 0;
            virtual void deleteById(int64_t id) = 0;
            virtual size_t count(const std::string& whereClause = "") = 0;

            // 페이징 처리
            virtual std::vector<T> findWithPaging(size_t limit, size_t offset) = 0;

        protected:
            BaseMapper() = default;
            virtual ~BaseMapper() = default;
            
            // DB 클라이언트 가져오기
            auto getDbClient() {
                return drogon::app().getDbClient();
            }
        };

    } // namespace mappers
} // namespace models
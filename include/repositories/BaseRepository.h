#pragma once

#include <vector>
#include <optional>
#include <memory>
#include <drogon/drogon.h>
#include <trantor/utils/Date.h>
#include <functional>

namespace repositories {

    template<typename T>
    class BaseRepository {
    public:
        virtual ~BaseRepository() = default;

        // CRUD 작업
        virtual T save(const T& entity) = 0;
        virtual std::optional<T> findById(int64_t id) const = 0;
        virtual bool deleteById(int64_t id) = 0;

        // 페이징
        struct PaginationResult {
            std::vector<T> items;
            size_t totalCount;
            size_t pageSize;
            size_t currentPage;
            size_t totalPages;
        };

        virtual PaginationResult findAll(size_t page, size_t pageSize) const = 0;
        
        // 시간 범위 조회
        virtual std::vector<T> findByTimeRange(
            const trantor::Date& start,
            const trantor::Date& end
        ) const = 0;

        // 트랜잭션 실행
        using TransactionPtr = std::shared_ptr<drogon::orm::Transaction>;
        template<typename Func>
        void executeInTransaction(Func&& func) {
            auto clientPtr = drogon::app().getDbClient();
            clientPtr->newTransactionAsync(
                [func = std::forward<Func>(func)](const TransactionPtr& transPtr) mutable {
                    try {
                        // 사용자 제공 함수 호출
                        func(transPtr);

                        // COMMIT
                        auto commitBinder = (*transPtr) << "COMMIT";
                        commitBinder >> [](const drogon::orm::Result &r) {
                            // Commit 성공 시 처리
                            LOG_DEBUG << "Transaction committed successfully";
                        };
                        commitBinder >> [](const std::exception_ptr &e) {
                            // Commit 실행 중 예외 발생 시 처리
                            try {
                                if (e) std::rethrow_exception(e);
                            } catch (const std::exception &ex) {
                                LOG_ERROR << "Commit failed: " << ex.what();
                            }
                        };
                        commitBinder.exec(); // 여기서 exec()는 commitBinder(=SqlBinder) 객체에서 호출

                    } catch (const std::exception& e) {
                        // ROLLBACK
                        auto rollbackBinder = (*transPtr) << "ROLLBACK";
                        rollbackBinder >> [](const drogon::orm::Result &r) {
                            // Rollback 성공 시 처리
                            LOG_DEBUG << "Transaction rolled back";
                        };
                        rollbackBinder >> [](const std::exception_ptr &e) {
                            // Rollback 실행 중 예외 발생 시 처리
                            try {
                                if (e) std::rethrow_exception(e);
                            } catch (const std::exception &ex) {
                                LOG_ERROR << "Rollback failed: " << ex.what();
                            }
                        };
                        rollbackBinder.exec(); // rollbackBinder(=SqlBinder)에서 exec() 호출

                        throw; // 원래 예외 재throw
                    }
                }
            );
        }


    protected:
        BaseRepository() = default;

        // DB 클라이언트 가져오기
        drogon::orm::DbClientPtr getDbClient() const {
            return drogon::app().getDbClient();
        }
    };

} // namespace repositories
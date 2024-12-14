#include <catch2/catch.hpp>
#include "models/mappers/MarketDataMapper.h"
#include "models/MarketData.h"
#include <trantor/utils/Date.h>
#include <drogon/drogon.h>
#include <chrono>

// Helper function to create a MarketData object
static models::MarketData createTestMarketData(const std::string& symbol = "BTC/USD") {
    models::MarketData data;
    data.setSymbol(symbol);
    data.setPrice(30000.5);
    data.setVolume(123.45);
    // 현재 시간 기준
    auto now = trantor::Date::now();
    data.setTimestamp(now);
    data.setSource("TestSource");
    data.setCreatedAt(now);
    return data;
}

TEST_CASE("MarketDataMapper CRUD Test", "[MarketDataMapper]") {
    auto& mapper = models::mappers::MarketDataMapper::getInstance();
    REQUIRE_NOTHROW(models::mappers::MarketDataMapper::getInstance()); // Singleton 접근 확인

    // 1. Insert 테스트
    auto testData = createTestMarketData("ETH/USD");
    REQUIRE_NOTHROW(mapper.insert(testData)); // Insert without transaction
    // Insert 후, testData는 DB에 없던 ID가 할당되어야 하나, RETURNING *로 가져올 때 setter호출이 필요. 
    // 여기서는 mapper.insert()에서 fromDbRow로 만든 새 객체를 반환하므로 새로운 객체를 받아야 함.
    auto inserted = mapper.insert(testData);
    REQUIRE(inserted.getId() != 0);
    int64_t newId = inserted.getId();

    // 2. FindById 테스트
    REQUIRE_NOTHROW(mapper.findById(newId));
    auto found = mapper.findById(newId);
    REQUIRE(found.getId() == newId);
    REQUIRE(found.getSymbol() == "ETH/USD");
    REQUIRE(found.getPrice() == Approx(30000.5));
    REQUIRE(found.getVolume() == Approx(123.45));
    REQUIRE(found.getSource() == "TestSource");

    // 3. Update 테스트 (비트코인/달러 가격 변경)
    found.setPrice(31000.0);
    REQUIRE_NOTHROW(mapper.update(found));
    auto updated = mapper.findById(newId);
    REQUIRE(updated.getPrice() == Approx(31000.0));

    // 4. findLatestBySymbol 테스트
    // 동일 심볼로 또다른 데이터 삽입
    {
        auto anotherData = createTestMarketData("ETH/USD");
        auto inserted2 = mapper.insert(anotherData);
        REQUIRE(inserted2.getId() != 0);

        // findLatestBySymbol 하면 inserted2가 더 나중 timestamp이므로 리턴되어야 함
        auto latest = mapper.findLatestBySymbol("ETH/USD");
        REQUIRE(latest.getId() == inserted2.getId());
    }

    // 5. findBySymbolWithLimit 테스트
    // "ETH/USD" 심볼 데이터가 2개 있으므로 limit 1로 조회하면 최신 1개만
    {
        auto list = mapper.findBySymbolWithLimit("ETH/USD", 1);
        REQUIRE(list.size() == 1);
        auto latest = mapper.findLatestBySymbol("ETH/USD");
        REQUIRE(list[0].getId() == latest.getId());
    }

    // 6. TimeRange 조회 테스트
    {
        auto start = trantor::Date::now().after(-3600); // 1시간 전
        auto end = trantor::Date::now().after(3600); // 1시간 후
        // Time range 내 "ETH/USD" 모두 조회
        auto list = mapper.findBySymbolAndTimeRange("ETH/USD", start, end);
        REQUIRE(list.size() >= 2); // 위에서 ETH/USD로 2개 넣었으니 최소 2개 존재
    }

    // 7. Count 테스트
    {
        size_t cnt = mapper.count();
        REQUIRE(cnt > 0);
    }

    // 8. Delete 테스트
    REQUIRE_NOTHROW(mapper.deleteById(newId));
    REQUIRE_THROWS_AS(mapper.findById(newId), std::runtime_error); // 삭제 후 조회 시 에러

    // 트랜잭션 기반 테스트 (옵션)
    {
        auto client = drogon::app().getDbClient();
        auto trans = client->newTransaction();
        auto transData = createTestMarketData("XRP/USD");
        auto insertedTrans = mapper.insert(transData, *trans);
        REQUIRE(insertedTrans.getId() != 0);

        // rollback
        auto rollbackBinder = (*trans) << "ROLLBACK";
        rollbackBinder >> [](const drogon::orm::Result&){ }
                    >> [](const std::exception_ptr &){};

        // rollback 실행
        rollbackBinder.exec();

        // 롤백했으니 조회 시 존재하지 않아야 함
        REQUIRE_THROWS_AS(mapper.findById(insertedTrans.getId()), std::runtime_error);
    }

    // 테스트 끝
}

cmake_minimum_required(VERSION 3.14)
project(trading_system VERSION 0.1.0)

# C++17 설정
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 테스트 활성화
enable_testing()

# 빌드 타입이 지정되지 않은 경우 Debug로 설정
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# 컴파일러 플래그
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -mrdrnd")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")

# 의존성 패키지 찾기
find_package(Drogon REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP jsoncpp)
find_package(spdlog REQUIRED)
find_package(OpenSSL REQUIRED)
# libcds 및 의존성 찾기
find_package(CDS REQUIRED)
find_package(TBB REQUIRED)
find_package(Boost REQUIRED)

# PostgreSQL 찾기
find_package(PostgreSQL REQUIRED)

# Catch2 설치
Include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v2.13.8
)
FetchContent_MakeAvailable(Catch2)

# 명시적으로 소스 파일 나열
set(SOURCES
    src/main.cpp
    # controllers
    src/controllers/HealthController.cpp
    # utils
    src/utils/Config.cpp
    src/utils/Logger.cpp
    src/utils/MigrationManager.cpp
    # models
    src/models/MarketData.cpp
    src/models/TradingSignal.cpp
    src/models/Order.cpp
    src/models/Trade.cpp
    src/models/ModelManager.cpp
    src/models/User.cpp
    src/models/UserSettings.cpp
    # models/mappers
    src/models/mappers/MarketDataMapper.cpp
    src/models/mappers/TradingSignalMapper.cpp
    src/models/mappers/OrderMapper.cpp
    src/models/mappers/TradeMapper.cpp
    src/models/mappers/UserMapper.cpp
    src/models/mappers/UserSettingsMapper.cpp
    # models/factories
    src/models/factories/MarketDataFactory.cpp
    src/models/factories/TradingSignalFactory.cpp
    src/models/factories/OrderFactory.cpp
    src/models/factories/TradeFactory.cpp
    src/models/factories/UserFactory.cpp
    src/models/factories/UserSettingsFactory.cpp
    # repositories
    src/repositories/MarketDataRepository.cpp
    src/repositories/OrderRepository.cpp
    src/repositories/TradeRepository.cpp
    src/repositories/TradingSignalRepository.cpp
    src/repositories/UserRepository.cpp
    src/repositories/UserSettingsRepository.cpp
    # secure
    src/secure/KeyGenerator.cpp
    src/secure/CryptoAuditor.cpp
    src/secure/SecurityPolicy.cpp
    src/secure/EncyptionManager.cpp
    src/secure/SensitiveDataHandler.cpp
    src/secure/IpWhitelistManager.cpp
    # memory
    src/memory/PoolMemoryResource.cpp
    # containers
    src/containers/LockFreeContainers.cpp
)

# 메인 실행 파일 생성
add_executable(${PROJECT_NAME} ${SOURCES})

# 포함 디렉토리 설정
target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${JSONCPP_INCLUDE_DIRS}
    ${PostgreSQL_INCLUDE_DIRS}
    ${CDS_INCLUDE_DIRS}
    ${TBB_INCLUDE_DIRS}
)

# 라이브러리 링크
target_link_libraries(${PROJECT_NAME} PRIVATE
    Drogon::Drogon
    ${JSONCPP_LIBRARIES}
    spdlog::spdlog
    ${PostgreSQL_LIBRARIES}
    OpenSSL::SSL OpenSSL::Crypto
)

# 단위 테스트 실행 파일 생성
add_executable(unit_tests 
    tests/unit/main_test.cpp
    tests/unit/health_controller_test.cpp
    src/controllers/HealthController.cpp
    tests/unit/mappers/MarketDataMapper_test.cpp
    src/models/MarketData.cpp
    src/models/mappers/MarketDataMapper.cpp
)

# 테스트 헤더 파일 경로 설정
target_include_directories(unit_tests PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${PostgreSQL_INCLUDE_DIRS}
)

# 테스트 라이브러리 링크
target_link_libraries(unit_tests PRIVATE
    Catch2::Catch2
    Drogon::Drogon
    ${JSONCPP_LIBRARIES}
    spdlog::spdlog
    ${PostgreSQL_LIBRARIES}
)

# 테스트 케이스 등록
add_test(NAME unit_tests COMMAND unit_tests)
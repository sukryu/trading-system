enable_testing()

# Catch2 설치
Include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v2.13.8
)
FetchContent_MakeAvailable(Catch2)

# 단위 테스트 실행 파일 생성
add_executable(unit_tests 
    unit/main_test.cpp
    unit/health_controller_test.cpp
)

# 헤더 파일 경로 설정
target_include_directories(unit_tests PRIVATE 
    ${CMAKE_SOURCE_DIR}/include
)

# 라이브러리 링크
target_link_libraries(unit_tests PRIVATE
    Catch2::Catch2
    Drogon::Drogon
    ${JSONCPP_LIBRARIES}
    spdlog::spdlog
)

# 테스트 케이스 등록
add_test(NAME unit_tests COMMAND unit_tests)
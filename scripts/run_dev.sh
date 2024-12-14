#!/bin/bash
set -e

# 환경 변수 로드
source .env

# 빌드 디렉토리 생성 및 이동
mkdir -p build
cd build

# CMake 구성 및 빌드
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)

# 실행
./trading_system
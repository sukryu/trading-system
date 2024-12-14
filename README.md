# Secure Trading System

## 📌 프로젝트 소개
**Secure Trading System**은 암호화폐 및 주식 거래를 안전하게 관리하기 위한 고성능 거래 시스템입니다.  
이 시스템은 강력한 보안, 확장성, 신뢰성을 갖춘 거래 플랫폼으로 설계되었으며, API를 통해 사용자와 애플리케이션 간의 효율적인 통합을 제공합니다.

---

## 📂 주요 기능
- **실시간 시장 데이터 관리**: 다양한 거래소에서 수집된 데이터를 기반으로 실시간 가격 및 볼륨 업데이트.
- **자동화된 거래 신호**: 전략 기반의 거래 신호 생성 및 관리.
- **주문 및 거래 관리**: 안전하고 빠른 주문 생성, 체결 및 기록.
- **보안 강화**:
  - 암호화 키 관리 및 분할 저장 (Shamir’s Secret Sharing)
  - IP 화이트리스트 및 비정상 활동 감지
  - 암호화 작업 감사 로그와 경고 시스템
- **확장성 및 모니터링**:
  - PostgreSQL, Redis, Kubernetes 기반의 확장 가능한 구조
  - Prometheus 및 Grafana로 실시간 시스템 모니터링

---

## 🛠️ 기술 스택
- **Backend**: C++, Drogon Framework
- **Database**: PostgreSQL, Redis
- **Monitoring**: Prometheus, Grafana
- **CI/CD**: GitHub Actions
- **Security**: OpenSSL, Shamir’s Secret Sharing
- **Infrastructure**: Docker, Kubernetes, Helm

---

## 🚀 시작하기

### 1. 요구 사항
- **OS**: Linux/Unix
- **C++ Compiler**: GCC 13.0 이상
- **Database**: PostgreSQL 14.x 이상, Redis 6.x 이상
- **추가 패키지**:
  - OpenSSL
  - Drogon Framework
  - CMake 3.20 이상

### 2. 설치

1. **코드 클론**
   ```bash
   git clone https://github.com/sukryu/trading-system.git
   cd trading-system

2. **빌드**
    ```bash
    mkdir build && cd build
    cmake ..
    make -j$(nproc)
    ```

3. 환경 파일 설정 프로젝트 루트에 `.env`파일을 생성하고 다음 내용을 추가합니다.
    ```makefile
    DB_HOST=localhost
    DB_PORT=5432
    DB_USER=your_user
    DB_PASSWORD=your_password
    REDIS_HOST=localhost
    REDIS_PORT=6379
    ```

4. 데이터베이스 초기화
    ```bash
    psql -U your_user -d your_database -f schema.sql
    ```

5. 애플리케이션 실행
    ```bash
    ./trading_system
    ```

## 📄 사용 방법

1. API
    - Market Data
        - `GET /api/market_data`
        - 시장 데이터를 조회합니다.

    - Trading Signals
        - `POST /api/trading_signals`
        - 거래 신호를 생성합니다.

    - Orders
        - `POST /api/orders`
        - 주문을 생성합니다.

2. 보안 관리
    - IP 화이트리스트: 비정상적인 활동 감지를 위한 IP 관리.
    - 암호화 키 관리: 민감한 데이터를 보호하기 위한 키 생성 및 복구.

## 📊 모니터링 및 로깅
    - Prometheus: 애플리케이션 성능 및 리소스 모니터링.
    - Grafana: 대시보드 시각화
    - 로그 관리: 운영 중 발생하는 오류 및 작업 상태를 실시간으로 로깅.

## 📜 라이선스
이 프로젝트는 MIT License에 따라 배포됩니다.


### 📌 주요 내용
- 프로젝트 소개와 주요 기능
- 설치 및 시작 방법
- API와 사용 예시
- 프로젝트 라이선스

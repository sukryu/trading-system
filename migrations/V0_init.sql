-- 데이터베이스 사용자 생성
DO $$
BEGIN
    IF NOT EXISTS (SELECT 1 FROM pg_roles WHERE rolname = 'trading_user') THEN
        CREATE USER trading_user WITH PASSWORD 'trading-system-jinhyeok';
    END IF;
END
$$;

-- 스키마 권한 설정
GRANT USAGE ON SCHEMA public TO trading_user;
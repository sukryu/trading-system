-- Trigger function for updated_at columns
CREATE OR REPLACE FUNCTION update_updated_at_column()
RETURNS TRIGGER AS $$
BEGIN
    NEW.updated_at = CURRENT_TIMESTAMP;
    RETURN NEW;
END;
$$ language 'plpgsql';

-- Market Data table
CREATE TABLE market_data (
    id BIGSERIAL PRIMARY KEY,
    symbol VARCHAR(20) NOT NULL,
    price DECIMAL(20,8) NOT NULL,
    volume DECIMAL(20,8) NOT NULL,
    timestamp TIMESTAMPTZ NOT NULL,
    source VARCHAR(50) NOT NULL,
    created_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP,

    CONSTRAINT market_data_symbol_check CHECK (length(symbol) > 0),
    CONSTRAINT market_data_price_check CHECK (price > 0),
    CONSTRAINT market_data_volume_check CHECK (volume >= 0)
);

-- Trading Signals table
CREATE TABLE trading_signals (
    id BIGSERIAL PRIMARY KEY,
    symbol VARCHAR(20) NOT NULL,
    signal_type VARCHAR(10) NOT NULL,
    price DECIMAL(20,8) NOT NULL,
    quantity DECIMAL(20,8) NOT NULL,
    strategy_name VARCHAR(50) NOT NULL,
    confidence DECIMAL(5,2),
    parameters JSONB,
    timestamp TIMESTAMPTZ NOT NULL,
    created_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP,

    CONSTRAINT trading_signals_symbol_check CHECK (length(symbol) > 0),
    CONSTRAINT trading_signals_signal_type_check CHECK (signal_type IN ('BUY', 'SELL')),
    CONSTRAINT trading_signals_price_check CHECK (price > 0),
    CONSTRAINT trading_signals_quantity_check CHECK (quantity > 0),
    CONSTRAINT trading_signals_confidence_check CHECK (confidence IS NULL OR (confidence >= 0 AND confidence <= 100))
);

-- Orders table
CREATE TABLE orders (
    id BIGSERIAL PRIMARY KEY,
    order_id VARCHAR(100) NOT NULL,
    symbol VARCHAR(20) NOT NULL,
    order_type VARCHAR(20) NOT NULL,
    side VARCHAR(10) NOT NULL,
    quantity DECIMAL(20,8) NOT NULL,
    price DECIMAL(20,8),
    status VARCHAR(20) NOT NULL,
    signal_id BIGINT REFERENCES trading_signals(id),
    filled_quantity DECIMAL(20,8) DEFAULT 0,
    filled_price DECIMAL(20,8),
    error_message TEXT,
    timestamp TIMESTAMPTZ NOT NULL,
    updated_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP,
    created_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP,

    CONSTRAINT orders_order_id_unique UNIQUE (order_id),
    CONSTRAINT orders_symbol_check CHECK (length(symbol) > 0),
    CONSTRAINT orders_order_type_check CHECK (order_type IN ('MARKET', 'LIMIT')),
    CONSTRAINT orders_side_check CHECK (side IN ('BUY', 'SELL')),
    CONSTRAINT orders_quantity_check CHECK (quantity > 0),
    CONSTRAINT orders_price_check CHECK (price IS NULL OR price > 0),
    CONSTRAINT orders_status_check CHECK (status IN ('PENDING', 'FILLED', 'CANCELLED', 'REJECTED')),
    CONSTRAINT orders_filled_quantity_check CHECK (filled_quantity >= 0)
);

-- Trades table
CREATE TABLE trades (
    id BIGSERIAL PRIMARY KEY,
    trade_id VARCHAR(100) NOT NULL,
    order_id BIGINT REFERENCES orders(id),
    symbol VARCHAR(20) NOT NULL,
    side VARCHAR(10) NOT NULL,
    quantity DECIMAL(20,8) NOT NULL,
    price DECIMAL(20,8) NOT NULL,
    commission DECIMAL(20,8),
    commission_asset VARCHAR(20),
    timestamp TIMESTAMPTZ NOT NULL,
    created_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP,

    CONSTRAINT trades_trade_id_unique UNIQUE (trade_id),
    CONSTRAINT trades_symbol_check CHECK (length(symbol) > 0),
    CONSTRAINT trades_side_check CHECK (side IN ('BUY', 'SELL')),
    CONSTRAINT trades_quantity_check CHECK (quantity > 0),
    CONSTRAINT trades_price_check CHECK (price > 0),
    CONSTRAINT trades_commission_check CHECK (commission IS NULL OR commission >= 0),
    CONSTRAINT trades_commission_asset_check CHECK (
        (commission IS NULL AND commission_asset IS NULL) OR
        (commission IS NOT NULL AND commission_asset IS NOT NULL)
    )
);

-- Users table
CREATE TABLE users (
    id BIGSERIAL PRIMARY KEY,
    email VARCHAR(255) NOT NULL UNIQUE,
    username VARCHAR(100) NOT NULL UNIQUE,
    password_hash VARCHAR(255) NOT NULL,
    is_active BOOLEAN NOT NULL DEFAULT TRUE,
    last_login_at TIMESTAMPTZ,
    created_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP,

    CONSTRAINT users_email_check CHECK (length(email) > 3),
    CONSTRAINT users_username_check CHECK (length(username) >= 3)
);

-- User Settings table
CREATE TABLE user_settings (
    id BIGSERIAL PRIMARY KEY,
    user_id BIGINT NOT NULL REFERENCES users(id),
    exchange_name VARCHAR(50) NOT NULL,
    api_credentials JSONB NOT NULL,  -- {api_key, api_secret, additional_params}
    strategy_params JSONB NOT NULL,  -- {strategy_type, ma_period, rsi_period, etc}
    watchlist JSONB NOT NULL,        -- ["BTC/USDT", "ETH/USDT", ...]
    risk_params JSONB NOT NULL,      -- {max_position_size, stop_loss_pct, take_profit_pct}
    auto_trade_enabled BOOLEAN NOT NULL DEFAULT FALSE,
    created_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP,

    CONSTRAINT user_settings_unique_user UNIQUE (user_id, exchange_name),
    CONSTRAINT user_settings_exchange_check CHECK (length(exchange_name) > 0)
);

-- Key Management Tables
CREATE TABLE key_metadata (
    id BIGSERIAL PRIMARY KEY,
    key_id VARCHAR(100) NOT NULL UNIQUE,
    metadata JSONB NOT NULL,
    status VARCHAR(20) NOT NULL DEFAULT 'active',
    version INTEGER NOT NULL DEFAULT 1,
    created_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP,

    CONSTRAINT key_metadata_key_id_check CHECK (length(key_id) > 0),
    CONSTRAINT key_metadata_status_check CHECK (status IN ('active', 'inactive', 'revoked')),
    CONSTRAINT key_metadata_version_check CHECK (version > 0)
);

CREATE TABLE key_shares (
    id BIGSERIAL PRIMARY KEY,
    key_id VARCHAR(100) NOT NULL,
    share_index INTEGER NOT NULL,
    share_data BYTEA NOT NULL,
    iv BYTEA NOT NULL,
    checksum VARCHAR(64) NOT NULL, -- SHA-256 체크섬
    encryption_metadata JSONB NOT NULL, -- 암호화 관련 메타데이터
    created_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP,

    CONSTRAINT key_shares_key_id_fkey FOREIGN KEY (key_id) 
        REFERENCES key_metadata(key_id) ON DELETE CASCADE,
    CONSTRAINT key_shares_index_range CHECK (share_index >= 0),
    CONSTRAINT key_shares_unique_index UNIQUE (key_id, share_index)
);

CREATE TABLE key_events (
    id BIGSERIAL PRIMARY KEY,
    key_id VARCHAR(100) NOT NULL,
    event_type VARCHAR(50) NOT NULL,
    description TEXT NOT NULL,
    user_id BIGINT REFERENCES users(id),
    source_ip VARCHAR(45),
    metadata JSONB,
    created_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP,

    CONSTRAINT key_events_key_id_fkey FOREIGN KEY (key_id) 
        REFERENCES key_metadata(key_id) ON DELETE CASCADE
);

CREATE TABLE key_share_operations (
    id BIGSERIAL PRIMARY KEY,
    key_id VARCHAR(100) NOT NULL,
    operation_type VARCHAR(50) NOT NULL,
    share_index INTEGER NOT NULL,
    user_id BIGINT REFERENCES users(id),
    source_ip VARCHAR(45),
    metadata JSONB NOT NULL,
    status VARCHAR(20) NOT NULL DEFAULT 'success',
    error_message TEXT,
    created_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP,

    CONSTRAINT key_share_operations_key_id_fkey FOREIGN KEY (key_id) 
        REFERENCES key_metadata(key_id) ON DELETE CASCADE,
    CONSTRAINT key_share_operations_status_check CHECK (status IN ('success', 'failed'))
);

-- 암호화 작업 감사 로그 테이블
CREATE TABLE crypto_audit_logs (
    id BIGSERIAL PRIMARY KEY,
    key_id VARCHAR(100) NOT NULL,
    operation VARCHAR(50) NOT NULL,
    success BOOLEAN NOT NULL,
    user_id VARCHAR(100),
    source_ip VARCHAR(45),  -- IPv6 주소 길이 고려
    metadata JSONB NOT NULL, -- 추가 메타데이터 (타임스탬프, 세부 정보 등)
    created_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP,

    CONSTRAINT crypto_audit_logs_key_id_fkey 
        FOREIGN KEY (key_id) REFERENCES key_metadata(key_id) ON DELETE CASCADE,
    CONSTRAINT crypto_audit_logs_operation_check 
        CHECK (operation IN ('encrypt', 'decrypt', 'key_generation', 'key_rotation', 
                           'key_export', 'key_import', 'share_creation', 'share_recovery'))
);

-- 암호화 관련 알림 및 경고 테이블
CREATE TABLE crypto_alerts (
    id BIGSERIAL PRIMARY KEY,
    alert_type VARCHAR(50) NOT NULL,
    severity VARCHAR(20) NOT NULL,
    operation VARCHAR(50),
    error_message TEXT,
    key_id VARCHAR(100),
    user_id VARCHAR(100),
    metadata JSONB NOT NULL,
    resolved BOOLEAN NOT NULL DEFAULT FALSE,
    resolved_at TIMESTAMPTZ,
    resolved_by VARCHAR(100),
    resolution_notes TEXT,
    created_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP,

    CONSTRAINT crypto_alerts_severity_check 
        CHECK (severity IN ('low', 'medium', 'high', 'critical')),
    CONSTRAINT crypto_alerts_key_id_fkey 
        FOREIGN KEY (key_id) REFERENCES key_metadata(key_id) ON DELETE SET NULL
);

-- 접근 IP 화이트리스트
CREATE TABLE ip_whitelist (
    id BIGSERIAL PRIMARY KEY,
    user_id VARCHAR(100) NOT NULL,
    ip_address VARCHAR(45) NOT NULL,
    description TEXT,
    is_active BOOLEAN NOT NULL DEFAULT TRUE,
    last_used_at TIMESTAMPTZ,
    created_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP,

    CONSTRAINT ip_whitelist_unique_ip UNIQUE (user_id, ip_address)
);

-- 작업 비율 제한 모니터링
CREATE TABLE operation_rate_limits (
    id BIGSERIAL PRIMARY KEY,
    user_id VARCHAR(100) NOT NULL,
    operation VARCHAR(50) NOT NULL,
    attempt_count INTEGER NOT NULL DEFAULT 1,
    first_attempt_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP,
    last_attempt_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP,
    window_start TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP,

    CONSTRAINT operation_rate_limits_unique 
        UNIQUE (user_id, operation, window_start)
);

-- Indices
CREATE INDEX idx_market_data_symbol_timestamp ON market_data(symbol, timestamp);
CREATE INDEX idx_market_data_created_at ON market_data(created_at);

CREATE INDEX idx_trading_signals_symbol_timestamp ON trading_signals(symbol, timestamp);
CREATE INDEX idx_trading_signals_created_at ON trading_signals(created_at);

CREATE INDEX idx_orders_symbol_timestamp ON orders(symbol, timestamp);
CREATE INDEX idx_orders_status ON orders(status);
CREATE INDEX idx_orders_created_at ON orders(created_at);

CREATE INDEX idx_trades_symbol_timestamp ON trades(symbol, timestamp);
CREATE INDEX idx_trades_created_at ON trades(created_at);

CREATE INDEX idx_users_email ON users(email);
CREATE INDEX idx_users_username ON users(username);
CREATE INDEX idx_users_created_at ON users(created_at);

CREATE INDEX idx_user_settings_user_id ON user_settings(user_id);
CREATE INDEX idx_user_settings_exchange ON user_settings(exchange_name);
CREATE INDEX idx_user_settings_created_at ON user_settings(created_at);

CREATE INDEX idx_key_metadata_status ON key_metadata(status);
CREATE INDEX idx_key_metadata_created_at ON key_metadata(created_at);
CREATE INDEX idx_key_metadata_updated_at ON key_metadata(updated_at);
CREATE INDEX idx_key_shares_key_id ON key_shares(key_id);
CREATE INDEX idx_key_events_key_id_type ON key_events(key_id, event_type);
CREATE INDEX idx_key_events_created_at ON key_events(created_at);
CREATE INDEX idx_key_share_operations_key_id ON key_share_operations(key_id);
CREATE INDEX idx_key_share_operations_status ON key_share_operations(status);
CREATE INDEX idx_key_share_operations_created_at ON key_share_operations(created_at);

CREATE INDEX idx_crypto_audit_logs_key_id ON crypto_audit_logs(key_id);
CREATE INDEX idx_crypto_audit_logs_user_id ON crypto_audit_logs(user_id);
CREATE INDEX idx_crypto_audit_logs_created_at ON crypto_audit_logs(created_at);
CREATE INDEX idx_crypto_audit_logs_operation_success 
    ON crypto_audit_logs(operation, success);

CREATE INDEX idx_crypto_alerts_created_at ON crypto_alerts(created_at);
CREATE INDEX idx_crypto_alerts_severity ON crypto_alerts(severity);
CREATE INDEX idx_crypto_alerts_resolved ON crypto_alerts(resolved);
CREATE INDEX idx_crypto_alerts_user_id ON crypto_alerts(user_id);

CREATE INDEX idx_ip_whitelist_user_ip ON ip_whitelist(user_id, ip_address);
CREATE INDEX idx_ip_whitelist_last_used ON ip_whitelist(last_used_at);

CREATE INDEX idx_operation_rate_limits_user_op 
    ON operation_rate_limits(user_id, operation);
CREATE INDEX idx_operation_rate_limits_window 
    ON operation_rate_limits(window_start);

-- 자동 업데이트 트리거
CREATE TRIGGER update_users_updated_at
    BEFORE UPDATE ON users
    FOR EACH ROW
    EXECUTE FUNCTION update_updated_at_column();

-- 자동 업데이트 트리거
CREATE TRIGGER update_user_settings_updated_at
    BEFORE UPDATE ON user_settings
    FOR EACH ROW
    EXECUTE FUNCTION update_updated_at_column();

-- 자동 업데이트 트리거
CREATE TRIGGER update_key_metadata_updated_at
    BEFORE UPDATE ON key_metadata
    FOR EACH ROW
    EXECUTE FUNCTION update_updated_at_column();
    
-- 자동 업데이트 트리거
CREATE TRIGGER update_ip_whitelist_updated_at
    BEFORE UPDATE ON ip_whitelist
    FOR EACH ROW
    EXECUTE FUNCTION update_updated_at_column();

-- 오래된 데이터 정리를 위한 파티셔닝 (선택사항)
CREATE TABLE crypto_audit_logs_partitioned (
    LIKE crypto_audit_logs INCLUDING ALL
) PARTITION BY RANGE (created_at);

-- 파티션 예시 (매월)
CREATE TABLE crypto_audit_logs_y2024m01 PARTITION OF crypto_audit_logs_partitioned
    FOR VALUES FROM ('2024-01-01') TO ('2024-02-01');

-- 뷰 생성
CREATE VIEW v_recent_suspicious_activities AS
SELECT 
    cal.user_id,
    cal.operation,
    cal.source_ip,
    COUNT(*) as failure_count,
    MAX(cal.created_at) as last_failure_at
FROM crypto_audit_logs cal
WHERE 
    cal.success = false 
    AND cal.created_at > CURRENT_TIMESTAMP - INTERVAL '1 hour'
GROUP BY 
    cal.user_id,
    cal.operation,
    cal.source_ip
HAVING 
    COUNT(*) >= 5;

-- Grant permissions
GRANT SELECT, INSERT, UPDATE, DELETE ON ALL TABLES IN SCHEMA public TO trading_user;
GRANT USAGE, SELECT ON ALL SEQUENCES IN SCHEMA public TO trading_user;
GRANT SELECT, INSERT ON crypto_audit_logs TO trading_user;
GRANT SELECT, INSERT ON crypto_alerts TO trading_user;
GRANT SELECT, INSERT, UPDATE ON ip_whitelist TO trading_user;
GRANT SELECT, INSERT, UPDATE ON operation_rate_limits TO trading_user;

-- Create extension for UUID generation if needed
CREATE EXTENSION IF NOT EXISTS "uuid-ossp";
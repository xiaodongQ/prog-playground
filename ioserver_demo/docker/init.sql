-- 创建数据库
CREATE DATABASE IF NOT EXISTS ioserver;
USE ioserver;

-- 创建数据表
CREATE TABLE IF NOT EXISTS data (
    id INT AUTO_INCREMENT PRIMARY KEY,
    key_name VARCHAR(255) NOT NULL,
    value TEXT NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 插入一些测试数据
-- INSERT INTO data (key_name, value) VALUES
--     ('test_key_1', 'test_value_1'),
--     ('test_key_2', 'test_value_2'),
--     ('test_key_3', 'test_value_3');
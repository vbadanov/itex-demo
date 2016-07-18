
--DROP DATABASE itex_demo

CREATE DATABASE itex_demo;
\connect itex_demo

--CREATE SCHEMA demo

/*
DROP TABLE securities CASCADE;
DROP TABLE active_securities CASCADE;
DROP TABLE orderbook_log CASCADE;
DROP TABLE orderbook_log_details CASCADE;
DROP TABLE orderbook_log_array CASCADE;
*/


CREATE TABLE securities (
	id BIGSERIAL PRIMARY KEY,
	parent_id BIGINT REFERENCES securities (id),
	alias_id BIGINT,
	sec_board VARCHAR(128),
	sec_code VARCHAR(128),
	description VARCHAR(1024)
);
CREATE INDEX securities_index_id on securities (id);


CREATE TABLE orderbook_log (
	id BIGSERIAL PRIMARY KEY,
	security_id BIGINT REFERENCES securities (id),
	timestamp DOUBLE PRECISION
);
CREATE INDEX orderbook_log_index_id on orderbook_log(id);
CREATE INDEX orderbook_log_index_security_id on orderbook_log(security_id);


CREATE TABLE orderbook_log_details (
	id BIGSERIAL PRIMARY KEY,
	orderbook_log_id BIGINT REFERENCES orderbook_log (id),
	price DOUBLE PRECISION,
	volume DOUBLE PRECISION
);
CREATE INDEX orderbook_log_details_index_orderbook_log_id on orderbook_log_details(orderbook_log_id);



INSERT INTO securities (id, parent_id, alias_id, sec_board, sec_code, description) VALUES
(0, 0, 0, 'Si', 'Si-9.16', 'USDRUR Future')
;

/*
INSERT INTO active_securities (id, security_id) VALUES
(0,1),
;
*/


CREATE TABLE IF NOT EXISTS `cracks` (
	`id`          INT(10)      NOT NULL AUTO_INCREMENT PRIMARY KEY,
	`algorithm`   VARCHAR(32)  NOT NULL,
	`charset`     VARCHAR(256) NOT NULL,
	`hash`        TEXT         NOT NULL,
	`maxlen`      INT(11)      NOT NULL,
	`nextwu`      VARCHAR(64)  NOT NULL,
	`started`     INT(11)      NOT NULL,
	`expiration`  INT(11)      NOT NULL,
	`priority`    INT(11)      NOT NULL,
	`active`      INT(11)      NOT NULL,
	`disposition` VARCHAR(6)   NOT NULL,
	`collision`   VARCHAR(255) NOT NULL,
	`updated`     INT(11)      NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;

CREATE TABLE IF NOT EXISTS `hashes` (
	`id`          INT(10)      NOT NULL AUTO_INCREMENT PRIMARY KEY,
	`active`      INT(11)      NOT NULL,
	`crack_id`    INT(10)      NOT NULL,
	`hash`        TEXT         NOT NULL,
	`collision`   TEXT         NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;

CREATE TABLE IF NOT EXISTS `work_units` (
	`id`          INT(10)      NOT NULL AUTO_INCREMENT PRIMARY KEY,
	`crack_id`    INT(10)      NOT NULL,
	`hostname`    VARCHAR(255) NOT NULL,
	`devtype`     VARCHAR(16)  NOT NULL,
	`devid`       INT(11)      NOT NULL,
	`start`       VARCHAR(255) NOT NULL,
	`end`         VARCHAR(255) NOT NULL,
	`started`     INT(11)      NOT NULL,
	`expiration`  INT(11)      NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;

-- A memory table must be better than a MyISAM table. History has lots of access
-- but no essential data
-- UNIQUE KEY `time` (`time`,`speed`)
CREATE TABLE IF NOT EXISTS `history` (
	`id`          INT(10)      NOT NULL AUTO_INCREMENT PRIMARY KEY,
	`time`        INT(11)      NOT NULL,
	`speed`       FLOAT        NOT NULL
) ENGINE=MEMORY DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;

CREATE TABLE IF NOT EXISTS `stats` (
	`id`          INT(10)      NOT NULL AUTO_INCREMENT PRIMARY KEY,
	`updated`     INT(11)      NOT NULL,
	`device`      VARCHAR(255) NOT NULL,
	`time`        INT(11)      NOT NULL,
	`speed`       FLOAT        NOT NULL
) ENGINE=MEMROY DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;

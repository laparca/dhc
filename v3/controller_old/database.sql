CREATE TABLE IF NOT EXISTS `cracks` (
	`id`          INT(10)      NOT NULL AUTO_INCREMENT,
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
	`updated`     INT(11)      NOT NULL,
	
	PRIMARY KEY  (`id`)
	
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci AUTO_INCREMENT=1;

CREATE TABLE IF NOT EXISTS `stats` (
	`updated`     INT(11) NOT NULL,
	`device`      VARCHAR(255) collate latin1_general_ci NOT NULL,
	`time`        INT(11) NOT NULL,
	`speed`       FLOAT NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;

CREATE TABLE IF NOT EXISTS `workunits` (
	`id`          INT(11) NOT NULL AUTO_INCREMENT,
	`crack`       INT(11) NOT NULL,
	`hostname`    VARCHAR(255) collate latin1_general_ci NOT NULL,
	`devtype`     VARCHAR(16) collate latin1_general_ci NOT NULL,
	`devid`       INT(11) NOT NULL,
	`start`       VARCHAR(255) collate latin1_general_ci NOT NULL,
	`end`         VARCHAR(255) collate latin1_general_ci NOT NULL,
	`started`     INT(11) NOT NULL,
	`expiration`  INT(11) NOT NULL,

	PRIMARY KEY  (`id`)

) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci AUTO_INCREMENT=1;

CREATE TABLE IF NOT EXISTS `history` (
	`time`        INT(11) NOT NULL,
	`speed`       FLOAT NOT NULL,

	UNIQUE KEY `time` (`time`,`speed`)

) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;

CREATE TABLE IF NOT EXISTS `hashes` (
	`id`          INT(11) NOT NULL AUTO_INCREMENT,
	`active`      INT(11) NOT NULL,
	`crack`       INT(11) NOT NULL,
	`hash`        TEXT collate latin1_general_ci NOT NULL,
	`collision`   TEXT collate latin1_general_ci NOT NULL,

	PRIMARY KEY (`id`)

) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;
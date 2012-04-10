DROP TABLE IF EXISTS `game_event`;
CREATE TABLE `game_event` (
  `event_id` int(10) unsigned NOT NULL COMMENT 'Unique Identifier for the event',
  `start_time` int(10) unsigned NOT NULL COMMENT 'seconds since 1970-01-01 00:00:00 till event starts',
  `length` int(10) unsigned NOT NULL COMMENT 'length of the event in seconds',
  `occurence` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'time from event start till next start in seconds',
  `name` varchar(255) NOT NULL COMMENT 'A Name to describe the event',
  PRIMARY KEY (`event_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='Event Definitions for GameEventMgr';

DROP TABLE IF EXISTS `game_event_creature_spawns`;
CREATE TABLE `game_event_creature_spawns` (
  `event_id` int(10) unsigned NOT NULL,
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `entry` int(30) NOT NULL,
  `map` int(30) NOT NULL,
  `position_x` float NOT NULL,
  `position_y` float NOT NULL,
  `position_z` float NOT NULL,
  `orientation` float NOT NULL,
  `movetype` int(30) NOT NULL DEFAULT '0',
  `displayid` int(30) unsigned NOT NULL DEFAULT '0',
  `faction` int(30) NOT NULL DEFAULT '14',
  `flags` int(30) NOT NULL DEFAULT '0',
  `bytes0` int(30) NOT NULL DEFAULT '0',
  `bytes1` int(30) NOT NULL DEFAULT '0',
  `bytes2` int(30) NOT NULL DEFAULT '0',
  `emote_state` int(30) NOT NULL DEFAULT '0',
  `npc_respawn_link` int(30) NOT NULL DEFAULT '0',
  `channel_spell` int(30) NOT NULL DEFAULT '0',
  `channel_target_sqlid` int(30) NOT NULL DEFAULT '0',
  `channel_target_sqlid_creature` int(30) NOT NULL DEFAULT '0',
  `standstate` int(10) NOT NULL DEFAULT '0',
  `death_state` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `mountdisplayid` int(10) unsigned NOT NULL DEFAULT '0',
  `slot1item` int(10) unsigned NOT NULL DEFAULT '0',
  `slot2item` int(10) unsigned NOT NULL DEFAULT '0',
  `slot3item` int(10) unsigned NOT NULL DEFAULT '0',
  `CanFly` smallint(3) NOT NULL DEFAULT '0',
  `phase` int(10) unsigned NOT NULL DEFAULT '1',
  PRIMARY KEY (`event_id`,`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='Creature Spawns for Events';

DROP TABLE IF EXISTS `game_event_gameobject_spawns`;
CREATE TABLE `game_event_gameobject_spawns` (
  `event_id` int(10) unsigned NOT NULL,
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `Entry` int(10) unsigned NOT NULL DEFAULT '0',
  `map` int(10) unsigned NOT NULL DEFAULT '0',
  `position_x` float NOT NULL DEFAULT '0',
  `position_y` float NOT NULL DEFAULT '0',
  `position_z` float NOT NULL DEFAULT '0',
  `Facing` float NOT NULL DEFAULT '0',
  `orientation1` float NOT NULL DEFAULT '0',
  `orientation2` float NOT NULL DEFAULT '0',
  `orientation3` float NOT NULL DEFAULT '0',
  `orientation4` float NOT NULL DEFAULT '0',
  `State` int(10) unsigned NOT NULL DEFAULT '0',
  `Flags` int(10) unsigned NOT NULL DEFAULT '0',
  `Faction` int(10) unsigned NOT NULL DEFAULT '0',
  `Scale` float NOT NULL DEFAULT '0',
  `stateNpcLink` int(10) unsigned NOT NULL DEFAULT '0',
  `phase` int(10) unsigned NOT NULL DEFAULT '1',
  `overrides` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`event_id`,`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='GameObject Spawns for Events';

DROP TABLE IF EXISTS `game_event_creature_waypoints`;
CREATE TABLE `game_event_creature_waypoints` (
  `event_id` int(10) unsigned NOT NULL,
  `spawnid` int(10) unsigned NOT NULL DEFAULT '0',
  `waypointid` int(10) unsigned NOT NULL DEFAULT '0',
  `position_x` float NOT NULL DEFAULT '0',
  `position_y` float NOT NULL DEFAULT '0',
  `position_z` float NOT NULL DEFAULT '0',
  `waittime` int(10) unsigned NOT NULL DEFAULT '0',
  `flags` int(10) unsigned NOT NULL DEFAULT '0',
  `forwardemoteoneshot` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `forwardemoteid` int(10) unsigned NOT NULL DEFAULT '0',
  `backwardemoteoneshot` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `backwardemoteid` int(10) unsigned NOT NULL DEFAULT '0',
  `forwardskinid` int(10) unsigned NOT NULL DEFAULT '0',
  `backwardskinid` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`event_id`,`spawnid`,`waypointid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='Creature Waypoints for Events';

DROP TABLE IF EXISTS `game_event_scripts`;
CREATE TABLE `game_event_scripts` (
  `event_id` int(10) unsigned NOT NULL,
  `sql_id` int(10) unsigned NOT NULL,
  `type` tinyint(3) unsigned NOT NULL,
  `data_1` int(10) unsigned NOT NULL,
  `data_2` int(10) unsigned NOT NULL,
  `data_3` int(10) unsigned NOT NULL,
  `say_on_event_start` varchar(255) DEFAULT "",
  UNIQUE KEY `event_id` (`event_id`,`sql_id`,`type`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

ALTER TABLE `quests` ADD `event_id` INT UNSIGNED NOT NULL DEFAULT '0';

-- MySQL dump 10.15  Distrib 10.0.21-MariaDB, for Linux (x86_64)
--
-- Host: localhost    Database: aqi
-- ------------------------------------------------------
-- Server version	10.0.21-MariaDB

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `aqi_data`
--

DROP TABLE IF EXISTS `aqi_data`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `aqi_data` (
  `station_id` char(3) NOT NULL,
  `create_time` datetime NOT NULL,
  `o3` double DEFAULT NULL,
  `o3_aqi` int(10) unsigned DEFAULT NULL,
  `co` double(10,6) DEFAULT NULL,
  `co_aqi` int(10) unsigned DEFAULT NULL,
  `so2` double DEFAULT NULL,
  `so2_aqi` int(10) unsigned DEFAULT NULL,
  `no2` double DEFAULT NULL,
  `no2_aqi` int(10) unsigned DEFAULT NULL,
  `pm10` double DEFAULT NULL,
  `pm10_aqi` int(10) unsigned DEFAULT NULL,
  `pm25` double DEFAULT NULL,
  `pm25_aqi` int(10) unsigned DEFAULT NULL,
  `aqi` int(10) unsigned DEFAULT NULL,
  PRIMARY KEY (`create_time`,`station_id`),
  KEY `FD_STATION_ID` (`station_id`),
  CONSTRAINT `FD_STATION_ID` FOREIGN KEY (`station_id`) REFERENCES `station` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `aqi_data`
--

LOCK TABLES `aqi_data` WRITE;
/*!40000 ALTER TABLE `aqi_data` DISABLE KEYS */;
/*!40000 ALTER TABLE `aqi_data` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `station`
--

DROP TABLE IF EXISTS `station`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `station` (
  `id` char(3) NOT NULL DEFAULT '',
  `name` varchar(128) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `station`
--

LOCK TABLES `station` WRITE;
/*!40000 ALTER TABLE `station` DISABLE KEYS */;
INSERT INTO `station` VALUES ('0','全市'),('1','市检测站'),('2','烟厂'),('3','河医大'),('4','郑纺机'),('5','银行学校'),('6','供水公司'),('7','岗里水库'),('8','经开区管委'),('9','四十七中');
/*!40000 ALTER TABLE `station` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Temporary table structure for view `station_aqi`
--

DROP TABLE IF EXISTS `station_aqi`;
/*!50001 DROP VIEW IF EXISTS `station_aqi`*/;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
/*!50001 CREATE TABLE `station_aqi` (
  `station_name` tinyint NOT NULL,
  `station_id` tinyint NOT NULL,
  `create_time` tinyint NOT NULL,
  `aqi` tinyint NOT NULL,
  `o3` tinyint NOT NULL,
  `o3_aqi` tinyint NOT NULL,
  `co` tinyint NOT NULL,
  `co_aqi` tinyint NOT NULL,
  `so2` tinyint NOT NULL,
  `so2_aqi` tinyint NOT NULL,
  `no2` tinyint NOT NULL,
  `no2_aqi` tinyint NOT NULL,
  `pm10` tinyint NOT NULL,
  `pm10_aqi` tinyint NOT NULL,
  `pm25` tinyint NOT NULL,
  `pm25_aqi` tinyint NOT NULL
) ENGINE=MyISAM */;
SET character_set_client = @saved_cs_client;

--
-- Final view structure for view `station_aqi`
--

/*!50001 DROP TABLE IF EXISTS `station_aqi`*/;
/*!50001 DROP VIEW IF EXISTS `station_aqi`*/;
/*!50001 SET @saved_cs_client          = @@character_set_client */;
/*!50001 SET @saved_cs_results         = @@character_set_results */;
/*!50001 SET @saved_col_connection     = @@collation_connection */;
/*!50001 SET character_set_client      = utf8 */;
/*!50001 SET character_set_results     = utf8 */;
/*!50001 SET collation_connection      = utf8_general_ci */;
/*!50001 CREATE ALGORITHM=UNDEFINED */
/*!50013 DEFINER=`root`@`localhost` SQL SECURITY DEFINER */
/*!50001 VIEW `station_aqi` AS select `station`.`name` AS `station_name`,`station`.`id` AS `station_id`,`aqi_data`.`create_time` AS `create_time`,`aqi_data`.`aqi` AS `aqi`,`aqi_data`.`o3` AS `o3`,`aqi_data`.`o3_aqi` AS `o3_aqi`,`aqi_data`.`co` AS `co`,`aqi_data`.`co_aqi` AS `co_aqi`,`aqi_data`.`so2` AS `so2`,`aqi_data`.`so2_aqi` AS `so2_aqi`,`aqi_data`.`no2` AS `no2`,`aqi_data`.`no2_aqi` AS `no2_aqi`,`aqi_data`.`pm10` AS `pm10`,`aqi_data`.`pm10_aqi` AS `pm10_aqi`,`aqi_data`.`pm25` AS `pm25`,`aqi_data`.`pm25_aqi` AS `pm25_aqi` from (`station` join `aqi_data` on((`station`.`id` = `aqi_data`.`station_id`))) */
/*!50002 WITH CASCADED CHECK OPTION */;
/*!50001 SET character_set_client      = @saved_cs_client */;
/*!50001 SET character_set_results     = @saved_cs_results */;
/*!50001 SET collation_connection      = @saved_col_connection */;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2015-12-25 13:54:42

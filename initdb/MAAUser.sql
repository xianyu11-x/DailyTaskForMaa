/*
 Navicat Premium Dump SQL

 Source Server         : wsl-mysql
 Source Server Type    : MySQL
 Source Server Version : 90100 (9.1.0)
 Source Host           : 172.23.17.52:3306
 Source Schema         : MAABackendDB

 Target Server Type    : MySQL
 Target Server Version : 90100 (9.1.0)
 File Encoding         : 65001

 Date: 28/12/2024 14:01:58
*/

SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- ----------------------------
-- Table structure for MAAUser
-- ----------------------------
DROP TABLE IF EXISTS `MAAUser`;
CREATE TABLE `MAAUser`  (
  `userID` char(255) CHARACTER SET utf16 COLLATE utf16_general_ci NOT NULL,
  `deviceID` char(255) CHARACTER SET utf16 COLLATE utf16_general_ci NOT NULL,
  `dailyTaskTime` datetime NULL DEFAULT NULL,
  `taskStartTime` datetime NULL DEFAULT NULL,
  `taskEndTime` datetime NULL DEFAULT NULL,
  `taskStrategy` json NULL,
  `dailyTaskID` char(255) CHARACTER SET utf16 COLLATE utf16_general_ci NULL DEFAULT NULL,
  PRIMARY KEY (`userID`, `deviceID` DESC) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf16 COLLATE = utf16_general_ci ROW_FORMAT = Dynamic;

SET FOREIGN_KEY_CHECKS = 1;

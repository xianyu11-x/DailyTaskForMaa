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

 Date: 23/01/2025 01:39:53
*/

SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- ----------------------------
-- Table structure for MAAAction
-- ----------------------------
DROP TABLE IF EXISTS `MAAAction`;
CREATE TABLE `MAAAction`  (
  `actionID` char(255) CHARACTER SET utf16 COLLATE utf16_general_ci NOT NULL,
  `taskID` char(255) CHARACTER SET utf16 COLLATE utf16_general_ci NOT NULL,
  `actionIsFinish` char(10) CHARACTER SET utf16 COLLATE utf16_general_ci NOT NULL,
  PRIMARY KEY (`actionID`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf16 COLLATE = utf16_general_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Table structure for MAADailyTaskPlan
-- ----------------------------
DROP TABLE IF EXISTS `MAADailyTaskPlan`;
CREATE TABLE `MAADailyTaskPlan`  (
  `id` int NOT NULL AUTO_INCREMENT,
  `planID` char(255) CHARACTER SET utf16 COLLATE utf16_general_ci NOT NULL,
  `userID` char(255) CHARACTER SET utf16 COLLATE utf16_general_ci NOT NULL,
  `deviceID` char(255) CHARACTER SET utf16 COLLATE utf16_general_ci NOT NULL,
  `dailyTaskStrategy` json NULL,
  `dailyTaskTime` varchar(255) CHARACTER SET utf16 COLLATE utf16_general_ci NULL DEFAULT NULL,
  PRIMARY KEY (`id`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf16 COLLATE = utf16_general_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Table structure for MAAQuickTask
-- ----------------------------
DROP TABLE IF EXISTS `MAAQuickTask`;
CREATE TABLE `MAAQuickTask`  (
  `taskID` char(255) CHARACTER SET utf16 COLLATE utf16_general_ci NOT NULL,
  `userID` char(255) CHARACTER SET utf16 COLLATE utf16_general_ci NOT NULL,
  `deviceID` char(255) CHARACTER SET utf16 COLLATE utf16_general_ci NOT NULL,
  `taskCommitTime` datetime NULL DEFAULT NULL,
  `taskStartTime` datetime NULL DEFAULT NULL,
  `taskIsFinish` char(10) CHARACTER SET utf16 COLLATE utf16_general_ci NULL DEFAULT NULL,
  `taskActions` json NULL,
  PRIMARY KEY (`taskID` DESC) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf16 COLLATE = utf16_general_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Table structure for MAAUser
-- ----------------------------
DROP TABLE IF EXISTS `MAAUser`;
CREATE TABLE `MAAUser`  (
  `userID` char(255) CHARACTER SET utf16 COLLATE utf16_general_ci NOT NULL,
  `deviceID` char(255) CHARACTER SET utf16 COLLATE utf16_general_ci NOT NULL,
  `nextDailyTaskTime` datetime NULL DEFAULT NULL,
  `dailyTaskStartTime` datetime NULL DEFAULT NULL,
  `dailyTaskEndTime` datetime NULL DEFAULT NULL,
  `dailyTaskID` int NULL DEFAULT NULL,
  PRIMARY KEY (`userID` DESC, `deviceID` DESC) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf16 COLLATE = utf16_general_ci ROW_FORMAT = Dynamic;

SET FOREIGN_KEY_CHECKS = 1;

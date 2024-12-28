#!/bin/bash

# 设置一个标识，用于向 crontab 中添加/移除
CRON_JOB="0 5 * * * $(realpath "$0")"

if [ "$1" = "install" ]; then
  # 安装定时任务
  ( crontab -l 2>/dev/null | grep -v "$(realpath "$0")" ; echo "$CRON_JOB" ) | crontab -
  echo "已添加定时任务：每日凌晨 5 点执行 $(realpath "$0")"
  exit 0
elif [ "$1" = "uninstall" ]; then
  # 卸载定时任务
  ( crontab -l 2>/dev/null | grep -v "$(realpath "$0")" ) | crontab -
  echo "已卸载定时任务。"
  exit 0
fi

# 如果未传递 install 或 uninstall 参数，则执行更新逻辑
JSON_URL="https://raw.githubusercontent.com/MaaAssistantArknights/MaaResource/refs/heads/main/cache/gui/StageActivity.json"
POST_URL="http://0.0.0.0:8080/maa/updateSideStory"
TEMP_FILE="/tmp/temp_download.json"

# 下载 JSON 文件
echo "正在下载 JSON 文件：$JSON_URL"
curl -s -o "$TEMP_FILE" "$JSON_URL"

# 将 JSON 文件内容通过 POST 推送致服务器
echo "正在推送 JSON 文件：$POST_URL"
curl -X POST "$POST_URL" \
     -H "Content-Type: application/json" \
     --data-binary @"$TEMP_FILE"

# 清理临时文件
rm -f "$TEMP_FILE"
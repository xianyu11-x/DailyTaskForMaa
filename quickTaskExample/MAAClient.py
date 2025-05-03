import os
import json
import sys
import requests
import argparse
from pathlib import Path


def main():
    # 使用argparse处理命令行参数
    parser = argparse.ArgumentParser(description='发送MAA任务或策略更新请求')
    parser.add_argument('-u', '--user', required=True, help='用户名')
    parser.add_argument('-d', '--device', required=True, help='设备名称')
    parser.add_argument('-f', '--file', required=True, help='任务或策略JSON文件名')
    parser.add_argument('-m', '--mode', required=True, choices=['task', 'strategy'], 
                        help='请求模式: task (任务请求) 或 strategy (策略更新)')
    parser.add_argument('--url', default='192.168.137.134', 
                        help='请求的服务器IP地址 (默认: 192.168.137.134)')
    
    args = parser.parse_args()
    
    # 获取参数
    user = args.user
    device = args.device
    json_file = args.file
    mode = args.mode
    
    # 根据模式确定API端点
    if mode == 'task':
        endpoint = "/maa/quickTask"
    else:  # strategy
        endpoint = "/maa/updateStrategy"
        
    # 构建完整URL
    server_ip = args.url
    url = f"http://{server_ip}:8080{endpoint}"
    
    # 构建JSON文件路径（当前脚本目录下的指定JSON文件）
    current_dir = Path(__file__).parent
    json_path = current_dir / f"{json_file}"
    
    try:
        # 读取JSON文件
        with open(json_path, 'r', encoding='utf-8') as f:
            data = json.load(f)
        
        # 替换user和device字段
        if isinstance(data, dict):
            if "user" in data:
                data["user"] = user
            if "device" in data:
                data["device"] = device
        
        # 发送POST请求
        headers = {'Content-Type': 'application/json'}
        
        print(f"正在发送{mode}请求到 {url}")
        response = requests.post(url, json=data, headers=headers)
        
        # 处理响应
        if response.status_code == 200:
            print(f"请求成功: {response.status_code}")
            print(f"响应内容: {response.text}")
        else:
            print(f"请求失败: {response.status_code}")
            print(f"错误信息: {response.text}")
            
    except FileNotFoundError:
        print(f"错误: 找不到JSON文件 '{json_path}'")
    except json.JSONDecodeError:
        print(f"错误: '{json_path}' 不是有效的JSON文件")
    except requests.RequestException as e:
        print(f"请求错误: {e}")


if __name__ == "__main__":
    main()
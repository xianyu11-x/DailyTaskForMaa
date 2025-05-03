import os
import json
import sys
import requests
import argparse
from pathlib import Path

def main():
    # 使用argparse处理命令行参数
    parser = argparse.ArgumentParser(description='发送MAA策略更新请求')
    parser.add_argument('-u', '--user', required=True, help='用户名')
    parser.add_argument('-d', '--device', required=True, help='设备名称')
    parser.add_argument('-s', '--strategy', required=True, help='策略JSON文件名')
    parser.add_argument('--url', default='192.168.137.134', 
                        help='请求的服务器IP地址 (默认: 192.168.137.134)')
    
    args = parser.parse_args()
    
    # 获取参数
    user = args.user
    device = args.device
    strategy_file = args.strategy
    
    # 构建完整URL
    server_ip = args.url
    url = f"http://{server_ip}:8080/maa/updateStrategy"
    
    # 构建JSON文件路径（当前脚本目录下的指定JSON文件）
    current_dir = Path(__file__).parent
    json_path = current_dir / f"{strategy_file}"
    
    try:
        # 读取JSON文件
        with open(json_path, 'r', encoding='utf-8') as f:
            strategy_data = json.load(f)
        
        # 替换user和device字段
        if isinstance(strategy_data, dict):
            if "user" in strategy_data:
                strategy_data["user"] = user
            if "device" in strategy_data:
                strategy_data["device"] = device
        
        # 发送POST请求
        headers = {'Content-Type': 'application/json'}
        
        response = requests.post(url, json=strategy_data, headers=headers)
        
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
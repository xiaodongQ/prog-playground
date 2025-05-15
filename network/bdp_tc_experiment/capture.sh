#!/bin/bash

# 变量设置
PORT=8000
CAPTURE_DIR="./captures"
CAPTURE_TYPE=""
SCENARIO=""

# 定义场景与描述的映射关系
declare -A SCENARIOS=(
    [1]="delay-2ms"
    [2]="delay-100ms"
    [3]="loss-1"
    [4]="loss-10"
    [5]="duplicate-1"
    [6]="duplicate-10"
    [7]="corrupt-1"
    [8]="corrupt-10"
    [9]="reorder-1"
    [10]="reorder-20"
    [11]="bd-400mb"
    [12]="bd-8mb"
)

# 场景编号对应中文说明（用于提示）
declare -A SCENARIO_DESC=(
    [1]="客户端延时 2ms"
    [2]="客户端延时 100ms"
    [3]="服务端丢包 1%"
    [4]="服务端丢包 10%"
    [5]="服务端包重复 1%"
    [6]="服务端包重复 10%"
    [7]="服务端包损坏 1%"
    [8]="服务端包损坏 10%"
    [9]="服务端包乱序 1%"
    [10]="服务端包乱序 20%"
    [11]="限制带宽 400Mbps (50MBps)"
	[12]="限制带宽 8Mbps (1MBps)"
)

# 参数检查
if [ "$#" -ne 2 ]; then
    echo "用法: $0 <capture_type> <scenario_number>"
    echo "capture_type: cli 或 server"
    echo "scenario_number: 1 到 12 的整数，代表以下场景："
    echo "--------------------"
    for i in {1..12}; do
        echo "$i. ${SCENARIO_DESC[$i]}"
    done
    echo "--------------------"
    exit 1
fi

# 获取参数
CAPTURE_TYPE=$1
SCENARIO_NUMBER=$2

# 根据编号获取具体场景
SCENARIO=${SCENARIOS[$SCENARIO_NUMBER]}
if [ -z "$SCENARIO" ]; then
    echo "无效的 scenario_number 参数: $SCENARIO_NUMBER (应为 1 到 12)"
    exit 1
fi

# 创建抓包目录
mkdir -p "$CAPTURE_DIR"

# 根据输入生成对应的抓包命令和文件名
if [ "$CAPTURE_TYPE" == "cli" ]; then
    CAPTURE_FILE="${CAPTURE_DIR}/${SCENARIO}_cli.cap"
elif [ "$CAPTURE_TYPE" == "server" ]; then
    CAPTURE_FILE="${CAPTURE_DIR}/${SCENARIO}_server.cap"
else
    echo "无效的 capture_type 参数: $CAPTURE_TYPE (应为 cli 或 server)"
    exit 1
fi

# 执行抓包命令
echo "开始抓包... (请按 Ctrl+C 停止)"
tcpdump -i any port $PORT -s 100 -v -w $CAPTURE_FILE

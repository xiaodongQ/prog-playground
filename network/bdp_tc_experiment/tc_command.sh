#!/bin/bash

# 网卡设备名，请根据实际情况修改
DEV="eth0"

show_menu() {
    echo "================= tc 测试工具菜单 ================="
    echo "1. 添加客户端延时 2ms"
    echo "2. 修改客户端延时为 100ms"
    echo "3. 模拟服务端丢包 1%"
    echo "4. 修改丢包率为 10%"
    echo "5. 模拟服务端包重复 1%"
    echo "6. 修改包重复率为 10%"
    echo "7. 模拟服务端包损坏 1%"
    echo "8. 修改包损坏率为 10%"
    echo "9. 模拟服务端包乱序：1% 乱序，延迟 100ms，相关性 10%"
    echo "10. 修改乱序率为 20%"
    echo "11. 限制服务端带宽为 400Mbps (50MBps)"
	echo "12. 限制服务端带宽为 8Mbps (1MBps)"
    echo "13. 清除所有规则"
    echo "0. 退出"
    echo "=================================================="
}

clear_rules() {
    tc qdisc del dev $DEV root 2>/dev/null
    echo "已清除 $DEV 上的所有 tc 规则"
}

execute_choice() {
    case $1 in
        1) clear_rules; tc qdisc add dev $DEV root netem delay 2ms;;
        2) tc qdisc change dev $DEV root netem delay 100ms;;
        3) clear_rules; tc qdisc add dev $DEV root netem loss 1%;;
        4) tc qdisc change dev $DEV root netem loss 10%;;
        5) clear_rules; tc qdisc add dev $DEV root netem duplicate 1%;;
        6) tc qdisc change dev $DEV root netem duplicate 10%;;
        7) clear_rules; tc qdisc add dev $DEV root netem corrupt 1%;;
        8) tc qdisc change dev $DEV root netem corrupt 10%;;
        9) clear_rules; tc qdisc add dev $DEV root netem delay 100ms reorder 99% 10%;;
       10) tc qdisc change dev $DEV root netem delay 100ms reorder 80% 10%;;
       11) clear_rules; tc qdisc add dev $DEV root tbf rate 400mbit burst 10mbit latency 10ms;;
       12) clear_rules; tc qdisc add dev $DEV root tbf rate 8mbit burst 200kbit latency 10ms;;
       13) clear_rules;;
        0) echo "退出脚本"; exit 0;;
        *) echo "无效选项" ;;
    esac
}

# 主程序
while true; do
    show_menu
    read -p "请输入选项编号: " choice
    execute_choice "$choice"
    echo "操作完成。"
    read -p "按任意键继续..."
    clear
done

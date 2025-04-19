#!/bin/bash

# gen_diff_flamegraph.sh - 生成差分火焰图对
# 用法: ./gen_diff_flamegraph.sh 旧版本.folded 新版本.folded

# 参数检查
if [ $# -ne 2 ]; then
    echo "错误：必须指定两个折叠文件"
    echo "用法: $0 <基准文件> <对比文件>"
    echo "示例: $0 v1.folded v2.folded"
    exit 1
fi

BASE_FILE=$1  # 基准文件（旧版本）
COMPARE_FILE=$2  # 对比文件（新版本）

# 文件存在性检查
for file in $BASE_FILE $COMPARE_FILE; do
    if [ ! -f "$file" ]; then
        echo "错误：输入文件 $file 不存在"
        exit 1
    fi
done

# 工具依赖检查
for cmd in difffolded.pl flamegraph.pl; do
    if ! command -v $cmd &> /dev/null; then
        echo "错误：缺少依赖工具 $cmd"
        echo "请从以下地址安装 FlameGraph 工具集："
        echo "https://github.com/brendangregg/FlameGraph"
        exit 1
    fi
done

# 生成主对比图（新版本变化视图）
echo -e "\033[34m▶ 生成主对比图（显示新版本的变化）...\033[0m"
difffolded.pl -n $BASE_FILE $COMPARE_FILE | \
flamegraph.pl --title "Differential $COMPARE_FILE vs $BASE_FILE" > diff_${COMPARE_FILE}_vs_${BASE_FILE}.svg

# 生成互补视图（捕获旧版本特有内容）
echo -e "\033[34m▶ 生成互补视图（避免遗漏基准版本的路径变化）...\033[0m"
difffolded.pl -n $COMPARE_FILE $BASE_FILE | \
flamegraph.pl --negate --title "Complementary $BASE_FILE vs $COMPARE_FILE" > diff_${BASE_FILE}_complement.svg

# 生成结果说明
echo -e "\n\033[32m✔ 生成完成！请按以下方式分析：\033[0m"
echo "----------------------------------------"
echo "文件1：diff_${COMPARE_FILE}_vs_${BASE_FILE}.svg"
echo "  作用：查看新版本的主要变化"
echo "  颜色说明："
echo "    █ 红色区域 → 新版本耗时增加"
echo "    █ 蓝色区域 → 新版本耗时减少"
echo "----------------------------------------"
echo "文件2：diff_${BASE_FILE}_complement.svg"
echo "  作用：避免遗漏新版本缺失的调用栈"
echo "  颜色说明："
echo "    █ 红色区域 → 新版本耗时增加"
echo "    █ 蓝色区域 → 新版本耗时减少或者消失的代码路径"
echo "----------------------------------------"
echo -e "\033[33m★ 分析建议：\033[0m"
echo "1. 优先查看主视图的红色热点（性能衰退点）"
echo "2. 检查互补视图的蓝色区域是否预期内的代码删除"
echo "3. 在浏览器中使用 Ctrl+F 跨文件搜索关键函数"
echo "4. 结合代码变更记录分析变化原因"

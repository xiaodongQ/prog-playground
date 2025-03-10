#!/bin/bash

# 定义运行次数
RUN_TIMES=10
# 已知上下文切换次数
CONTEXT_SWITCH_COUNT=20000

total_time_diff=0

for ((i = 0; i < RUN_TIMES; i++)); do
    # 运行程序并捕获输出
    output=$(./a.out)

    # 提取开始时间和结束时间的秒和微秒部分
    before_sec=$(echo "$output" | grep "Before Context Switch Time" | awk '{print $5}')
    before_usec=$(echo "$output" | grep "Before Context Switch Time" | awk '{print $7}')
    after_sec=$(echo "$output" | grep "After Context SWitch Time" | awk '{print $5}')
    after_usec=$(echo "$output" | grep "After Context SWitch Time" | awk '{print $7}')

    # 计算时间差
    sec_diff=$((after_sec - before_sec))
    usec_diff=$((after_usec - before_usec))
    time_diff=$((sec_diff * 1000000 + usec_diff))

    # 累加总时间差
    total_time_diff=$((total_time_diff + time_diff))

    echo "Run $((i + 1)): Time difference = $time_diff microseconds"
done

# 计算平均时间差，并使用 bc 保留两位小数
average_time_diff=$(echo "scale=2; $total_time_diff / $RUN_TIMES" | bc)

# 计算平均上下文切换时间，并使用 bc 保留两位小数
average_context_switch_time=$(echo "scale=2; $average_time_diff / $CONTEXT_SWITCH_COUNT" | bc)

echo "Average time difference over $RUN_TIMES runs: $average_time_diff microseconds"
echo "Average context switch time: $average_context_switch_time microseconds"

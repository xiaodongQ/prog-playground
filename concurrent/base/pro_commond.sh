#!/bin/bash

# 定义顶层结果目录
TOP_RESULT_DIR="results"

# 定义 gperftools 报告目录
GPERFTOOLS_DIR="$TOP_RESULT_DIR/gperftools_report"

function run_program() {
    echo "启动程序并等待回车..."
    mkdir -p "$GPERFTOOLS_DIR"
    CPUPROFILE="$GPERFTOOLS_DIR/prof.out" ./thread_pool_withwait
}

function run_perf_stat() {
    PID=$(pidof thread_pool_withwait)
    if [ -z "$PID" ]; then
        echo "未找到 thread_pool_withwait 进程"
        return
    fi
    PERF_STAT_DIR="$TOP_RESULT_DIR/perf_stat_result"
    mkdir -p $PERF_STAT_DIR
    echo "执行 perf stat..."
    perf stat -p $PID > $PERF_STAT_DIR/perf_stat.out 2>&1
}

function run_perf_record_and_generate_flamegraphs() {
    PID=$(pidof thread_pool_withwait)
    if [ -z "$PID" ]; then
        echo "未找到 thread_pool_withwait 进程"
        return
    fi
    REGULAR_DIR="$TOP_RESULT_DIR/regular_flamegraph"
    mkdir -p $REGULAR_DIR
    echo "执行 perf record..."
    perf record -a -g -p$PID -o "$REGULAR_DIR/perf.data"
    read -p "perf record 执行完成，按回车键生成常规火焰图..."
    echo "生成常规火焰图..."
    perf script -i "$REGULAR_DIR/perf.data" | stackcollapse-perf.pl | flamegraph.pl > "$REGULAR_DIR/on_cpu.svg"
    perf script -i "$REGULAR_DIR/perf.data" | stackcollapse-perf.pl | flamegraph.pl --reverse --inverted > "$REGULAR_DIR/on_cpu_icicle.svg"
}

function generate_wakeup_flamegraphs() {
    WAKEUP_DIR="$TOP_RESULT_DIR/wakeup_flamegraph"
    mkdir -p $WAKEUP_DIR
    PID=$(pidof thread_pool_withwait)
    if [ -z "$PID" ]; then
        echo "未找到 thread_pool_withwait 进程"
        return
    fi
    echo "生成 wakeup 火焰图...(程序执行完成后打断采集，会自动生成)"
    /usr/share/bcc/tools/wakeuptime -f -p$PID -f > $WAKEUP_DIR/out.stacks
    cd $WAKEUP_DIR
    flamegraph.pl --color=wakeup --title="Wakeup Time Flame Graph" --countname=us < out.stacks > wakeup.svg
    flamegraph.pl --color=wakeup --title="Wakeup Time Flame Graph" --countname=us --reverse --inverted < out.stacks > wakeup_icicle.svg
    cd ..
}

function generate_offwaketime_flamegraph() {
    OFFWAKEUP_DIR="$TOP_RESULT_DIR/offwaketime_flamegraph"
    mkdir -p $OFFWAKEUP_DIR
    PID=$(pidof thread_pool_withwait)
    if [ -z "$PID" ]; then
        echo "未找到 thread_pool_withwait 进程"
        return
    fi
    echo "生成 offwaketime 火焰图...(程序执行完成后打断采集，会自动生成)"
    /usr/share/bcc/tools/offwaketime -f -p$PID > $OFFWAKEUP_DIR/out.stacks
    cd $OFFWAKEUP_DIR
    flamegraph.pl --color=chain --title="Off - Wake Time Flame Graph" --countname=us < out.stacks > offwaketime_out.svg
    cd ..
}

function generate_gperftools_report() {
    mkdir -p "$GPERFTOOLS_DIR"
    echo "生成 gperftools 报告..."
    pprof --pdf ./thread_pool_withwait "$GPERFTOOLS_DIR/prof.out" > "$GPERFTOOLS_DIR/prof.pdf"
}

function clean_results() {
    if [ -d "$TOP_RESULT_DIR" ]; then
        echo "正在清理目录: $TOP_RESULT_DIR"
        rm -rf "$TOP_RESULT_DIR"
    fi
    echo "所有结果目录已清理完毕。"
}

function show_help() {
    echo "用法: $0 [选项]"
    echo "选项:"
    echo "  -h, --help                        显示此帮助信息"
    echo "  -r, --run-program                 启动程序并等待回车"
    echo "  -s, --run-perf-stat               执行 perf stat 并保存结果"
    echo "  -c, --run-perf-record-and-flame   执行 perf record 并生成常规火焰图"
    echo "  -w, --generate-wakeup-flamegraphs 生成 wakeup 火焰图"
    echo "  -o, --generate-offwaketime-flamegraph 生成 offwaketime 火焰图"
    echo "  -g, --generate-gperftools-report  生成 gperftools 报告"
    echo "  -x, --clean-results               清理所有结果目录"
}

if [ $# -eq 0 ]; then
    show_help
    exit 1
fi

while [ $# -gt 0 ]; do
    case "$1" in
        -h|--help)
            show_help
            shift
            ;;
        -r|--run-program)
            run_program
            shift
            ;;
        -s|--run-perf-stat)
            run_perf_stat
            shift
            ;;
        -c|--run-perf-record-and-flame)
            run_perf_record_and_generate_flamegraphs
            shift
            ;;
        -w|--generate-wakeup-flamegraphs)
            generate_wakeup_flamegraphs
            shift
            ;;
        -o|--generate-offwaketime-flamegraph)
            generate_offwaketime_flamegraph
            shift
            ;;
        -g|--generate-gperftools-report)
            generate_gperftools_report
            shift
            ;;
        -x|--clean-results)
            clean_results
            shift
            ;;
        *)
            echo "无效的选项: $1"
            show_help
            exit 1
            ;;
    esac
done

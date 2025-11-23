#!/bin/bash

# iOS 测试脚本
# 用于在 iOS 模拟器上运行各种测试

set -e  # 遇到错误时退出

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# 设置 Xcode 开发者目录
export DEVELOPER_DIR=/Applications/Xcode.app/Contents/Developer

# 默认模拟器
DEFAULT_SIMULATOR="iPhone 16 Pro"
SIMULATOR="${2:-$DEFAULT_SIMULATOR}"

# 打印带颜色的消息
print_header() {
    echo -e "${BLUE}===============================================${NC}"
    echo -e "${BLUE}🍎 Mini React Native - iOS 测试脚本${NC}"
    echo -e "${BLUE}===============================================${NC}"
}

print_info() {
    echo -e "${CYAN}ℹ️  $1${NC}"
}

print_success() {
    echo -e "${GREEN}✅ $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}⚠️  $1${NC}"
}

print_error() {
    echo -e "${RED}❌ $1${NC}"
}

# 检查 iOS 构建是否存在
check_ios_build() {
    if [ ! -d "build_ios_sim" ]; then
        print_error "iOS 模拟器构建不存在！"
        print_info "请先运行: make ios-sim-build"
        exit 1
    fi
}

# 检查模拟器是否可用
check_simulator() {
    print_info "检查模拟器: $SIMULATOR"

    if ! xcrun simctl list devices available | grep -q "$SIMULATOR"; then
        print_error "模拟器 '$SIMULATOR' 不可用！"
        print_info "可用的模拟器："
        xcrun simctl list devices available | grep iPhone | head -5
        exit 1
    fi

    print_success "模拟器 '$SIMULATOR' 可用"
}

# 启动模拟器
boot_simulator() {
    print_info "启动模拟器: $SIMULATOR"

    # 检查模拟器是否已经启动
    if xcrun simctl list devices | grep "$SIMULATOR" | grep -q "Booted"; then
        print_success "模拟器已经启动"
    else
        print_info "正在启动模拟器..."
        xcrun simctl boot "$SIMULATOR"
        print_success "模拟器启动成功"

        # 等待模拟器完全启动
        print_info "等待模拟器完全启动..."
        sleep 3
    fi
}

# 运行测试
run_test() {
    local test_name=$1
    local test_path=$2
    local description=$3

    echo
    print_info "🧪 运行测试: $test_name"
    print_info "📝 描述: $description"
    echo -e "${PURPLE}----------------------------------------${NC}"

    if [ ! -f "$test_path" ]; then
        print_error "测试文件不存在: $test_path"
        return 1
    fi

    # 运行测试
    if xcrun simctl spawn "$SIMULATOR" "$test_path"; then
        echo -e "${PURPLE}----------------------------------------${NC}"
        print_success "$test_name 测试完成"
    else
        echo -e "${PURPLE}----------------------------------------${NC}"
        print_error "$test_name 测试失败"
        return 1
    fi
}

# 显示帮助信息
show_help() {
    print_header
    echo
    echo -e "${YELLOW}用法:${NC}"
    echo "  $0 <test_type> [simulator_name]"
    echo
    echo -e "${YELLOW}测试类型:${NC}"
    echo "  basic        - 基础功能测试 (JSCExecutor)"
    echo "  deviceinfo   - DeviceInfo 模块测试"
    echo "  module       - 模块框架测试"
    echo "  integration  - 完整集成测试"
    echo "  all          - 运行所有测试"
    echo "  list         - 列出可用的模拟器"
    echo
    echo -e "${YELLOW}示例:${NC}"
    echo "  $0 deviceinfo                    # 在默认模拟器上运行 DeviceInfo 测试"
    echo "  $0 all \"iPhone 15 Pro\"          # 在指定模拟器上运行所有测试"
    echo "  $0 list                          # 列出可用模拟器"
    echo
    echo -e "${YELLOW}默认模拟器:${NC} $DEFAULT_SIMULATOR"
    echo
}

# 列出可用模拟器
list_simulators() {
    print_header
    print_info "可用的 iOS 模拟器："
    echo
    xcrun simctl list devices available | grep -E "(iPhone|iPad)" | head -10
    echo
}

# 运行所有测试
run_all_tests() {
    print_header
    print_info "在模拟器 '$SIMULATOR' 上运行所有测试"
    echo

    check_ios_build
    check_simulator
    boot_simulator

    local failed_tests=0

    # 基础测试
    if ! run_test "基础功能" "./build_ios_sim/mini_rn_test.app/mini_rn_test" "验证 JSCExecutor 基础功能"; then
        ((failed_tests++))
    fi

    # DeviceInfo 测试
    if ! run_test "DeviceInfo" "./build_ios_sim/test_ios_deviceinfo.app/test_ios_deviceinfo" "测试 iOS 设备信息获取和性能"; then
        ((failed_tests++))
    fi

    # 模块框架测试
    if ! run_test "模块框架" "./build_ios_sim/test_module_framework.app/test_module_framework" "验证模块注册和调用机制"; then
        ((failed_tests++))
    fi

    # 集成测试
    if ! run_test "集成测试" "./build_ios_sim/test_integration.app/test_integration" "完整 JavaScript ↔ Native 通信测试"; then
        ((failed_tests++))
    fi

    # 测试总结
    echo
    echo -e "${BLUE}===============================================${NC}"
    if [ $failed_tests -eq 0 ]; then
        print_success "🎉 所有测试通过！iOS 平台功能正常"
    else
        print_warning "⚠️  $failed_tests 个测试失败"
    fi
    echo -e "${BLUE}===============================================${NC}"

    return $failed_tests
}

# 主函数
main() {
    case "${1:-help}" in
        "basic")
            print_header
            check_ios_build
            check_simulator
            boot_simulator
            run_test "基础功能" "./build_ios_sim/mini_rn_test.app/mini_rn_test" "验证 JSCExecutor 基础功能"
            ;;
        "deviceinfo")
            print_header
            check_ios_build
            check_simulator
            boot_simulator
            run_test "DeviceInfo" "./build_ios_sim/test_ios_deviceinfo.app/test_ios_deviceinfo" "测试 iOS 设备信息获取和性能"
            ;;
        "module")
            print_header
            check_ios_build
            check_simulator
            boot_simulator
            run_test "模块框架" "./build_ios_sim/test_module_framework.app/test_module_framework" "验证模块注册和调用机制"
            ;;
        "integration")
            print_header
            check_ios_build
            check_simulator
            boot_simulator
            run_test "集成测试" "./build_ios_sim/test_integration.app/test_integration" "完整 JavaScript ↔ Native 通信测试"
            ;;
        "all")
            run_all_tests
            ;;
        "list")
            list_simulators
            ;;
        "help"|"--help"|"-h"|"")
            show_help
            ;;
        *)
            print_error "未知的测试类型: $1"
            echo
            show_help
            exit 1
            ;;
    esac
}

# 运行主函数
main "$@"
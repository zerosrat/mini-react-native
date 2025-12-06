#!/bin/bash

# Mini React Native - Android 运行脚本
#
# 使用方法:
#   ./run_android.sh            - 构建并安装到模拟器
#   ./run_android.sh build      - 仅构建
#   ./run_android.sh install    - 构建并安装
#   ./run_android.sh emulator   - 启动模拟器
#   ./run_android.sh clean      - 清理构建

set -e

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 项目配置
ANDROID_DIR="$(pwd)/android"
APP_ID="com.minirn.demo"
APP_NAME="MiniRN Demo"

# 检查并设置 JAVA_HOME
setup_java() {
    echo -e "${BLUE}🔧 Setting up Java environment...${NC}"

    if [ -z "$JAVA_HOME" ]; then
        # 尝试 Android Studio 自带的 JRE
        if [ -d "/Applications/Android Studio.app/Contents/jbr/Contents/Home" ]; then
            export JAVA_HOME="/Applications/Android Studio.app/Contents/jbr/Contents/Home"
            echo -e "${GREEN}✅ Using Android Studio JRE${NC}"
        # 尝试系统 Java
        elif command -v java >/dev/null 2>&1; then
            export JAVA_HOME="$(dirname $(dirname $(readlink $(which java))))"
            echo -e "${GREEN}✅ Using system Java${NC}"
        else
            echo -e "${RED}❌ Java not found. Please install Java or Android Studio.${NC}"
            exit 1
        fi
    fi

    echo "JAVA_HOME: $JAVA_HOME"
    java -version
}

# 检查 Android SDK 和 NDK
check_android_sdk() {
    echo -e "${BLUE}🔧 Checking Android SDK...${NC}"

    if [ -z "$ANDROID_HOME" ]; then
        # 尝试常见路径
        if [ -d "$HOME/Library/Android/sdk" ]; then
            export ANDROID_HOME="$HOME/Library/Android/sdk"
        elif [ -d "$HOME/Android/Sdk" ]; then
            export ANDROID_HOME="$HOME/Android/Sdk"
        else
            echo -e "${RED}❌ Android SDK not found. Please install Android Studio.${NC}"
            exit 1
        fi
    fi

    export PATH="$PATH:$ANDROID_HOME/tools:$ANDROID_HOME/platform-tools"
    echo "ANDROID_HOME: $ANDROID_HOME"
}

# 启动模拟器
start_emulator() {
    echo -e "${BLUE}📱 Starting Android emulator...${NC}"

    # 检查是否有运行中的模拟器
    if adb devices | grep -q "emulator-" && adb devices | grep -q "device$"; then
        echo -e "${GREEN}✅ Emulator already running${NC}"
        return
    fi

    # 获取可用模拟器列表
    EMULATORS=$("$ANDROID_HOME/tools/emulator" -list-avds 2>/dev/null || true)

    if [ -z "$EMULATORS" ]; then
        echo -e "${YELLOW}⚠️  No emulators found. Please create an emulator in Android Studio.${NC}"
        echo "Open Android Studio -> Tools -> AVD Manager -> Create Virtual Device"
        return 1
    fi

    # 启动第一个模拟器
    EMULATOR_NAME=$(echo "$EMULATORS" | head -n1)
    echo -e "${BLUE}🚀 Starting emulator: $EMULATOR_NAME${NC}"

    "$ANDROID_HOME/tools/emulator" -avd "$EMULATOR_NAME" -no-snapshot-load >/dev/null 2>&1 &

    # 等待模拟器启动
    echo -e "${YELLOW}⏳ Waiting for emulator to boot...${NC}"
    for i in {1..60}; do
        if adb devices | grep -q "emulator-" && adb devices | grep -q "device$"; then
            echo -e "${GREEN}✅ Emulator ready${NC}"
            return
        fi
        sleep 2
        echo -n "."
    done

    echo -e "\n${RED}❌ Emulator failed to start${NC}"
    return 1
}

# 构建 Android 项目
build_android() {
    echo -e "${BLUE}🔨 Building Android project...${NC}"

    cd "$ANDROID_DIR"

    # 构建 JavaScript bundle
    echo -e "${BLUE}📦 Building JavaScript bundle...${NC}"
    cd ..
    npm run build
    cd android

    # 使用 Gradle 构建
    if [ -f "./gradlew" ]; then
        chmod +x ./gradlew
        ./gradlew assembleDebug
    else
        gradle assembleDebug
    fi

    echo -e "${GREEN}✅ Build complete${NC}"
}

# 安装到设备/模拟器
install_android() {
    echo -e "${BLUE}📲 Installing to device...${NC}"

    cd "$ANDROID_DIR"

    # 检查设备
    if ! adb devices | grep -q "device$"; then
        echo -e "${YELLOW}⚠️  No devices found. Starting emulator...${NC}"
        start_emulator
    fi

    # 安装 APK
    APK_PATH="app/build/outputs/apk/debug/app-debug.apk"
    if [ -f "$APK_PATH" ]; then
        adb install -r "$APK_PATH"
        echo -e "${GREEN}✅ Installed successfully${NC}"

        # 启动应用
        echo -e "${BLUE}🚀 Starting app...${NC}"
        adb shell am start -n "$APP_ID/.MainActivity"
    else
        echo -e "${RED}❌ APK not found at $APK_PATH${NC}"
        exit 1
    fi
}

# 清理构建文件
clean_android() {
    echo -e "${BLUE}🧹 Cleaning Android build...${NC}"

    cd "$ANDROID_DIR"

    if [ -f "./gradlew" ]; then
        ./gradlew clean
    else
        gradle clean
    fi

    # 清理 JavaScript 构建文件
    cd ..
    npm run clean

    echo -e "${GREEN}✅ Clean complete${NC}"
}

# 显示帮助
show_help() {
    echo "Mini React Native - Android 运行脚本"
    echo ""
    echo "使用方法:"
    echo "  $0              - 构建并安装到模拟器"
    echo "  $0 build        - 仅构建项目"
    echo "  $0 install      - 构建并安装"
    echo "  $0 emulator     - 启动模拟器"
    echo "  $0 clean        - 清理构建文件"
    echo "  $0 help         - 显示此帮助"
    echo ""
    echo "环境要求:"
    echo "  - Android Studio"
    echo "  - Java 8+"
    echo "  - Android SDK"
    echo "  - Android NDK"
}

# 主函数
main() {
    case "${1:-run}" in
        "build")
            setup_java
            check_android_sdk
            build_android
            ;;
        "install")
            setup_java
            check_android_sdk
            build_android
            install_android
            ;;
        "emulator")
            check_android_sdk
            start_emulator
            ;;
        "clean")
            clean_android
            ;;
        "help"|"-h"|"--help")
            show_help
            ;;
        "run"|"")
            setup_java
            check_android_sdk
            start_emulator
            build_android
            install_android
            ;;
        *)
            echo -e "${RED}❌ Unknown command: $1${NC}"
            show_help
            exit 1
            ;;
    esac
}

# 运行主函数
main "$@"
# Mini React Native - Makefile
#
# 简化的构建脚本，提供常用的开发任务：
# - build: 编译项目
# - test: 运行测试
# - clean: 清理构建文件
# - install: 安装依赖
# - format: 代码格式化

# 变量定义
BUILD_DIR = build
CMAKE_BUILD_TYPE ?= Debug
CORES = $(shell sysctl -n hw.ncpu)

# 默认目标
# make 等价于 make all 等价于 make build
.PHONY: all
all: build

# 创建构建目录并配置 CMake，生成 build/Makefile
.PHONY: configure
configure:
	@echo "🔧 Configuring build system..."
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake -DCMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE) -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
	@echo "✅ Configuration complete"

# 构建 JavaScript bundle
.PHONY: js-build
js-build:
	@echo "📦 Building JavaScript bundle..."
	@npm run build
	@echo "✅ JavaScript bundle built"

# 监视 JavaScript 文件变化（开发模式）
.PHONY: js-watch
js-watch:
	@echo "👀 Watching JavaScript files for changes..."
	@npm run build:watch

# 清理 JavaScript 构建文件
.PHONY: js-clean
js-clean:
	@echo "🧹 Cleaning JavaScript build files..."
	@npm run clean
	@echo "✅ JavaScript files cleaned"

# 编译项目
# 执行顺序：js-build → configure → build
.PHONY: build
build: js-build configure
	@echo "🔨 Building Mini React Native..."
	@cd $(BUILD_DIR) && make -j$(CORES)
	@echo "✅ Build complete"

# iOS 构建配置（模拟器）
.PHONY: ios-configure
ios-configure:
	@echo "🔧 Configuring iOS build system..."
	@mkdir -p $(BUILD_DIR)_ios
	@cd $(BUILD_DIR)_ios && DEVELOPER_DIR=/Applications/Xcode.app/Contents/Developer cmake \
		-DCMAKE_SYSTEM_NAME=iOS \
		-DCMAKE_OSX_ARCHITECTURES=$$(uname -m) \
		-DCMAKE_OSX_SYSROOT=$$(DEVELOPER_DIR=/Applications/Xcode.app/Contents/Developer xcrun --sdk iphonesimulator --show-sdk-path) \
		-DCMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE) \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
		..
	@echo "✅ iOS configuration complete"

# 构建 iOS 版本（模拟器）
.PHONY: ios-build
ios-build: js-build ios-configure
	@echo "🔨 Building Mini React Native for iOS..."
	@cd $(BUILD_DIR)_ios && make -j$(CORES)
	@echo "✅ iOS build complete"

# iOS 测试目标
.PHONY: ios-test
ios-test: ios-build
	@echo "🍎 Running iOS tests..."
	@./test_ios.sh all

# iOS DeviceInfo 测试
.PHONY: ios-test-deviceinfo
ios-test-deviceinfo: ios-build
	@echo "🍎 Running iOS DeviceInfo test..."
	@./test_ios.sh deviceinfo

# 运行测试
# 执行顺序：configure → build → test
.PHONY: test
test: build
	@echo "🧪 Running all tests..."
	@echo "\n📝 Test 1: Basic functionality test"
	@./$(BUILD_DIR)/mini_rn_test
	@echo "\n📝 Test 2: Module framework test"
	@./$(BUILD_DIR)/test_module_framework
	@echo "\n📝 Test 3: Integration test"
	@./$(BUILD_DIR)/test_integration
	@echo "\n✅ All tests complete"

# 运行基础测试
.PHONY: test-basic
test-basic: build
	@echo "🧪 Running basic functionality test..."
	@./$(BUILD_DIR)/mini_rn_test
	@echo "✅ Basic test complete"

# 运行模块框架测试
.PHONY: test-module
test-module: build
	@echo "🧪 Running module framework test..."
	@./$(BUILD_DIR)/test_module_framework
	@echo "✅ Module framework test complete"

# 运行集成测试
.PHONY: test-integration
test-integration: build
	@echo "🧪 Running integration test..."
	@./$(BUILD_DIR)/test_integration
	@echo "✅ Integration test complete"

# 清理构建文件
.PHONY: clean
clean: js-clean
	@echo "🧹 Cleaning build files..."
	@rm -rf $(BUILD_DIR) $(BUILD_DIR)_ios
	@echo "✅ Clean complete"

# 完全重建
.PHONY: rebuild
rebuild: clean build

# 安装开发依赖（macOS）
.PHONY: install-deps
install-deps:
	@echo "📦 Installing development dependencies..."
	@if ! command -v cmake &> /dev/null; then \
		echo "Installing CMake via Homebrew..."; \
		brew install cmake; \
	fi
	@if ! command -v clang++ &> /dev/null; then \
		echo "Please install Xcode Command Line Tools: xcode-select --install"; \
		exit 1; \
	fi
	@echo "✅ Dependencies installed"

# 代码格式化（如果有 clang-format）
.PHONY: format
format:
	@if command -v clang-format &> /dev/null; then \
		echo "🎨 Formatting code..."; \
		find src examples -name "*.cpp" -o -name "*.h" | xargs clang-format -i; \
		echo "✅ Code formatted"; \
	else \
		echo "clang-format not found, skipping..."; \
	fi

# 显示构建信息
.PHONY: info
info:
	@echo "📋 Build Information:"
	@echo "  Project: Mini React Native"
	@echo "  Build Type: $(CMAKE_BUILD_TYPE)"
	@echo "  Build Directory: $(BUILD_DIR)"
	@echo "  CPU Cores: $(CORES)"
	@echo "  Platform: $(shell uname -s)"

# 开发模式 - 监视文件变化并自动重新构建（需要 fswatch）
.PHONY: dev
dev:
	@if command -v fswatch &> /dev/null; then \
		echo "👀 Watching for file changes... (Ctrl+C to stop)"; \
		fswatch -o src examples CMakeLists.txt | while read; do \
			echo "🔄 Files changed, rebuilding..."; \
			make build; \
		done; \
	else \
		echo "fswatch not found. Install with: brew install fswatch"; \
	fi

# 帮助信息
.PHONY: help
help:
	@echo "Mini React Native - Available Commands:"
	@echo ""
	@echo "构建命令:"
	@echo "  make build            - 编译项目 (默认目标，包含 JS 构建)"
	@echo "  make js-build         - 仅构建 JavaScript bundle"
	@echo "  make js-watch         - 监视 JS 文件变化并自动构建"
	@echo "  make clean            - 清理所有构建文件"
	@echo "  make js-clean         - 仅清理 JavaScript 构建文件"
	@echo "  make rebuild          - 完全重新构建"
	@echo "  make configure        - 仅配置 CMake"
	@echo ""
	@echo "iOS 构建命令:"
	@echo "  make ios-build        - 构建 iOS 版本（模拟器）"
	@echo "  make ios-configure    - 仅配置 iOS 构建"
	@echo ""
	@echo "iOS 测试命令:"
	@echo "  make ios-test         - 运行所有 iOS 测试"
	@echo "  make ios-test-deviceinfo - 运行 iOS DeviceInfo 测试"
	@echo ""
	@echo "测试命令:"
	@echo "  make test             - 运行所有测试"
	@echo "  make test-basic       - 仅运行基础功能测试"
	@echo "  make test-module      - 仅运行模块框架测试"
	@echo "  make test-integration - 仅运行集成测试"
	@echo ""
	@echo "开发工具:"
	@echo "  make install-deps     - 安装开发依赖"
	@echo "  make format           - 格式化代码"
	@echo "  make info             - 显示构建信息"
	@echo "  make dev              - 开发模式（自动重建）"
	@echo "  make help             - 显示此帮助信息"
	@echo ""
	@echo "环境变量:"
	@echo "  CMAKE_BUILD_TYPE      - 构建类型 (Debug/Release, 默认: Debug)"
	@echo ""
	@echo "示例:"
	@echo "  make CMAKE_BUILD_TYPE=Release build"
	@echo "  make test"
	@echo "  make test-integration"
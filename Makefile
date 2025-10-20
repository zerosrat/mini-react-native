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
	@cd $(BUILD_DIR) && cmake -DCMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE) ..
	@echo "✅ Configuration complete"

# 编译项目
# 执行顺序：configure → build
.PHONY: build
build: configure
	@echo "🔨 Building Mini React Native..."
	@cd $(BUILD_DIR) && make -j$(CORES)
	@echo "✅ Build complete"

# 运行测试
# 执行顺序：configure → build → test
.PHONY: test
test: build
	@echo "🧪 Running all tests..."
	@echo "\n📝 Test 1: Basic functionality test"
	@./$(BUILD_DIR)/mini_rn_test
	@echo "\n📝 Test 2: MessageQueue validation test"
	@./$(BUILD_DIR)/test_messagequeue
		@echo "\n📝 Test 2: NativeModule validation test"
	@./$(BUILD_DIR)/test_module_framework
	@echo "\n✅ All tests complete"

# 运行基础测试
.PHONY: test-basic
test-basic: build
	@echo "🧪 Running basic functionality test..."
	@./$(BUILD_DIR)/mini_rn_test
	@echo "✅ Basic test complete"

# 运行 MessageQueue 测试
.PHONY: test-messagequeue
test-messagequeue: build
	@echo "🧪 Running MessageQueue validation test..."
	@./$(BUILD_DIR)/test_messagequeue
	@echo "✅ MessageQueue test complete"

# 运行 NativeModule 测试
.PHONY: test-module
test-messagequeue: build
	@echo "🧪 Running NativeModule validation test..."
	@./$(BUILD_DIR)/test_module_framework
	@echo "✅ NativeModule test complete"

# 清理构建文件
.PHONY: clean
clean:
	@echo "🧹 Cleaning build files..."
	@rm -rf $(BUILD_DIR)
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
	@echo "  make build            - 编译项目 (默认目标)"
	@echo "  make clean            - 清理构建文件"
	@echo "  make rebuild          - 完全重新构建"
	@echo "  make configure        - 仅配置 CMake"
	@echo ""
	@echo "测试命令:"
	@echo "  make test             - 运行所有测试"
	@echo "  make test-basic       - 仅运行基础功能测试"
	@echo "  make test-messagequeue- 仅运行 MessageQueue 测试"
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
	@echo "  make test-messagequeue"
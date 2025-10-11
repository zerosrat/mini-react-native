# React Native Bridge Demo Makefile

# 项目配置
PROJECT_NAME = mini-rn-bridge
BUILD_DIR = build
DIST_DIR = dist

# 编译器配置
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2
INCLUDES = -I./cpp -I/usr/local/include
LIBS = -ljsoncpp

# 源文件
CPP_SOURCES = cpp/JSCExecutor.cpp cpp/NativeModule.cpp
HEADERS = cpp/JSCExecutor.h cpp/NativeModule.h

# 目标文件
OBJECTS = $(CPP_SOURCES:cpp/%.cpp=$(BUILD_DIR)/%.o)

# 默认目标
.PHONY: all clean test demo help install

all: clean build test

# 创建构建目录
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(DIST_DIR):
	mkdir -p $(DIST_DIR)

# 编译 C++ 源文件
$(BUILD_DIR)/%.o: cpp/%.cpp $(HEADERS) | $(BUILD_DIR)
	@echo "编译 $<..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# 构建静态库
$(BUILD_DIR)/lib$(PROJECT_NAME).a: $(OBJECTS)
	@echo "创建静态库..."
	ar rcs $@ $(OBJECTS)

# 构建目标
build: $(BUILD_DIR)/lib$(PROJECT_NAME).a
	@echo "构建完成: $(BUILD_DIR)/lib$(PROJECT_NAME).a"

# 安装到系统
install: build | $(DIST_DIR)
	@echo "安装头文件和库文件..."
	cp $(HEADERS) $(DIST_DIR)/
	cp $(BUILD_DIR)/lib$(PROJECT_NAME).a $(DIST_DIR)/
	@echo "安装完成: $(DIST_DIR)/"

# 运行 JavaScript 测试
test-js:
	@echo "运行 JavaScript 测试..."
	node examples/basic-usage.js

# 运行 C++ 测试
test-cpp: build
	@echo "运行 C++ 测试..."
	@echo "注意: C++ 测试需要更完整的环境配置"

# 运行所有测试
test: test-js
	@echo "所有测试完成"

# 启动演示服务器
demo:
	@echo "启动演示服务器..."
	@echo "在浏览器中打开: http://localhost:8080"
	python3 -m http.server 8080 --directory examples 2>/dev/null || \
	python -m SimpleHTTPServer 8080 2>/dev/null || \
	@echo "请手动打开 examples/bridge-demo.html"

# 代码格式化
format:
	@echo "格式化代码..."
	@command -v clang-format >/dev/null && find cpp -name "*.cpp" -o -name "*.h" | xargs clang-format -i || echo "需要安装 clang-format"
	@command -v prettier >/dev/null && prettier --write "js/*.js" "examples/*.js" || echo "需要安装 prettier"

# 代码检查
lint:
	@echo "运行代码检查..."
	@command -v eslint >/dev/null && eslint js/ examples/ || echo "需要安装 eslint"
	@command -v cppcheck >/dev/null && cppcheck cpp/ || echo "需要安装 cppcheck"

# 生成文档
docs:
	@echo "生成文档..."
	@command -v jsdoc >/dev/null && jsdoc js/ -d docs/js-api/ || echo "需要安装 jsdoc"
	@command -v doxygen >/dev/null && doxygen Doxyfile || echo "需要安装 doxygen"

# 打包发布
package: clean build test | $(DIST_DIR)
	@echo "打包项目..."
	cp -r js $(DIST_DIR)/
	cp -r examples $(DIST_DIR)/
	cp -r docs $(DIST_DIR)/
	cp package.json README.md $(DIST_DIR)/
	tar -czf $(PROJECT_NAME)-release.tar.gz -C $(DIST_DIR) .
	@echo "打包完成: $(PROJECT_NAME)-release.tar.gz"

# iOS 构建 (需要 Xcode)
ios:
	@echo "构建 iOS 版本..."
	@if [ -d ios/ ]; then \
		xcodebuild -project ios/BridgeDemo.xcodeproj -scheme BridgeDemo -configuration Release; \
	else \
		echo "iOS 项目目录不存在"; \
	fi

# Android 构建 (需要 Android NDK)
android:
	@echo "构建 Android 版本..."
	@if [ -d android/ ] && [ -f android/CMakeLists.txt ]; then \
		mkdir -p android/build && cd android/build && \
		cmake .. -DANDROID_ABI=arm64-v8a -DCMAKE_TOOLCHAIN_FILE=$$ANDROID_NDK/build/cmake/android.toolchain.cmake && \
		make; \
	else \
		echo "Android 项目配置不完整"; \
	fi

# 清理构建文件
clean:
	@echo "清理构建文件..."
	rm -rf $(BUILD_DIR) $(DIST_DIR)
	rm -f $(PROJECT_NAME)-release.tar.gz
	@echo "清理完成"

# 开发环境设置
setup:
	@echo "设置开发环境..."
	@echo "检查依赖..."
	@command -v node >/dev/null || echo "❌ 需要安装 Node.js"
	@command -v npm >/dev/null || echo "❌ 需要安装 npm"
	@command -v g++ >/dev/null || echo "❌ 需要安装 C++ 编译器"
	@echo "安装 npm 依赖..."
	@if [ -f package.json ]; then npm install; fi
	@echo "开发环境设置完成"

# 帮助信息
help:
	@echo "React Native Bridge Demo - 构建系统"
	@echo ""
	@echo "可用的命令:"
	@echo "  all        - 完整构建和测试"
	@echo "  build      - 编译 C++ 代码"
	@echo "  test       - 运行所有测试"
	@echo "  test-js    - 运行 JavaScript 测试"
	@echo "  test-cpp   - 运行 C++ 测试"
	@echo "  demo       - 启动演示服务器"
	@echo "  format     - 格式化代码"
	@echo "  lint       - 代码检查"
	@echo "  docs       - 生成文档"
	@echo "  package    - 打包发布版本"
	@echo "  ios        - 构建 iOS 版本"
	@echo "  android    - 构建 Android 版本"
	@echo "  clean      - 清理构建文件"
	@echo "  setup      - 设置开发环境"
	@echo "  install    - 安装到系统"
	@echo "  help       - 显示此帮助信息"
	@echo ""
	@echo "示例:"
	@echo "  make all       # 完整构建"
	@echo "  make demo      # 启动演示"
	@echo "  make test      # 运行测试"

# 监听文件变化并自动重新构建
watch:
	@echo "监听文件变化..."
	@command -v fswatch >/dev/null && \
	fswatch -o cpp/ js/ | xargs -n1 -I{} make build || \
	echo "需要安装 fswatch 来使用监听功能"

# 性能测试
benchmark: build
	@echo "运行性能测试..."
	node -e "require('./examples/basic-usage').benchmarkTest()" 2>/dev/null || \
	echo "性能测试需要完整的运行环境"

# 内存检查 (需要 valgrind)
memcheck: build
	@echo "运行内存检查..."
	@command -v valgrind >/dev/null && \
	valgrind --tool=memcheck --leak-check=full ./test-binary 2>/dev/null || \
	echo "需要安装 valgrind 并配置测试二进制文件"

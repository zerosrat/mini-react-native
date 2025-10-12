# 开发环境配置文档

## 🛠️ 开发环境要求

### 基础环境

- **操作系统**: macOS 10.15+ (当前主要支持平台)
- **编译器**: Clang++ (Apple LLVM)
- **构建系统**: CMake 3.15+
- **C++ 标准**: C++17 (GNU++17)

### 必需工具

- **CMake**: 用于项目构建配置
- **Clang**: C++ 编译器 (系统自带)
- **JavaScriptCore**: Apple 系统框架 (系统自带)

## 📝 C++ 标准配置

### 项目标准

```cmake
# CMakeLists.txt
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
```

### 实际编译标准

- **编译标志**: `-std=gnu++17`
- **说明**: 使用 GNU 扩展版本的 C++17，包含 GCC/Clang 特有扩展
- **兼容性**: 与标准 C++17 向后兼容

### 编译器配置

```bash
# 实际编译命令示例
/usr/bin/clang++ -std=gnu++17 -Wall -Wextra -g -O0 \
    -arch arm64 -mmacosx-version-min=10.15 \
    -I/path/to/src \
    -F/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/System/Library/Frameworks
```

## 🔧 IDE 配置

### Cursor/VS Code

推荐使用 **clangd** 扩展而非 Microsoft C++ 扩展：

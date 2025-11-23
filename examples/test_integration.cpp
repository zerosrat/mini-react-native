#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <vector>
#include <chrono>

#include "common/bridge/JSCExecutor.h"
#include "common/modules/DeviceInfoModule.h"
#include "common/modules/ModuleRegistry.h"

using namespace mini_rn::bridge;
using namespace mini_rn::modules;

/**
 * Mini React Native - 端到端集成测试
 *
 * 这是标准的集成测试模板，验证完整的 JavaScript ↔ Native 通信流程。
 * 使用 Rollup 打包后的 JavaScript bundle，测试真实的模块交互场景。
 *
 * 测试内容：
 * - JavaScript bundle 加载和执行
 * - Native 模块注册和配置注入
 * - Bridge 双向通信
 * - 具体模块功能验证
 *
 * 使用方式：
 * - make test-integration
 * - 或直接运行 ./build/test_integration
 */

/**
 * 读取文件内容到字符串
 * @param filePath 文件路径
 * @return 文件内容字符串，如果失败返回空字符串
 */
std::string readFile(const std::string& filePath) {
  try {
    std::ifstream file(filePath);
    if (!file.is_open()) {
      std::cout << "[File Reader] Error: Cannot open file: " << filePath
                << std::endl;
      return "";
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    file.close();

    std::string content = buffer.str();
    std::cout << "[File Reader] Successfully read file: " << filePath
              << " (size: " << content.length() << " bytes)" << std::endl;

    return content;
  } catch (const std::exception& e) {
    std::cout << "[File Reader] Exception reading file " << filePath << ": "
              << e.what() << std::endl;
    return "";
  }
}

/**
 * 性能测试函数 - 测试 Bridge 通信性能
 * @param executor JSCExecutor 实例
 */
void performanceTest(JSCExecutor& executor) {
  std::cout << "\n3.5. Testing Bridge communication performance..." << std::endl;

  auto start = std::chrono::high_resolution_clock::now();

  // 执行性能测试脚本
  std::string performanceScript = R"(
    // 性能测试：调用 DeviceInfo 方法
    try {
      // getSystemVersion (methodId = 1)
      var systemVersion = global.nativeCallSyncHook(0, 1, []);

      // getDeviceId (methodId = 2)
      var deviceId = global.nativeCallSyncHook(0, 2, []);

      console.log('✅ Performance test completed - SystemVersion:', systemVersion, 'DeviceId:', deviceId);
    } catch (e) {
      console.log('❌ Performance test failed:', e.toString());
    }
  )";

  executor.loadApplicationScript(performanceScript, "performance_test.js");

  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

  // 性能报告
  double ms = duration.count() / 1000.0;
  std::cout << "   Bridge call duration: " << ms << " ms" << std::endl;

  if (ms < 10.0) {
    std::cout << "   ✅ Performance requirement met (< 10ms)" << std::endl;
  } else {
    std::cout << "   ⚠️ Performance slower than expected (>= 10ms)" << std::endl;
  }
}

void testIntegration() {
  std::cout << "\n=== Mini React Native Integration Test ===" << std::endl;

  try {
    // 创建 JSCExecutor
    JSCExecutor executor;

    // 设置异常处理器
    executor.setJSExceptionHandler([](const std::string& error) {
      std::cout << "[JS Exception] " << error << std::endl;
    });

    // 直接测试 DeviceInfo 原生方法
    std::cout << "\n1. Testing DeviceInfo methods directly..." << std::endl;
    auto deviceInfoForTesting = std::make_unique<DeviceInfoModule>();
    std::cout << "   UniqueId: " << deviceInfoForTesting->getUniqueIdImpl() << std::endl;
    std::cout << "   SystemVersion: " << deviceInfoForTesting->getSystemVersionImpl() << std::endl;
    std::cout << "   DeviceId: " << deviceInfoForTesting->getDeviceIdImpl() << std::endl;

    // 注册 DeviceInfo 模块（自动注入配置）
    std::cout << "\n2. Registering DeviceInfo module and injecting configuration..."
              << std::endl;
    std::vector<std::unique_ptr<mini_rn::modules::NativeModule>> modules;
    modules.push_back(std::make_unique<DeviceInfoModule>());
    executor.registerModules(std::move(modules));

    // 加载打包后的 JavaScript bundle
    std::cout << "\n3. Loading JavaScript bundle..." << std::endl;

    // 尝试多个可能的路径（支持不同的工作目录）
    std::vector<std::string> possiblePaths = {
        "dist/bundle.js",                                    // 项目根目录
        "../dist/bundle.js",                                 // 从 build 目录
        "../../dist/bundle.js",                              // 从深层目录
        "/Users/yujiayu02/Dev/Repo/github/mini-react-native/dist/bundle.js"  // 绝对路径
    };

    std::string bundlePath;
    std::string bundleScript;

    for (const auto& path : possiblePaths) {
        bundleScript = readFile(path);
        if (!bundleScript.empty()) {
            bundlePath = path;
            break;
        }
    }

    if (bundleScript.empty()) {
      std::cout << "[Error] Failed to load JavaScript bundle: " << bundlePath
                << std::endl;
      std::cout << "        Make sure you have run 'make js-build' first."
                << std::endl;
      return;
    }

    std::cout << "   ✓ Bundle loaded successfully (" << bundleScript.length()
              << " bytes)" << std::endl;

    // 执行打包后的 JavaScript bundle
    executor.loadApplicationScript(bundleScript, bundlePath);
    std::cout << "   ✓ Bundle executed successfully" << std::endl;

    // 性能测试
    performanceTest(executor);

    // 加载测试文件
    std::cout << "\n4. Loading DeviceInfo integration test..." << std::endl;

    // 尝试多个可能的测试脚本路径
    std::vector<std::string> possibleTestPaths = {
        "examples/scripts/test_deviceinfo.js",                                    // 项目根目录
        "../examples/scripts/test_deviceinfo.js",                                 // 从 build 目录
        "../../examples/scripts/test_deviceinfo.js",                              // 从深层目录
        "/Users/yujiayu02/Dev/Repo/github/mini-react-native/examples/scripts/test_deviceinfo.js"  // 绝对路径
    };

    std::string testPath;
    std::string testScript;

    for (const auto& path : possibleTestPaths) {
        testScript = readFile(path);
        if (!testScript.empty()) {
            testPath = path;
            break;
        }
    }

    if (testScript.empty()) {
      std::cout << "[Error] Failed to load test file: " << testPath
                << std::endl;
      std::cout << "        Make sure the file exists and is readable."
                << std::endl;
      return;
    }

    std::cout << "   ✓ Test file loaded successfully" << std::endl;
    std::cout << "   ✓ Executing DeviceInfo integration test..." << std::endl;

    executor.loadApplicationScript(testScript, testPath);

    std::cout << "\n5. Integration Test Completed!" << std::endl;
    std::cout
        << "   Check the JavaScript output above for detailed test results."
        << std::endl;

  } catch (const std::exception& e) {
    std::cout << "\nBundle-based test failed with exception: " << e.what()
              << std::endl;
  }
}

int main() {
  std::cout << "Mini React Native - Integration Test" << std::endl;
  std::cout << "This test verifies the complete JavaScript ↔ Native communication using bundled JavaScript" << std::endl;

  // 运行集成测试
  testIntegration();

  return 0;
}
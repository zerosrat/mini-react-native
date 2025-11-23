#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <vector>

#ifdef __APPLE__
#include <TargetConditionals.h>
#if TARGET_OS_IPHONE
// iOS特定声明，实现在单独的文件中
extern "C" {
    const char* getBundlePath();
    const char* getResourcePath(const char* resourceName);
}
#endif
#endif

#include "common/bridge/JSCExecutor.h"
#include "common/modules/DeviceInfoModule.h"
#include "common/modules/ModuleRegistry.h"

using namespace mini_rn::bridge;
using namespace mini_rn::modules;

#ifdef __APPLE__
#if TARGET_OS_IPHONE
/**
 * 获取iOS应用包路径
 * @return iOS应用包的完整路径
 */
std::string getMainBundlePath() {
  const char* path = getBundlePath();
  return path ? std::string(path) : "";
}

/**
 * 获取iOS应用包内资源路径
 * @param resourceName 资源文件名
 * @return 资源文件的完整路径
 */
std::string getBundleResourcePath(const std::string& resourceName) {
  const char* path = getResourcePath(resourceName.c_str());
  return path ? std::string(path) : "";
}
#endif
#endif

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
    std::cout << "   UniqueId: " << deviceInfoForTesting->getUniqueIdImpl()
              << std::endl;
    std::cout << "   SystemVersion: "
              << deviceInfoForTesting->getSystemVersionImpl() << std::endl;
    std::cout << "   DeviceId: " << deviceInfoForTesting->getDeviceIdImpl()
              << std::endl;

    // 注册 DeviceInfo 模块（自动注入配置）
    std::cout
        << "\n2. Registering DeviceInfo module and injecting configuration..."
        << std::endl;
    std::vector<std::unique_ptr<mini_rn::modules::NativeModule>> modules;
    modules.push_back(std::make_unique<DeviceInfoModule>());
    executor.registerModules(std::move(modules));

    // 加载打包后的 JavaScript bundle
    std::cout << "\n3. Loading JavaScript bundle..." << std::endl;

    // 构建路径列表，iOS优先使用bundle路径
    std::vector<std::string> possiblePaths;

#ifdef __APPLE__
#if TARGET_OS_IPHONE
    // iOS: 优先尝试应用包内的bundle.js
    std::string bundleResourcePath = getBundleResourcePath("bundle.js");
    if (!bundleResourcePath.empty()) {
        possiblePaths.push_back(bundleResourcePath);
        std::cout << "   [iOS] Found bundle resource path: " << bundleResourcePath << std::endl;
    }

    // iOS: 也尝试应用包根目录
    std::string appBundlePath = getMainBundlePath();
    if (!appBundlePath.empty()) {
        possiblePaths.push_back(appBundlePath + "/bundle.js");
        std::cout << "   [iOS] Bundle path: " << appBundlePath << std::endl;
    }
#endif
#endif

    // 通用路径（适用于macOS和iOS fallback）
    possiblePaths.insert(possiblePaths.end(), {
        "bundle.js",             // 当前目录
        "dist/bundle.js",        // 项目根目录
        "./dist/bundle.js",      // 从 build 目录
        "../dist/bundle.js",     // 从 build 目录
        "../../dist/bundle.js",  // 从深层目录
    });

    std::string foundBundlePath;
    std::string bundleScript;

    for (const auto& path : possiblePaths) {
      bundleScript = readFile(path);
      if (!bundleScript.empty()) {
        foundBundlePath = path;
        break;
      }
    }

    if (bundleScript.empty()) {
      std::cout << "[Error] Failed to load JavaScript bundle: " << foundBundlePath
                << std::endl;
      std::cout << "        Make sure you have run 'make js-build' first."
                << std::endl;
      return;
    }

    std::cout << "   ✓ Bundle loaded successfully (" << bundleScript.length()
              << " bytes)" << std::endl;

    // 执行打包后的 JavaScript bundle
    executor.loadApplicationScript(bundleScript, foundBundlePath);
    std::cout << "   ✓ Bundle executed successfully" << std::endl;

    // 加载测试文件
    std::cout << "\n4. Loading DeviceInfo integration test..." << std::endl;

    // 构建测试脚本路径列表，iOS优先使用bundle路径
    std::vector<std::string> possibleTestPaths;

#ifdef __APPLE__
#if TARGET_OS_IPHONE
    // iOS: 优先尝试应用包内的测试脚本
    std::string testResourcePath = getBundleResourcePath("test_deviceinfo.js");
    if (!testResourcePath.empty()) {
        possibleTestPaths.push_back(testResourcePath);
        std::cout << "   [iOS] Found test resource path: " << testResourcePath << std::endl;
    }

    // iOS: 也尝试应用包根目录
    if (!appBundlePath.empty()) {
        possibleTestPaths.push_back(appBundlePath + "/test_deviceinfo.js");
    }
#endif
#endif

    // 通用路径（适用于macOS和iOS fallback）
    possibleTestPaths.insert(possibleTestPaths.end(), {
        "examples/scripts/test_deviceinfo.js",        // 项目根目录
        "./examples/scripts/test_deviceinfo.js",      // 从 build 目录
        "../examples/scripts/test_deviceinfo.js",     // 从 build 目录
        "../../examples/scripts/test_deviceinfo.js",  // 从深层目录
    });

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
  std::cout << "This test verifies the complete JavaScript ↔ Native "
               "communication using bundled JavaScript"
            << std::endl;

  // 运行集成测试
  testIntegration();

  return 0;
}
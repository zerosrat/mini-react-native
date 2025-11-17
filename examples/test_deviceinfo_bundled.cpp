#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <vector>

#include "common/bridge/JSCExecutor.h"
#include "common/modules/DeviceInfoModule.h"
#include "common/modules/ModuleRegistry.h"

using namespace mini_rn::bridge;
using namespace mini_rn::modules;

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

void testDeviceInfoWithBundle() {
  std::cout << "\n=== DeviceInfo Module Test with Bundled JavaScript ===" << std::endl;

  try {
    // 创建 JSCExecutor
    JSCExecutor executor;

    // 设置异常处理器
    executor.setJSExceptionHandler([](const std::string& error) {
      std::cout << "[JS Exception] " << error << std::endl;
    });

    // 注册 DeviceInfo 模块（自动注入配置）
    std::cout << "\n1. Registering DeviceInfo module and injecting configuration..."
              << std::endl;
    std::vector<std::unique_ptr<mini_rn::modules::NativeModule>> modules;
    modules.push_back(std::make_unique<DeviceInfoModule>());
    executor.registerModules(std::move(modules));

    // 加载打包后的 JavaScript bundle
    std::cout << "\n2. Loading JavaScript bundle..." << std::endl;

    std::string bundlePath = "dist/bundle.js";
    std::string bundleScript = readFile(bundlePath);

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

    // 加载测试文件
    std::cout << "\n3. Loading DeviceInfo integration test..." << std::endl;

    std::string testPath = "examples/scripts/test_deviceinfo.js";
    std::string testScript = readFile(testPath);

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

    std::cout << "\n4. Bundle-based JavaScript Test Completed!" << std::endl;
    std::cout
        << "   Check the JavaScript output above for detailed test results."
        << std::endl;

  } catch (const std::exception& e) {
    std::cout << "\nBundle-based test failed with exception: " << e.what()
              << std::endl;
  }
}

int main() {
  std::cout << "Mini React Native - DeviceInfo Module Test (Bundled JavaScript)" << std::endl;
  std::cout << "This test verifies the DeviceInfo module using the Rollup-bundled JavaScript" << std::endl;

  // 测试使用打包后的 bundle
  testDeviceInfoWithBundle();

  return 0;
}
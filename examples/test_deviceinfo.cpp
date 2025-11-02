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

/**
 * 测试 DeviceInfo 模块的集成和功能
 */

void testDeviceInfoIntegration() {
  std::cout << "\n=== DeviceInfo Module Integration Test ===" << std::endl;

  try {
    // 创建 JSCExecutor
    JSCExecutor executor;

    // 获取 ModuleRegistry
    auto* moduleRegistry = executor.getModuleRegistry();

    // 创建 DeviceInfo 模块
    auto deviceInfoModule = std::make_unique<DeviceInfoModule>(
        [](int callId, const std::string& result, bool isError) {
          // 回调处理器：将结果返回给 JavaScript
          std::cout << "[Callback] CallId: " << callId << ", Result: " << result
                    << ", IsError: " << (isError ? "true" : "false")
                    << std::endl;
        });

    // 注册 DeviceInfo 模块
    std::vector<std::unique_ptr<NativeModule>> modules;
    modules.push_back(std::move(deviceInfoModule));
    moduleRegistry->registerModules(std::move(modules));

    std::cout << "\n1. Module Registration Test:" << std::endl;
    std::cout << "   - Total modules: " << moduleRegistry->getModuleCount()
              << std::endl;

    auto moduleNames = moduleRegistry->moduleNames();
    std::cout << "   - Module names: ";
    for (const auto& name : moduleNames) {
      std::cout << name << " ";
    }
    std::cout << std::endl;

    // 测试模块方法调用
    std::cout << "\n2. Method Invocation Test:" << std::endl;

    // 假设 DeviceInfo 是第一个模块 (moduleId = 0)
    unsigned int deviceInfoModuleId = 0;

    // 测试 getUniqueId (假设是第一个方法，methodId = 0)
    std::cout << "   - Testing getUniqueId..." << std::endl;
    moduleRegistry->callNativeMethod(deviceInfoModuleId, 0, "[]", 1001);

    // 测试 getSystemVersion (假设是第二个方法，methodId = 1)
    std::cout << "   - Testing getSystemVersion..." << std::endl;
    moduleRegistry->callNativeMethod(deviceInfoModuleId, 1, "[]", 1002);

    // 测试 getModel (假设是第三个方法，methodId = 2)
    std::cout << "   - Testing getModel..." << std::endl;
    moduleRegistry->callNativeMethod(deviceInfoModuleId, 2, "[]", 1003);

    // 测试 getSystemName (假设是第四个方法，methodId = 3)
    std::cout << "   - Testing getSystemName..." << std::endl;
    moduleRegistry->callNativeMethod(deviceInfoModuleId, 3, "[]", 1004);

    std::cout << "\n3. DeviceInfo Integration Test Completed!" << std::endl;

  } catch (const std::exception& e) {
    std::cout << "\nTest failed with exception: " << e.what() << std::endl;
  }
}

void testDeviceInfoJavaScriptIntegration() {
  std::cout << "\n=== DeviceInfo JavaScript Integration Test ===" << std::endl;

  try {
    // 创建 JSCExecutor
    JSCExecutor executor;

    // 设置异常处理器
    executor.setJSExceptionHandler([](const std::string& error) {
      std::cout << "[JS Exception] " << error << std::endl;
    });

    // 获取 ModuleRegistry 并注册 DeviceInfo 模块
    auto* moduleRegistry = executor.getModuleRegistry();

    auto deviceInfoModule = std::make_unique<DeviceInfoModule>(
        [&executor](int callId, const std::string& result, bool isError) {
          std::cout << "[DeviceInfo Callback] CallId: " << callId
                    << ", Result: " << result
                    << ", IsError: " << (isError ? "true" : "false")
                    << std::endl;
          // 调用 JSCExecutor 的回调方法，将结果返回给 JavaScript
          executor.handleModuleCallback(callId, result, isError);
        });

    std::vector<std::unique_ptr<NativeModule>> modules;
    modules.push_back(std::move(deviceInfoModule));
    moduleRegistry->registerModules(std::move(modules));

    // 注入模块配置到 JavaScript 环境
    std::cout << "\n   ✓ Injecting module configuration into JavaScript "
                 "environment..."
              << std::endl;
    executor.injectModuleConfig();

    // 按顺序加载 JavaScript 模块文件
    std::cout << "\n4. Loading JavaScript modules sequentially..." << std::endl;

    // 定义模块加载顺序和路径
    std::vector<std::pair<std::string, std::string>> jsModules = {
        {"MessageQueue", "src/js/MessageQueue.js"},
        {"BatchedBridge", "src/js/BatchedBridge.js"},
        {"NativeModule", "src/js/NativeModule.js"},
        {"DeviceInfo", "src/js/DeviceInfo.js"}};

    // 逐个加载模块文件
    for (const auto& module : jsModules) {
      const std::string& moduleName = module.first;
      const std::string& modulePath = module.second;

      std::cout << "   Loading " << moduleName << " from " << modulePath
                << "..." << std::endl;

      std::string moduleScript = readFile(modulePath);
      if (moduleScript.empty()) {
        std::cout << "[Error] Failed to load " << moduleName
                  << " from: " << modulePath << std::endl;
        std::cout << "        Make sure the file exists and is readable."
                  << std::endl;
        return;
      }

      executor.loadApplicationScript(moduleScript, modulePath);
      std::cout << "   ✓ " << moduleName << " loaded successfully" << std::endl;
    }

    // 加载测试文件
    std::cout << "\n5. Loading DeviceInfo integration test..." << std::endl;

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

    std::cout << "\n5. JavaScript Integration Test Completed!" << std::endl;
    std::cout
        << "   Check the JavaScript output above for detailed test results."
        << std::endl;

  } catch (const std::exception& e) {
    std::cout << "\nJavaScript test failed with exception: " << e.what()
              << std::endl;
  }
}

int main() {
  std::cout << "Mini React Native - DeviceInfo Module Test" << std::endl;
  std::cout << "This test verifies the DeviceInfo module integration and "
               "functionality"
            << std::endl;

  // 测试模块集成
  testDeviceInfoIntegration();

  // 测试 JavaScript 集成
  testDeviceInfoJavaScriptIntegration();

  return 0;
}
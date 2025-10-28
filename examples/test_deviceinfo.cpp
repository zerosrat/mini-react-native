#include <iostream>
#include <memory>
#include <vector>

#include "common/bridge/JSCExecutor.h"
#include "common/modules/DeviceInfoModule.h"
#include "common/modules/ModuleRegistry.h"

using namespace mini_rn::bridge;
using namespace mini_rn::modules;

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
        [](int callId, const std::string& result, bool isError) {
          std::cout << "[DeviceInfo Callback] CallId: " << callId
                    << ", Result: " << result
                    << ", IsError: " << (isError ? "true" : "false")
                    << std::endl;
        });

    std::vector<std::unique_ptr<NativeModule>> modules;
    modules.push_back(std::move(deviceInfoModule));
    moduleRegistry->registerModules(std::move(modules));

    // 执行 JavaScript 代码来测试 DeviceInfo 模块
    std::string testScript = R"(
            console.log("Testing DeviceInfo module from JavaScript...");

            // 测试直接调用 Bridge 函数
            if (typeof nativeFlushQueueImmediate === 'function') {
                console.log("Calling DeviceInfo.getUniqueId...");
                // 调用 DeviceInfo 模块 (moduleId=0) 的 getUniqueId 方法 (methodId=0)
                nativeFlushQueueImmediate([[0], [0], [[]], [2001]]);

                console.log("Calling DeviceInfo.getSystemVersion...");
                // 调用 DeviceInfo 模块的 getSystemVersion 方法
                nativeFlushQueueImmediate([[0], [1], [[]], [2002]]);

                console.log("Calling DeviceInfo.getModel...");
                // 调用 DeviceInfo 模块的 getModel 方法
                nativeFlushQueueImmediate([[0], [2], [[]], [2003]]);

                console.log("Calling DeviceInfo.getSystemName...");
                // 调用 DeviceInfo 模块的 getSystemName 方法
                nativeFlushQueueImmediate([[0], [3], [[]], [2004]]);
            } else {
                console.log("nativeFlushQueueImmediate not available!");
            }

            console.log("JavaScript DeviceInfo test completed.");
        )";

    executor.loadApplicationScript(testScript, "deviceinfo_test.js");

    std::cout << "\n4. JavaScript Integration Test Completed!" << std::endl;

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
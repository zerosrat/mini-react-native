#include <iostream>
#include <memory>
#include <vector>

#include "common/bridge/JSCExecutor.h"
#include "common/modules/ModuleRegistry.h"
#include "MockModule.h"

/**
 * 测试模块框架的基础功能
 *
 * 这个测试程序验证：
 * 1. ModuleRegistry 的模块注册功能
 * 2. 模块ID和方法ID的分配
 * 3. 模块方法调用
 * 4. 回调机制
 * 5. JSCExecutor 与 ModuleRegistry 的集成
 */

void testModuleRegistration() {
    std::cout << "\n=== 测试模块注册 ===" << std::endl;

    // 创建模块注册器
    auto registry = std::make_unique<mini_rn::modules::ModuleRegistry>();

    // 创建测试模块
    auto mockModule = std::make_unique<MockModule>();

    // 设置回调处理器
    mockModule->setCallbackHandler([](int callId, const std::string& result, bool isError) {
        std::cout << "[Test] Callback received - CallId: " << callId
                  << ", IsError: " << (isError ? "true" : "false")
                  << ", Result: " << result << std::endl;
    });

    // 注册模块
    std::vector<std::unique_ptr<mini_rn::modules::NativeModule>> modules;
    modules.push_back(std::move(mockModule));
    registry->registerModules(std::move(modules));

    // 验证模块注册结果
    std::vector<std::string> moduleNames = registry->moduleNames();
    std::cout << "注册的模块数量: " << moduleNames.size() << std::endl;
    for (const auto& name : moduleNames) {
        std::cout << "模块名称: " << name << std::endl;
    }

    std::cout << "模块注册测试完成" << std::endl;
}

void testModuleMethodCall() {
    std::cout << "\n=== 测试模块方法调用 ===" << std::endl;

    // 创建模块注册器
    auto registry = std::make_unique<mini_rn::modules::ModuleRegistry>();

    // 设置回调处理器
    registry->setCallbackHandler([](int callId, const std::string& result, bool isError) {
        std::cout << "[Test] Registry callback - CallId: " << callId
                  << ", IsError: " << (isError ? "true" : "false")
                  << ", Result: " << result << std::endl;
    });

    // 创建并注册测试模块
    auto mockModule = std::make_unique<MockModule>();
    mockModule->setCallbackHandler([&registry](int callId, const std::string& result, bool isError) {
        // 模块回调转发到注册器
        if (registry) {
            auto handler = [callId, result, isError](int, const std::string&, bool) {
                std::cout << "[Test] Module callback forwarded - CallId: " << callId
                          << ", IsError: " << (isError ? "true" : "false")
                          << ", Result: " << result << std::endl;
            };
            handler(callId, result, isError);
        }
    });

    std::vector<std::unique_ptr<mini_rn::modules::NativeModule>> modules;
    modules.push_back(std::move(mockModule));
    registry->registerModules(std::move(modules));

    // 测试方法调用
    std::cout << "调用 testMethod..." << std::endl;
    registry->callNativeMethod(0, 0, "{\"test\": \"value\"}", 1001);

    std::cout << "调用 echoMessage..." << std::endl;
    registry->callNativeMethod(0, 1, "{\"message\": \"hello world\"}", 1002);

    std::cout << "调用 throwError..." << std::endl;
    registry->callNativeMethod(0, 2, "{}", 1003);

    std::cout << "调用不存在的方法..." << std::endl;
    registry->callNativeMethod(0, 999, "{}", 1004);

    std::cout << "模块方法调用测试完成" << std::endl;
}

void testJSCExecutorIntegration() {
    std::cout << "\n=== 测试 JSCExecutor 集成 ===" << std::endl;

    try {
        // 创建 JSCExecutor
        auto executor = std::make_unique<mini_rn::bridge::JSCExecutor>();

        // 获取模块注册器
        auto* registry = executor->getModuleRegistry();
        if (!registry) {
            std::cout << "错误: 无法获取 ModuleRegistry" << std::endl;
            return;
        }

        // 创建并注册测试模块
        auto mockModule = std::make_unique<MockModule>();
        mockModule->setCallbackHandler([](int callId, const std::string& result, bool isError) {
            std::cout << "[Test] JSCExecutor integration callback - CallId: " << callId
                      << ", IsError: " << (isError ? "true" : "false")
                      << ", Result: " << result << std::endl;
        });

        std::vector<std::unique_ptr<mini_rn::modules::NativeModule>> modules;
        modules.push_back(std::move(mockModule));
        registry->registerModules(std::move(modules));

        // 验证集成
        std::cout << "JSCExecutor 中注册的模块数量: " << registry->getModuleCount() << std::endl;

        // 测试通过 JSCExecutor 调用模块方法
        std::cout << "通过 JSCExecutor 调用模块方法..." << std::endl;
        registry->callNativeMethod(0, 0, "{\"integration_test\": true}", 2001);

        std::cout << "JSCExecutor 集成测试完成" << std::endl;

    } catch (const std::exception& e) {
        std::cout << "JSCExecutor 集成测试失败: " << e.what() << std::endl;
    }
}

void testErrorHandling() {
    std::cout << "\n=== 测试错误处理 ===" << std::endl;

    auto registry = std::make_unique<mini_rn::modules::ModuleRegistry>();

    // 设置回调处理器
    registry->setCallbackHandler([](int callId, const std::string& result, bool isError) {
        std::cout << "[Test] Error handling callback - CallId: " << callId
                  << ", IsError: " << (isError ? "true" : "false")
                  << ", Result: " << result << std::endl;
    });

    // 测试无效的模块ID
    std::cout << "测试无效的模块ID..." << std::endl;
    registry->callNativeMethod(999, 0, "{}", 3001);

    // 注册模块后测试无效的方法ID
    auto mockModule = std::make_unique<MockModule>();
    std::vector<std::unique_ptr<mini_rn::modules::NativeModule>> modules;
    modules.push_back(std::move(mockModule));
    registry->registerModules(std::move(modules));

    std::cout << "测试无效的方法ID..." << std::endl;
    registry->callNativeMethod(0, 999, "{}", 3002);

    std::cout << "错误处理测试完成" << std::endl;
}

int main() {
    std::cout << "开始模块框架测试..." << std::endl;

    try {
        testModuleRegistration();
        testModuleMethodCall();
        testJSCExecutorIntegration();
        testErrorHandling();

        std::cout << "\n=== 所有测试完成 ===" << std::endl;
        std::cout << "模块框架基础功能正常工作！" << std::endl;

    } catch (const std::exception& e) {
        std::cout << "测试失败: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
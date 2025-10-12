#include <iostream>
#include <string>
#include "bridge/JSCExecutor.h"

using namespace mini_rn::bridge;

/**
 * 基础测试程序
 *
 * 这个程序用于验证 JSCExecutor 的基本功能：
 * 1. JavaScript 环境创建和初始化
 * 2. 简单的 JavaScript 代码执行
 * 3. Bridge 函数的注入和调用
 * 4. 错误处理机制
 */

void testJSCExecutor() {
    std::cout << "\n=== Mini React Native - JSCExecutor Basic Test ===" << std::endl;

    try {
        // 创建 JSCExecutor 实例
        std::cout << "\n1. Creating JSCExecutor..." << std::endl;
        JSCExecutor executor;

        // 设置异常处理器
        executor.setJSExceptionHandler([](const std::string& error) {
            std::cout << "[Exception Handler] " << error << std::endl;
        });

        // 测试基本的 JavaScript 执行
        std::cout << "\n2. Testing basic JavaScript execution..." << std::endl;

        std::string testScript = R"(
            console.log("Hello from JavaScript!");
            console.log("Testing global object:", typeof global);
            console.log("Development mode:", __DEV__);

            // 测试一些基本的 JavaScript 功能
            var message = "JavaScript environment is working!";
            console.log(message);

            // 测试数组和对象
            var testArray = [1, 2, 3];
            var testObject = { name: "Mini RN", version: "0.1.0" };
            console.log("Array length:", testArray.length);
            console.log("Object name:", testObject.name);
        )";

        executor.loadApplicationScript(testScript, "test_basic.js");

        // 测试 Bridge 函数调用
        std::cout << "\n3. Testing Bridge function calls..." << std::endl;

        std::string bridgeTestScript = R"(
            console.log("Testing Bridge functions...");

            // 测试日志函数
            if (typeof __nativeLoggingHook === 'function') {
                __nativeLoggingHook('INFO', 'This is a test log from JavaScript');
                __nativeLoggingHook('DEBUG', 'Bridge logging is working!');
            }

            // 测试消息队列函数 (目前只是打印)
            if (typeof __nativeFlushQueuedReactWork === 'function') {
                console.log("Calling __nativeFlushQueuedReactWork...");
                __nativeFlushQueuedReactWork([1], [0], [["test"]], [42]);
            }
        )";

        executor.loadApplicationScript(bridgeTestScript, "bridge_test.js");

        // 测试错误处理
        std::cout << "\n4. Testing error handling..." << std::endl;

        std::string errorTestScript = R"(
            console.log("Testing error handling...");
            try {
                // 故意制造一个错误
                nonExistentFunction();
            } catch (e) {
                console.log("Caught JavaScript error:", e.message);
            }
        )";

        executor.loadApplicationScript(errorTestScript, "error_test.js");

        std::cout << "\n5. JSCExecutor test completed successfully!" << std::endl;

    } catch (const std::exception& e) {
        std::cout << "\nTest failed with exception: " << e.what() << std::endl;
    }

    std::cout << "\n===========================================" << std::endl;
}

int main() {
    std::cout << "Mini React Native - Basic Functionality Test" << std::endl;
    std::cout << "This test verifies the core JSCExecutor implementation" << std::endl;

    testJSCExecutor();

    return 0;
}
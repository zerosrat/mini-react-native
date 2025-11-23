#include <iostream>
#include <chrono>
#include <memory>

#include "common/bridge/JSCExecutor.h"
#include "common/modules/DeviceInfoModule.h"

using namespace mini_rn::bridge;
using namespace mini_rn::modules;

/**
 * Mini React Native - 轻量级性能测试
 *
 * 专注于检测严重性能问题，不做复杂的性能分析
 * 主要验证 Bridge 通信没有明显的性能瓶颈
 */

/**
 * 快速性能检查 - 验证 Bridge 通信性能
 */
void quickPerformanceCheck() {
    std::cout << "\n🧪 Quick Performance Check" << std::endl;

    try {
        // 1. 创建 JSCExecutor
        auto executor = std::make_unique<JSCExecutor>();

        // 2. 注册 DeviceInfo 模块
        std::vector<std::unique_ptr<NativeModule>> modules;
        modules.push_back(std::make_unique<DeviceInfoModule>());
        executor->registerModules(std::move(modules));

        // 3. 测试 Bridge 调用延迟
        auto start = std::chrono::high_resolution_clock::now();

        // 执行几次关键的 Bridge 调用
        std::string testScript = R"(
            // 快速性能测试：调用几个关键方法
            try {
                var result1 = global.nativeCallSyncHook(0, 1, []); // getSystemVersion
                var result2 = global.nativeCallSyncHook(0, 2, []); // getDeviceId
                console.log('Performance test calls completed');
            } catch (e) {
                console.log('Performance test failed:', e.toString());
            }
        )";

        executor->loadApplicationScript(testScript, "performance_test.js");

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        // 4. 简单的性能阈值检查
        double ms = duration.count();
        std::cout << "   Bridge call latency: " << ms << "ms";

        if (ms < 10.0) {
            std::cout << " (< 10ms threshold)" << std::endl;
            std::cout << "✅ Bridge performance OK" << std::endl;
        } else if (ms < 50.0) {
            std::cout << " (< 50ms threshold)" << std::endl;
            std::cout << "⚠️ Bridge performance acceptable but slow" << std::endl;
        } else {
            std::cout << " (>= 50ms threshold)" << std::endl;
            std::cout << "❌ Bridge performance issue detected" << std::endl;
        }

    } catch (const std::exception& e) {
        std::cout << "❌ Performance test failed: " << e.what() << std::endl;
    }
}

/**
 * 模块注册性能检查
 */
void moduleRegistrationCheck() {
    std::cout << "\n🔧 Module Registration Performance Check" << std::endl;

    try {
        auto start = std::chrono::high_resolution_clock::now();

        // 测试模块注册性能
        auto executor = std::make_unique<JSCExecutor>();
        std::vector<std::unique_ptr<NativeModule>> modules;
        modules.push_back(std::make_unique<DeviceInfoModule>());
        executor->registerModules(std::move(modules));

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        double ms = duration.count() / 1000.0;
        std::cout << "   Module registration: " << ms << "ms" << std::endl;

        if (ms < 5.0) {
            std::cout << "✅ Module registration performance OK" << std::endl;
        } else {
            std::cout << "⚠️ Module registration slower than expected" << std::endl;
        }

    } catch (const std::exception& e) {
        std::cout << "❌ Module registration test failed: " << e.what() << std::endl;
    }
}

int main() {
    std::cout << "Mini React Native - Performance Test" << std::endl;
    std::cout << "Lightweight performance check for critical performance issues" << std::endl;
    std::cout << std::endl;

    std::cout << "=== Performance Test ===" << std::endl;

    // 执行轻量级性能检查
    moduleRegistrationCheck();
    quickPerformanceCheck();

    std::cout << "\n🏁 Performance test completed" << std::endl;
    std::cout << "   No severe performance issues detected if all checks passed" << std::endl;

    return 0;
}
#include <iostream>
#include <chrono>
#include "common/bridge/JSCExecutor.h"
#include "common/modules/DeviceInfoModule.h"

using namespace mini_rn::bridge;
using namespace mini_rn::modules;

int main() {
    std::cout << "Mini React Native - iOS DeviceInfo Test" << std::endl;
    std::cout << "This test verifies DeviceInfo module on iOS platform" << std::endl;
    std::cout << std::endl;

    std::cout << "=== iOS DeviceInfo Module Test ===" << std::endl;

    try {
        // 1. 创建 JSCExecutor
        std::cout << "\n1. Creating JSCExecutor..." << std::endl;
        auto executor = std::make_unique<JSCExecutor>();

        // 2. 创建 DeviceInfo 模块并直接测试
        std::cout << "2. Testing DeviceInfo methods directly..." << std::endl;
        auto deviceInfo = std::make_unique<DeviceInfoModule>();

        std::cout << "   UniqueId: " << deviceInfo->getUniqueIdImpl() << std::endl;
        std::cout << "   SystemVersion: " << deviceInfo->getSystemVersionImpl() << std::endl;
        std::cout << "   DeviceId: " << deviceInfo->getDeviceIdImpl() << std::endl;

        // 3. 注册模块到 JSCExecutor
        std::cout << "\n3. Registering DeviceInfo module..." << std::endl;
        std::vector<std::unique_ptr<NativeModule>> modules;
        modules.push_back(std::move(deviceInfo));
        executor->registerModules(std::move(modules));

        // 4. 测试同步调用性能
        std::cout << "\n4. Testing Bridge communication performance..." << std::endl;

        auto start = std::chrono::high_resolution_clock::now();

        std::string testScript = R"(
            // 测试 iOS DeviceInfo 方法
            console.log('🍎 iOS DeviceInfo Bridge 测试开始...');

            try {
                // getSystemVersion (methodId = 1)
                var systemVersion = global.nativeCallSyncHook(0, 1, []);
                console.log('✅ iOS SystemVersion:', systemVersion);

                // getDeviceId (methodId = 2)
                var deviceId = global.nativeCallSyncHook(0, 2, []);
                console.log('✅ iOS DeviceId:', deviceId);

                console.log('🎉 iOS DeviceInfo 测试成功!');
            } catch (e) {
                console.log('❌ 测试失败:', e.toString());
            }
        )";

        executor->loadApplicationScript(testScript, "ios_deviceinfo_test.js");

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        std::cout << "\n5. Performance Results:" << std::endl;
        std::cout << "   Bridge call duration: " << duration.count() / 1000.0 << " ms" << std::endl;

        if (duration.count() / 1000.0 < 10.0) {
            std::cout << "   ✅ Performance requirement met (< 10ms)" << std::endl;
        } else {
            std::cout << "   ⚠️ Performance slower than expected (>= 10ms)" << std::endl;
        }

        std::cout << "\n6. iOS DeviceInfo test completed successfully!" << std::endl;

    } catch (const std::exception& e) {
        std::cout << "❌ Test failed: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "\n===========================================" << std::endl;
    return 0;
}
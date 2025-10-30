/**
 * test_deviceinfo_integration.js - DeviceInfo 模块集成测试
 *
 * 这个脚本专门用于测试 DeviceInfo.js 与 examples/test_deviceinfo.cpp 的集成。
 * 验证所有四种方法类型的正确调用和回调。
 *
 * 测试映射：
 * - JavaScript getUniqueId() → C++ methodId=0 (Promise)
 * - JavaScript getSystemVersion() → C++ methodId=1 (Sync)
 * - JavaScript getModel(callback) → C++ methodId=2 (Callback)
 * - JavaScript getSystemName(callback) → C++ methodId=3 (Callback)
 */

'use strict';

console.log('🔥 DeviceInfo Integration Test Starting...');

// 全局测试状态
const testState = {
  totalTests: 4,
  completedTests: 0,
  passedTests: 0,
  results: {}
};

// 测试结果记录
function recordTestResult(methodName, success, result, error) {
  testState.results[methodName] = {
    success,
    result: success ? result : null,
    error: success ? null : error,
    timestamp: new Date().toISOString()
  };

  testState.completedTests++;
  if (success) testState.passedTests++;

  console.log(`📊 Progress: ${testState.completedTests}/${testState.totalTests} tests completed`);

  // 如果所有测试完成，显示最终报告
  if (testState.completedTests === testState.totalTests) {
    showFinalReport();
  }
}

// 显示最终测试报告
function showFinalReport() {
  console.log('\n🏁 === Final Test Report ===');
  console.log(`✅ Passed: ${testState.passedTests}/${testState.totalTests}`);
  console.log(`❌ Failed: ${testState.totalTests - testState.passedTests}/${testState.totalTests}`);

  console.log('\n📋 Detailed Results:');
  Object.entries(testState.results).forEach(([method, result]) => {
    const status = result.success ? '✅' : '❌';
    const value = result.success ? result.result : result.error;
    console.log(`  ${status} ${method}: ${value}`);
  });

  if (testState.passedTests === testState.totalTests) {
    console.log('\n🎉 ALL TESTS PASSED! DeviceInfo integration is working correctly.');
  } else {
    console.log('\n⚠️  Some tests failed. Check the C++ output for Native method execution details.');
  }

  console.log('\n💡 Verification checklist:');
  console.log('  1. ✓ JavaScript methods called successfully');
  console.log('  2. ? Check C++ console for Native method invocations');
  console.log('  3. ? Verify callback IDs match between JS and C++');
  console.log('  4. ? Confirm return values are correctly passed back');
}

// 主测试函数
async function runDeviceInfoIntegrationTest() {
  try {
    console.log('\n🔧 Loading required modules...');

    // 加载核心模块
    const BatchedBridge = require('../../src/js/BatchedBridge');
    console.log('  ✓ BatchedBridge loaded');

    const DeviceInfo = require('../../src/js/DeviceInfo');
    console.log('  ✓ DeviceInfo loaded');

    // 等待模块初始化
    console.log('\n⏳ Waiting for module initialization...');
    await new Promise(resolve => setTimeout(resolve, 200));

    console.log('\n🧪 Starting DeviceInfo method tests...');
    console.log('   (These should trigger corresponding C++ method calls)\n');

    // 测试 1: getUniqueId (Promise 方法)
    console.log('1️⃣ Testing getUniqueId() → Promise<string> (methodId=0)');
    try {
      const uniqueId = await DeviceInfo.getUniqueId();
      console.log('   📤 JavaScript call completed');
      console.log('   📥 Promise resolved with:', uniqueId);
      recordTestResult('getUniqueId', true, uniqueId);
    } catch (error) {
      console.log('   ❌ Promise rejected:', error.message);
      recordTestResult('getUniqueId', false, null, error.message);
    }

    // 测试 2: getSystemVersion (同步方法)
    console.log('\n2️⃣ Testing getSystemVersion() → string (methodId=1)');
    try {
      const systemVersion = DeviceInfo.getSystemVersion();
      console.log('   📤 JavaScript call completed');
      console.log('   📥 Sync result:', systemVersion);
      recordTestResult('getSystemVersion', true, systemVersion);
    } catch (error) {
      console.log('   ❌ Sync call failed:', error.message);
      recordTestResult('getSystemVersion', false, null, error.message);
    }

    // 测试 3: getModel (回调方法)
    console.log('\n3️⃣ Testing getModel(callback) → void (methodId=2)');
    DeviceInfo.getModel((error, result) => {
      console.log('   📤 JavaScript call initiated');
      if (error) {
        console.log('   ❌ Callback received error:', error);
        recordTestResult('getModel', false, null, error);
      } else {
        console.log('   📥 Callback received result:', result);
        recordTestResult('getModel', true, result);
      }
    });

    // 测试 4: getSystemName (回调方法)
    console.log('\n4️⃣ Testing getSystemName(callback) → void (methodId=3)');
    DeviceInfo.getSystemName((error, result) => {
      console.log('   📤 JavaScript call initiated');
      if (error) {
        console.log('   ❌ Callback received error:', error);
        recordTestResult('getSystemName', false, null, error);
      } else {
        console.log('   📥 Callback received result:', result);
        recordTestResult('getSystemName', true, result);
      }
    });

    console.log('\n⏱️  Waiting for async callbacks to complete...');

  } catch (error) {
    console.error('💥 Integration test failed:', error.message);
    console.error('Stack:', error.stack);
  }
}

// 环境检查
function checkEnvironment() {
  console.log('\n🔍 Environment Check:');

  // 检查全局对象
  const checks = [
    { name: 'global', condition: typeof global !== 'undefined' },
    { name: 'console', condition: typeof console !== 'undefined' },
    { name: 'nativeFlushQueueImmediate', condition: typeof nativeFlushQueueImmediate !== 'undefined' },
    { name: '__fbBatchedBridgeConfig', condition: typeof global.__fbBatchedBridgeConfig !== 'undefined' }
  ];

  checks.forEach(check => {
    const status = check.condition ? '✅' : '❌';
    console.log(`  ${status} ${check.name}`);
  });

  // 显示模块配置信息
  if (global.__fbBatchedBridgeConfig) {
    const config = global.__fbBatchedBridgeConfig;
    console.log('\n📋 Module Configuration:');
    if (config.remoteModuleConfig) {
      config.remoteModuleConfig.forEach((moduleConfig, index) => {
        const [name, constants, methods] = moduleConfig;
        console.log(`  📦 Module ${index}: ${name}`);
        console.log(`     Methods: ${methods.join(', ')}`);
      });
    }
  }

  return checks.every(check => check.condition);
}

// 启动测试
console.log('🚀 Initializing DeviceInfo Integration Test...');

if (checkEnvironment()) {
  console.log('\n✅ Environment check passed, starting tests...');
  runDeviceInfoIntegrationTest();
} else {
  console.log('\n❌ Environment check failed, cannot proceed with tests');
}

// 设置超时保护
setTimeout(() => {
  if (testState.completedTests < testState.totalTests) {
    console.log('\n⏰ Test timeout reached');
    console.log(`   Completed: ${testState.completedTests}/${testState.totalTests}`);
    console.log('   Some callbacks may not have been executed');
    showFinalReport();
  }
}, 5000);  // 5秒超时
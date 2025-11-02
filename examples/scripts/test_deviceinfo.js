/**
 * test_deviceinfo.js - DeviceInfo 模块集成测试 (纯测试逻辑版本)
 *
 * 这个脚本专门用于测试 DeviceInfo.js 与 examples/test_deviceinfo.cpp 的集成。
 * 验证所有四种方法类型的正确调用和回调。
 *
 * 模块化架构：
 * - 依赖的模块通过 C++ 预先加载到 global 对象
 * - 本文件只包含纯测试逻辑，不包含模块实现
 *
 * 测试映射：
 * - JavaScript getUniqueId() → C++ methodId=0 (Promise)
 * - JavaScript getSystemVersion() → C++ methodId=1 (Sync)
 * - JavaScript getDeviceId() → C++ methodId=2 (Sync)
 */

'use strict'

console.log('🔥 DeviceInfo Integration Test Starting...')

// ===== 获取预加载的模块依赖 =====
console.log('📦 Loading dependencies from global objects...')

// 从 global 对象获取预加载的模块
// MessageQueue = global.MessageQueue
// BatchedBridge = global.BatchedBridge || global.__fbBatchedBridge
// NativeModules = global.NativeModules
// DeviceInfo = global.DeviceInfo

// 验证依赖是否正确加载
const dependencies = [
  { name: 'MessageQueue', obj: MessageQueue },
  { name: 'BatchedBridge', obj: BatchedBridge },
  { name: 'NativeModules', obj: NativeModules },
  { name: 'DeviceInfo', obj: DeviceInfo },
]

let dependenciesLoaded = true
dependencies.forEach((dep) => {
  if (dep.obj) {
    console.log(`  ✅ ${dep.name} loaded successfully`)
  } else {
    console.log(`  ❌ ${dep.name} not found in global`)
    dependenciesLoaded = false
  }
})

if (!dependenciesLoaded) {
  console.log('💥 Dependency loading failed, cannot proceed with tests')
  // 在嵌入式环境中，我们无法抛出异常来停止执行，只能记录错误
  // 使用 try-catch 或直接退出，而不是 return
  throw new Error('Dependencies not loaded')
}

console.log('✅ All dependencies loaded successfully')

// ===== 测试逻辑 =====

// 全局测试状态
const testState = {
  totalTests: 3,
  completedTests: 0,
  passedTests: 0,
  results: {},
}

// 测试结果记录
function recordTestResult(methodName, success, result, error) {
  testState.results[methodName] = {
    success,
    result: success ? result : null,
    error: success ? null : error,
    timestamp: new Date().toISOString(),
  }

  testState.completedTests++
  if (success) testState.passedTests++

  console.log(`📊 Progress: ${testState.completedTests}/${testState.totalTests} tests completed`)

  // 如果所有测试完成，显示最终报告
  if (testState.completedTests === testState.totalTests) {
    showFinalReport()
  }
}

// 显示最终测试报告
function showFinalReport() {
  console.log('\n🏁 === Final Test Report ===')
  console.log(`✅ Passed: ${testState.passedTests}/${testState.totalTests}`)
  console.log(`❌ Failed: ${testState.totalTests - testState.passedTests}/${testState.totalTests}`)

  console.log('\n📋 Detailed Results:')
  Object.entries(testState.results).forEach(([method, result]) => {
    const status = result.success ? '✅' : '❌'
    const value = result.success ? result.result : result.error
    console.log(`  ${status} ${method}: ${value}`)
  })

  if (testState.passedTests === testState.totalTests) {
    console.log('\n🎉 ALL TESTS PASSED! DeviceInfo integration is working correctly.')
  } else {
    console.log('\n⚠️  Some tests failed. Check the C++ output for Native method execution details.')
  }

  console.log('\n💡 Verification checklist:')
  console.log('  1. ✓ JavaScript methods called successfully')
  console.log('  2. ? Check C++ console for Native method invocations')
  console.log('  3. ? Verify callback IDs match between JS and C++')
  console.log('  4. ? Confirm return values are correctly passed back')
}

// 主测试函数
async function runDeviceInfoIntegrationTest() {
  try {
    console.log('\n🔧 Initializing test environment...')

    // 初始化 NativeModules 系统（如果需要）
    if (NativeModules && typeof NativeModules.initialize === 'function') {
      console.log('  ✓ Initializing NativeModules system...')
      NativeModules.initialize()
      console.log('  ✓ NativeModules initialized')
    } else {
      console.log('  ℹ️  NativeModules already initialized or no initialization needed')
    }

    // 等待模块初始化
    console.log('\n⏳ Waiting for module initialization...')
    // Note: setTimeout not available in embedded environment, skipping delay

    console.log('\n🧪 Starting DeviceInfo method tests...')
    console.log('   (These should trigger corresponding C++ method calls)\n')

    // 测试 1: getUniqueId (Promise 方法)
    console.log('1️⃣ Testing getUniqueId() → Promise<string> (methodId=0)')
    try {
      const uniqueId = await DeviceInfo.getUniqueId()
      console.log('   📤 JavaScript call completed')
      console.log('   📥 Promise resolved with:', uniqueId)
      recordTestResult('getUniqueId', true, uniqueId)
    } catch (error) {
      console.log('   ❌ Promise rejected:', error.message)
      recordTestResult('getUniqueId', false, null, error.message)
    }

    // 测试 2: getSystemVersion (同步方法)
    console.log('\n2️⃣ Testing getSystemVersion() → string (methodId=1)')
    try {
      const systemVersion = DeviceInfo.getSystemVersion()
      console.log('   📤 JavaScript call completed')
      console.log('   📥 Sync result:', systemVersion)
      recordTestResult('getSystemVersion', true, systemVersion)
    } catch (error) {
      console.log('   ❌ Sync call failed:', error.message)
      recordTestResult('getSystemVersion', false, null, error.message)
    }

    // 测试 3: getDeviceId (同步方法)
    console.log('\n3️⃣ Testing getDeviceId() → string (methodId=2)')
    try {
      const deviceId = DeviceInfo.getDeviceId()
      console.log('   📤 JavaScript call completed')
      console.log('   📥 Sync result:', deviceId)
      recordTestResult('getDeviceId', true, deviceId)
    } catch (error) {
      console.log('   ❌ Sync call failed:', error.message)
      recordTestResult('getDeviceId', false, null, error.message)
    }

    console.log('\n⏱️  Waiting for async callbacks to complete...')
  } catch (error) {
    console.error('💥 Integration test failed:', error.message)
    console.error('Stack:', error.stack)
  }
}

// 环境检查
function checkEnvironment() {
  console.log('\n🔍 Environment Check:')

  // 检查全局对象
  const checks = [
    { name: 'global', condition: typeof global !== 'undefined' },
    { name: 'console', condition: typeof console !== 'undefined' },
    { name: 'nativeFlushQueueImmediate', condition: typeof nativeFlushQueueImmediate !== 'undefined' },
    { name: '__fbBatchedBridgeConfig', condition: typeof global.__fbBatchedBridgeConfig !== 'undefined' },
    { name: 'MessageQueue', condition: typeof MessageQueue !== 'undefined' },
    { name: 'BatchedBridge', condition: typeof BatchedBridge !== 'undefined' },
    { name: 'NativeModules', condition: typeof NativeModules !== 'undefined' },
    { name: 'DeviceInfo', condition: typeof DeviceInfo !== 'undefined' },
  ]

  checks.forEach((check) => {
    const status = check.condition ? '✅' : '❌'
    console.log(`  ${status} ${check.name}`)
  })

  // 显示模块配置信息
  if (global.__fbBatchedBridgeConfig) {
    const config = global.__fbBatchedBridgeConfig
    console.log('\n📋 Module Configuration:')
    if (config.remoteModuleConfig) {
      config.remoteModuleConfig.forEach((moduleConfig, index) => {
        const [name, constants, methods] = moduleConfig
        console.log(`  📦 Module ${index}: ${name}`)
        console.log(`     Methods: ${methods.join(', ')}`)
      })
    }
  }

  // 允许测试在某些检查失败时继续进行
  const criticalChecks = checks.filter(
    (check) =>
      check.name === 'global' ||
      check.name === 'console' ||
      check.name === 'nativeFlushQueueImmediate' ||
      check.name === 'DeviceInfo',
  )
  return criticalChecks.every((check) => check.condition)
}

// 启动测试
console.log('🚀 Initializing DeviceInfo Integration Test...')

if (checkEnvironment()) {
  console.log('\n✅ Environment check passed, starting tests...')
  runDeviceInfoIntegrationTest()
} else {
  console.log('\n❌ Environment check failed, cannot proceed with tests')
}

// Note: Timeout protection disabled in embedded environment
// In a real React Native environment, this would use setTimeout for timeout protection

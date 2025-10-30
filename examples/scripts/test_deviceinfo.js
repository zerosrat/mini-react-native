/**
 * test_deviceinfo.js - DeviceInfo 模块集成测试 (自包含版本)
 *
 * 这个脚本专门用于测试 DeviceInfo.js 与 examples/test_deviceinfo.cpp 的集成。
 * 验证所有四种方法类型的正确调用和回调。
 *
 * 自包含版本：包含所有必要的模块代码，无需 require() 机制
 *
 * 测试映射：
 * - JavaScript getUniqueId() → C++ methodId=0 (Promise)
 * - JavaScript getSystemVersion() → C++ methodId=1 (Sync)
 * - JavaScript getModel(callback) → C++ methodId=2 (Callback)
 * - JavaScript getSystemName(callback) → C++ methodId=3 (Callback)
 */

'use strict';

console.log('🔥 DeviceInfo Integration Test Starting...');

// ===== MessageQueue 模块 (内联) =====
/**
 * MessageQueue - Mini React Native 消息队列实现
 */
class MessageQueue {
  constructor() {
    // 消息队列：等待发送到 Native 的调用
    this._queue = [[], [], [], []] // [moduleIDs, methodIDs, params, callbackIDs]

    // 回调管理
    this._callbackID = 0 // 回调ID生成器
    this._callbacks = {} // 存储所有待执行的回调函数

    // 模块注册表
    this._lazyCallableModules = {} // 延迟加载的模块
    this._modules = {} // 已实例化的模块

    // 性能和调试
    this._isInCallback = false // 标记是否正在执行回调
    this._debugEnabled = true // 调试模式标志

    console.log('[MessageQueue] Initialized with RN-compatible structure')
  }

  /**
   * 调用 Native 模块方法
   */
  enqueueNativeCall(moduleID, methodID, params, onFail, onSucc) {
    if (this._debugEnabled) {
      console.log(`[MessageQueue] Calling native method - Module: ${moduleID}, Method: ${methodID}`)
    }

    // 验证参数
    if (typeof moduleID !== 'number' || typeof methodID !== 'number') {
      console.error('[MessageQueue] Invalid moduleID or methodID')
      if (onFail) onFail('Invalid module or method ID')
      return
    }

    // 生成回调ID并注册回调函数
    let callbackID = null
    if (onFail || onSucc) {
      callbackID = this._callbackID++
      this._callbacks[callbackID] = {
        onFail: onFail,
        onSucc: onSucc,
      }
    }

    // 将调用添加到队列中
    this._queue[0].push(moduleID) // moduleIds
    this._queue[1].push(methodID) // methodIds
    this._queue[2].push(params || []) // params
    this._queue[3].push(callbackID) // callbackIds

    if (this._debugEnabled) {
      console.log(`[MessageQueue] Queued call - Queue length: ${this._queue[0].length}`)
    }

    // 立即刷新队列
    this._flushQueue()
  }

  /**
   * 获取并清空待处理队列
   */
  flushedQueue() {
    if (this._debugEnabled) {
      console.log(`[MessageQueue] Flushing queue with ${this._queue[0].length} calls`)
    }

    // 返回当前队列并创建新的空队列
    const queue = this._queue
    this._queue = [[], [], [], []]

    return queue
  }

  /**
   * 执行回调并返回新的队列
   */
  invokeCallbackAndReturnFlushedQueue(callbackID, args) {
    if (this._debugEnabled) {
      console.log(`[MessageQueue] Invoking callback: ${callbackID}`)
    }

    this._isInCallback = true

    try {
      this._invokeCallback(callbackID, args)
    } catch (error) {
      console.error('[MessageQueue] Error invoking callback:', error)
    } finally {
      this._isInCallback = false
    }

    // 返回在回调执行过程中可能产生的新调用
    return this.flushedQueue()
  }

  /**
   * 刷新消息队列到 Native
   */
  _flushQueue() {
    if (this._queue[0].length === 0) {
      return // 队列为空，无需刷新
    }

    if (typeof global.nativeFlushQueueImmediate === 'function') {
      // 获取当前队列
      const queue = this.flushedQueue()

      if (this._debugEnabled) {
        console.log('[MessageQueue] Flushing to native:', {
          moduleIds: queue[0],
          methodIds: queue[1],
          params: queue[2],
          callbackIds: queue[3],
        })
      }

      // 调用 Native 函数
      global.nativeFlushQueueImmediate(queue)
    } else {
      console.error('[MessageQueue] nativeFlushQueueImmediate not available')
    }
  }

  /**
   * 执行单个回调
   */
  _invokeCallback(callbackID, args) {
    const callback = this._callbacks[callbackID]

    if (!callback) {
      console.error(`[MessageQueue] Callback ${callbackID} not found`)
      return
    }

    // 清理回调引用，避免内存泄漏
    delete this._callbacks[callbackID]

    try {
      // RN 的回调约定：第一个参数是错误，后续参数是结果
      if (args && args[0] != null) {
        // 有错误，调用失败回调
        if (callback.onFail) {
          callback.onFail(args[0])
        }
      } else {
        // 成功，调用成功回调
        if (callback.onSucc) {
          const result = args ? args.slice(1) : []
          callback.onSucc.apply(null, result)
        }
      }
    } catch (error) {
      console.error('[MessageQueue] Error in callback execution:', error)
    }
  }
}

// ===== BatchedBridge 模块 (内联) =====
// 创建 MessageQueue 实例作为 BatchedBridge
const BatchedBridge = new MessageQueue();

// 设置全局 Bridge 实例
Object.defineProperty(global, '__fbBatchedBridge', {
  configurable: true,
  value: BatchedBridge,
});

console.log('[BatchedBridge] BatchedBridge initialized');

// ===== NativeModule 模块 (内联) =====
// 类型定义
const MethodType = {
  ASYNC: 'async',
  PROMISE: 'promise',
  SYNC: 'sync'
};

/**
 * 辅助函数
 */
function arrayContains(array, value) {
  return array && array.indexOf(value) !== -1;
}

function createErrorFromErrorData(errorData) {
  const {message, ...extraErrorInfo} = errorData || {};
  const error = new Error(message);
  error.framesToPop = 1;
  return Object.assign(error, extraErrorInfo);
}

function invariant(condition, message) {
  if (!condition) {
    throw new Error(`Invariant Violation: ${message}`);
  }
}

/**
 * 生成单个方法的代理函数
 */
function genMethod(moduleID, methodID, type) {
  let fn = null;

  if (type === MethodType.PROMISE) {
    // Promise 方法：返回 Promise 对象
    fn = function(...args) {
      return new Promise((resolve, reject) => {
        BatchedBridge.enqueueNativeCall(
          moduleID,
          methodID,
          args,
          data => resolve(data),
          errorData => reject(createErrorFromErrorData(errorData))
        );
      });
    };
  } else if (type === MethodType.SYNC) {
    // 同步方法：使用 nativeCallSyncHook
    fn = function(...args) {
      if (typeof global.nativeCallSyncHook !== 'function') {
        invariant(
          false,
          'Calling synchronous methods on native modules is not supported. ' +
          'Consider providing alternative methods to expose this method in debug mode.'
        );
      }
      return global.nativeCallSyncHook(moduleID, methodID, args);
    };
  } else {
    // 异步方法：支持双回调模式
    fn = function(...args) {
      const lastArg = args.length > 0 ? args[args.length - 1] : null;
      const secondLastArg = args.length > 1 ? args[args.length - 2] : null;
      const hasSuccessCallback = typeof lastArg === 'function';
      const hasErrorCallback = typeof secondLastArg === 'function';

      if (hasErrorCallback) {
        invariant(
          hasSuccessCallback,
          'Cannot have a non-function arg after a function arg.'
        );
      }

      const onSuccess = hasSuccessCallback ? lastArg : null;
      const onFail = hasErrorCallback ? secondLastArg : null;
      const callbackCount = (hasSuccessCallback ? 1 : 0) + (hasErrorCallback ? 1 : 0);
      const actualArgs = args.slice(0, args.length - callbackCount);

      BatchedBridge.enqueueNativeCall(
        moduleID,
        methodID,
        actualArgs,
        onFail,
        onSuccess
      );
    };
  }

  fn.type = type;
  return fn;
}

/**
 * 生成模块代理对象
 */
function genModule(config, moduleID) {
  if (!config) {
    return null;
  }

  const [moduleName, constants, methods, promiseMethods, syncMethods] = config;

  invariant(
    !moduleName.startsWith('RCT') && !moduleName.startsWith('RK'),
    `Module name prefixes should've been stripped by the native side but wasn't for ${moduleName}`
  );

  if (!constants && !methods) {
    return {name: moduleName};
  }

  const module = {};

  // 生成所有方法
  if (methods) {
    methods.forEach((methodName, methodID) => {
      const isPromise = arrayContains(promiseMethods, methodID);
      const isSync = arrayContains(syncMethods, methodID);

      invariant(
        !isPromise || !isSync,
        'Cannot have a method that is both async and a sync hook'
      );

      const methodType = isPromise ? MethodType.PROMISE :
                        isSync ? MethodType.SYNC :
                        MethodType.ASYNC;

      module[methodName] = genMethod(moduleID, methodID, methodType);
    });
  }

  // 添加常量到模块对象
  if (constants) {
    Object.assign(module, constants);
  }

  if (typeof global.__DEV__ !== 'undefined' && global.__DEV__) {
    console.log(`[NativeModule] Created module: ${moduleName} with methods:`, methods);
  }

  return {name: moduleName, module};
}

// 创建 NativeModules 对象
let NativeModules = {};

// 模块初始化逻辑
function initializeNativeModules() {
  const bridgeConfig = global.__fbBatchedBridgeConfig;

  invariant(
    bridgeConfig,
    '__fbBatchedBridgeConfig is not set, cannot invoke native modules'
  );

  // 处理所有远程模块配置
  const remoteModuleConfig = bridgeConfig.remoteModuleConfig || [];

  remoteModuleConfig.forEach((config, moduleID) => {
    // 生成模块信息
    const info = genModule(config, moduleID);
    if (!info) {
      return;
    }

    if (info.module) {
      // 直接注册模块
      NativeModules[info.name] = info.module;
    }
  });

  console.log('[NativeModule] Initialized', remoteModuleConfig.length, 'native modules');
}

// 提供兼容的接口
const NativeModuleInterface = {
  get(moduleName) {
    return NativeModules[moduleName] || null;
  },

  getAll() {
    return NativeModules;
  },

  initialize() {
    if (global.__fbBatchedBridgeConfig) {
      initializeNativeModules();
    } else {
      console.warn('[NativeModule] Cannot initialize: __fbBatchedBridgeConfig not available');
    }
  }
};

console.log('[NativeModule] NativeModule system loaded');

// ===== DeviceInfo 模块 (内联) =====
// DeviceInfo 原生模块
let DeviceInfoNative = null;

// 延迟获取原生模块
function getDeviceInfoNative() {
  if (!DeviceInfoNative) {
    DeviceInfoNative = NativeModuleInterface.get('DeviceInfo');

    if (!DeviceInfoNative) {
      console.error('[DeviceInfo] Native module not found. Available modules:',
                   Object.keys(NativeModuleInterface.getAll()));
      throw new Error('DeviceInfo native module is not available');
    }

    console.log('[DeviceInfo] Native module loaded successfully');
  }

  return DeviceInfoNative;
}

/**
 * DeviceInfo 模块导出接口
 */
const DeviceInfo = {
  /**
   * 获取设备唯一标识符
   */
  getUniqueId() {
    console.log('[DeviceInfo] Calling getUniqueId (Promise method)');
    const native = getDeviceInfoNative();
    return native.getUniqueId();
  },

  /**
   * 获取系统版本
   */
  getSystemVersion() {
    console.log('[DeviceInfo] Calling getSystemVersion (Sync method)');
    const native = getDeviceInfoNative();
    return native.getSystemVersion();
  },

  /**
   * 获取设备型号
   */
  getModel(callback) {
    console.log('[DeviceInfo] Calling getModel (Callback method)');

    if (!callback || typeof callback !== 'function') {
      console.error('[DeviceInfo] getModel requires a callback function');
      return;
    }

    const native = getDeviceInfoNative();
    native.getModel(callback);
  },

  /**
   * 获取系统名称
   */
  getSystemName(callback) {
    console.log('[DeviceInfo] Calling getSystemName (Callback method)');

    if (!callback || typeof callback !== 'function') {
      console.error('[DeviceInfo] getSystemName requires a callback function');
      return;
    }

    const native = getDeviceInfoNative();
    native.getSystemName(callback);
  },

  // 提供直接访问原生模块的方法（用于调试）
  _getNativeModule() {
    return getDeviceInfoNative();
  },

  // 检查模块是否可用
  isAvailable() {
    try {
      getDeviceInfoNative();
      return true;
    } catch (error) {
      return false;
    }
  }
};

console.log('[DeviceInfo] DeviceInfo module loaded');

// ===== 测试逻辑 =====

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
    console.log('\n🔧 Initializing modules...');

    // 初始化 NativeModules 系统
    if (global.__fbBatchedBridgeConfig) {
      console.log('  ✓ Module config found, initializing NativeModules...');
      NativeModuleInterface.initialize();
      console.log('  ✓ NativeModules initialized');
    } else {
      console.log('  ⚠️  Module config not found, will initialize on first use');
    }

    // 等待模块初始化
    console.log('\n⏳ Waiting for module initialization...');
    // Note: setTimeout not available in embedded environment, skipping delay

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

  // Allow test to proceed even if some environment checks fail
  const criticalChecks = checks.filter(check =>
    check.name === 'global' || check.name === 'console' || check.name === 'nativeFlushQueueImmediate'
  );
  return criticalChecks.every(check => check.condition);
}

// 启动测试
console.log('🚀 Initializing DeviceInfo Integration Test...');

if (checkEnvironment()) {
  console.log('\n✅ Environment check passed, starting tests...');
  runDeviceInfoIntegrationTest();
} else {
  console.log('\n❌ Environment check failed, cannot proceed with tests');
}

// Note: Timeout protection disabled in embedded environment
// In a real React Native environment, this would use setTimeout for timeout protection
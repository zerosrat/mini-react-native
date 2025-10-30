/**
 * DeviceInfo.js - React Native 兼容的设备信息模块
 *
 * 基于标准的 React Native NativeModules 系统，提供设备信息相关的 JavaScript API。
 * 与 Native 端的 DeviceInfoModule 对应，支持四种方法类型：
 *
 * - getUniqueId() → Promise<string> (Promise方式)
 * - getSystemVersion() → string (同步返回方式)
 * - getModel(callback) → void (单回调方式)
 * - getSystemName(callback) → void (单回调方式)
 *
 * 设计原则：
 * - 与 React Native 的 DeviceInfo 模块 API 完全兼容
 * - 直接使用 NativeModules 系统生成的代理对象
 * - 遵循 React Native 的方法调用约定
 */

'use strict';

// 获取 NativeModules（适配多种加载方式）
const NativeModules = (function() {
  if (typeof require !== 'undefined') {
    return require('./NativeModule');
  } else if (typeof global !== 'undefined' && global.NativeModules) {
    return global.NativeModules;
  } else {
    console.error('[DeviceInfo] NativeModules not found');
    return null;
  }
})();

// 获取 DeviceInfo 原生模块
// 在新的系统中，模块会自动初始化并注册到 NativeModules 中
let DeviceInfoNative = null;

// 延迟获取原生模块，确保模块系统已初始化
function getDeviceInfoNative() {
  if (!DeviceInfoNative) {
    DeviceInfoNative = NativeModules.get('DeviceInfo');

    if (!DeviceInfoNative) {
      console.error('[DeviceInfo] Native module not found. Available modules:',
                   Object.keys(NativeModules.getAll()));
      throw new Error('DeviceInfo native module is not available');
    }

    console.log('[DeviceInfo] Native module loaded successfully');
  }

  return DeviceInfoNative;
}

/**
 * DeviceInfo 模块导出接口
 * 提供与 React Native DeviceInfo 兼容的 API
 */
const DeviceInfo = {
  /**
   * 获取设备唯一标识符
   * @returns {Promise<string>} 设备唯一ID的Promise
   */
  getUniqueId() {
    console.log('[DeviceInfo] Calling getUniqueId (Promise method)');

    const native = getDeviceInfoNative();

    // 由于使用了标准的 genMethod 实现，这个方法会自动返回 Promise
    return native.getUniqueId();
  },

  /**
   * 获取系统版本
   * @returns {string} 系统版本字符串
   */
  getSystemVersion() {
    console.log('[DeviceInfo] Calling getSystemVersion (Sync method)');

    const native = getDeviceInfoNative();

    // 由于使用了标准的 genMethod 实现，这个方法会同步返回结果
    return native.getSystemVersion();
  },

  /**
   * 获取设备型号
   * @param {Function} callback - 回调函数，接收 (error, result) 参数
   */
  getModel(callback) {
    console.log('[DeviceInfo] Calling getModel (Callback method)');

    if (!callback || typeof callback !== 'function') {
      console.error('[DeviceInfo] getModel requires a callback function');
      return;
    }

    const native = getDeviceInfoNative();

    // 由于使用了标准的 genMethod 实现，这个方法支持双回调模式
    // 但我们的实现是单回调，所以只传成功回调
    native.getModel(callback);
  },

  /**
   * 获取系统名称
   * @param {Function} callback - 回调函数，接收 (error, result) 参数
   */
  getSystemName(callback) {
    console.log('[DeviceInfo] Calling getSystemName (Callback method)');

    if (!callback || typeof callback !== 'function') {
      console.error('[DeviceInfo] getSystemName requires a callback function');
      return;
    }

    const native = getDeviceInfoNative();

    // 由于使用了标准的 genMethod 实现，这个方法支持双回调模式
    // 但我们的实现是单回调，所以只传成功回调
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

// 兼容性：支持类似 React Native 的使用方式
Object.defineProperty(DeviceInfo, 'Constants', {
  get() {
    const native = getDeviceInfoNative();
    // 返回原生模块的常量（如果有的话）
    return native.Constants || {};
  }
});

// 导出模块
if (typeof module !== 'undefined' && module.exports) {
  module.exports = DeviceInfo;
} else if (typeof global !== 'undefined') {
  global.DeviceInfo = DeviceInfo;
}

// 兼容性：支持 window 环境
if (typeof window !== 'undefined') {
  window.DeviceInfo = DeviceInfo;
}
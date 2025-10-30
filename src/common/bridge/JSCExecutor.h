#ifndef JSCEXECUTOR_H
#define JSCEXECUTOR_H

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "../modules/ModuleRegistry.h"

// 跨平台 JavaScript 引擎支持
#ifdef __APPLE__
  // Apple 平台：使用系统内置的 JavaScriptCore
  #include <JavaScriptCore/JavaScriptCore.h>
#elif __ANDROID__
  // Android 平台：使用移植的 JavaScriptCore
  #include <jsc/jsc.h>
#else
  #error "Unsupported platform"
#endif

namespace mini_rn {
namespace bridge {

/**
 * Bridge 消息队列数据结构
 * 严格遵循 React Native 的消息格式：[moduleIds, methodIds, params, callbackIds]
 */
struct BridgeMessage {
  std::vector<int> moduleIds;       // 模块ID数组
  std::vector<int> methodIds;       // 方法ID数组
  std::vector<std::string> params;  // 参数数组（JSON字符串格式）
  std::vector<int> callbackIds;     // 回调ID数组

  // 获取调用数量
  size_t getCallCount() const { return moduleIds.size(); }

  // 验证消息格式是否正确
  bool isValid() const {
    return moduleIds.size() == methodIds.size() &&
           methodIds.size() == params.size() &&
           params.size() == callbackIds.size();
  }
};

/**
 * 回调信息结构
 */
struct CallbackInfo {
  int callbackId;
  // 为后续扩展预留字段
  // std::function<void(const std::string&)> callback;
};

/**
 * 模块调用信息结构
 */
struct ModuleCall {
  int moduleId;
  int methodId;
  std::string params;  // JSON格式参数
  int callbackId;
};

/**
 * JSCExecutor - JavaScript 执行器
 *
 * 这是 React Native Bridge 架构的核心组件，负责：
 * 1. JavaScript 环境管理 - 创建和维护 JavaScript 执行上下文
 * 2. 代码执行 - 执行 JavaScript 代码（应用代码、框架代码）
 * 3. Bridge 函数注入 - 向 JavaScript 环境注入 Native 通信函数
 * 4. 异常处理 - 捕获和处理 JavaScript 执行错误
 *
 * 设计原则：
 * - 严格遵循 React Native JSCExecutor 的接口设计
 * - 保持与 RN 的架构思路一致，实现细节可以简化
 * - 专注核心功能，暂时忽略复杂的优化
 */
class JSCExecutor {
 private:
  /**
   * JSXXX 都是 JavaScriptCore 的 API，一些是类型（如
   * JSObjectRef）一些是方法（如 JSGlobalContextCreate） 通过使用
   * JSXXX，可以用 C++ 调用/管理 JS 引擎，可以理解为跨语言的桥梁和安全句柄
   * JavaScriptCore API Reference:
   * https://developer.apple.com/documentation/javascriptcore
   */

  // JavaScript 运行环境（全局上下文）
  JSGlobalContextRef m_context;
  // JS 运行环境的全局对象 global
  JSObjectRef m_globalObject;
  // 异常处理回调函数
  std::function<void(const std::string &)> m_exceptionHandler;
  // Native 模块注册器
  std::unique_ptr<mini_rn::modules::ModuleRegistry> m_moduleRegistry;

 public:
  JSCExecutor();
  ~JSCExecutor();

  // 禁用拷贝构造和赋值
  JSCExecutor(const JSCExecutor &) = delete;
  JSCExecutor &operator=(const JSCExecutor &) = delete;

  /**
   * 加载并执行 JavaScript 应用代码
   * @param script JavaScript 代码内容
   * @param sourceURL 代码来源 URL（用于调试）
   */
  void loadApplicationScript(const std::string &script,
                             const std::string &sourceURL = "");

  /**
   * 设置 JavaScript 异常处理器
   * @param handler 异常处理回调函数
   */
  void setJSExceptionHandler(std::function<void(const std::string &)> handler);

  /**
   * 调用 JavaScript 函数
   * @param module 模块名称
   * @param method 方法名称
   * @param arguments JSON 格式的参数
   */
  void callJSFunction(const std::string &module, const std::string &method,
                      const std::string &arguments);

  /**
   * 向 JavaScript 环境注入全局函数
   * @param name 函数名称
   * @param callback Native 回调函数
   */
  void installGlobalFunction(const std::string &name,
                             JSObjectCallAsFunctionCallback callback);

  /**
   * 销毁 JavaScript 执行环境
   */
  void destroy();

  /**
   * 获取 JavaScript 上下文（用于高级操作）
   */
  JSGlobalContextRef getContext() const { return m_context; }

  /**
   * 获取模块注册器
   * 用于注册 Native 模块和管理模块调用
   * @return 模块注册器指针
   */
  mini_rn::modules::ModuleRegistry *getModuleRegistry() {
    return m_moduleRegistry.get();
  }

  /**
   * 重新注入模块配置
   * 在模块注册完成后调用，更新 JavaScript 环境中的模块配置
   */
  void refreshModuleConfig();

  /**
   * 注入模块配置到 JavaScript 环境
   * 创建 __fbBatchedBridgeConfig 全局对象，包含所有已注册模块的配置信息
   */
  void injectModuleConfig();

 private:
  /**
   * 初始化 JavaScript 执行环境
   */
  void initializeJSContext();

  /**
   * 设置 React Native 标准的全局对象
   */
  void setupGlobalObjects();

  /**
   * 设置 console 对象
   */
  void setupConsole();

  /**
   * 给 JS 注入 React Native Bridge 的核心函数
   */
  void installBridgeFunctions();

  /**
   * 加载核心 Bridge JavaScript 模块
   * 设置基础的模块加载系统和核心 Bridge 组件
   */
  void loadCoreBridgeModules();

  /**
   * 处理 JavaScript 异常
   */
  void handleJSException(JSValueRef exception);

  /**
   * JSValue 和 C++ 类型之间的转换工具
   */
  std::string jsValueToString(JSValueRef value);
  JSValueRef stringToJSValue(const std::string &str);

  /**
   * JSValue 到 JSON 字符串转换（对齐 React Native 实现）
   * 这个方法模拟 RN 中的 JSValueToJSONString 功能
   * @param value JavaScript 值（通常是数组或对象）
   * @return JSON 格式的字符串表示
   */
  std::string jsValueToJSONString(JSValueRef value);

  /**
   * Native Bridge 函数实现（对齐RN架构）
   * 这些方法对应RN中JSCExecutor的Bridge函数，从静态回调中调用
   */

  /**
   * 处理来自JavaScript的队列刷新请求
   * 对齐React Native实现：JSCExecutor::nativeFlushQueueImmediate
   * @param queue JavaScript传递的队列数组 [moduleIds, methodIds, params,
   * callbackIds]
   */
  void nativeFlushQueueImmediate(JSValueRef queue);

  /**
   * 处理来自JavaScript的日志请求
   * 对齐React Native实现：JSCExecutor::nativeLoggingHook
   * @param level 日志级别
   * @param message 日志消息
   */
  void nativeLoggingHook(JSValueRef level, JSValueRef message);

  /**
   * 处理来自JavaScript的同步调用请求
   * 对齐React Native实现：JSCExecutor::nativeCallSyncHook
   * @param moduleID 模块ID
   * @param methodID 方法ID
   * @param args 参数数组
   * @return 同步调用的结果
   */
  JSValueRef nativeCallSyncHook(JSValueRef moduleID, JSValueRef methodID, JSValueRef args);

  /**
   * 静态实例管理（用于静态回调访问实例方法）
   */
  static JSCExecutor *getCurrentInstance();

 private:
  /**
   * 静态实例指针（简化的实例管理）
   */
  static JSCExecutor *s_currentInstance;

  /**
   * 处理Bridge消息（从JSON解析后的消息）
   * @param message 解析后的Bridge消息
   */
  void processBridgeMessage(const mini_rn::bridge::BridgeMessage &message);

  /**
   * 处理模块调用回调
   * 将 Native 模块的执行结果返回给 JavaScript
   * @param callId 调用标识符
   * @param result 执行结果（JSON格式）
   * @param isError 是否为错误结果
   */
  void handleModuleCallback(int callId, const std::string &result,
                            bool isError);
};

}  // namespace bridge
}  // namespace mini_rn

#endif  // JSCEXECUTOR_H
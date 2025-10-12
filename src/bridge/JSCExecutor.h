#ifndef JSCEXECUTOR_H
#define JSCEXECUTOR_H

#include <functional>
#include <memory>
#include <string>

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
   * 处理 JavaScript 异常
   */
  void handleJSException(JSValueRef exception);

  /**
   * JSValue 和 C++ 类型之间的转换工具
   */
  std::string jsValueToString(JSValueRef value);
  JSValueRef stringToJSValue(const std::string &str);
};

}  // namespace bridge
}  // namespace mini_rn

#endif  // JSCEXECUTOR_H
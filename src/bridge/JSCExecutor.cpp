#include "JSCExecutor.h"

#include <iostream>
#include <sstream>

// 统一的 JSValue 转换工具函数
// 这个函数被静态回调函数和成员函数共同使用，避免代码重复
static std::string convertJSValueToString(JSContextRef ctx, JSValueRef value) {
  JSStringRef strRef = JSValueToStringCopy(ctx, value, nullptr);
  if (!strRef) return "";

  size_t bufferSize = JSStringGetMaximumUTF8CStringSize(strRef);
  char *buffer = new char[bufferSize];
  JSStringGetUTF8CString(strRef, buffer, bufferSize);

  std::string result(buffer);
  delete[] buffer;
  JSStringRelease(strRef);

  return result;
}


namespace mini_rn {
namespace bridge {

JSCExecutor::JSCExecutor() : m_context(nullptr), m_globalObject(nullptr) {
  initializeJSContext();
}

JSCExecutor::~JSCExecutor() { destroy(); }

void JSCExecutor::initializeJSContext() {
  // 创建 JavaScript 执行上下文
  m_context = JSGlobalContextCreate(nullptr);
  if (!m_context) {
    throw std::runtime_error("Failed to create JavaScript context");
  }

  // 获取全局对象
  m_globalObject = JSContextGetGlobalObject(m_context);

  // 设置标准的全局对象
  setupGlobalObjects();

  // 注入 Bridge 通信函数
  installBridgeFunctions();

  std::cout << "[JSCExecutor] JavaScript context initialized successfully"
            << std::endl;
}

void JSCExecutor::setupGlobalObjects() {
  // 设置 global 对象（React Native 标准）
  JSStringRef globalName = JSStringCreateWithUTF8CString("global");
  JSObjectSetProperty(m_context, m_globalObject, globalName, m_globalObject,
                      kJSPropertyAttributeNone, nullptr);
  JSStringRelease(globalName);

  // 设置 __DEV__ 标志（开发模式标识）
  JSStringRef devName = JSStringCreateWithUTF8CString("__DEV__");
  // TODO: 生产模式需要设置为 false
  JSValueRef devValue = JSValueMakeBoolean(m_context, true);
  JSObjectSetProperty(m_context, m_globalObject, devName, devValue,
                      kJSPropertyAttributeReadOnly, nullptr);
  JSStringRelease(devName);

  // JS 引擎是没有 console 对象的，需要宿主环境如浏览器或 Node.js 提供
  // 设置基础的 console 对象
  setupConsole();
}

void JSCExecutor::setupConsole() {
  // 创建 console 对象
  // JSObjectMake Creates a JavaScript object.
  JSObjectRef consoleObj = JSObjectMake(m_context, nullptr, nullptr);

  // 添加 console.log 函数
  JSStringRef logName = JSStringCreateWithUTF8CString("log");
  JSObjectRef logFunc = JSObjectMakeFunctionWithCallback(
      m_context, logName,
      [](JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject,
         size_t argumentCount, const JSValueRef arguments[],
         JSValueRef *exception) -> JSValueRef {
        // 避免未使用参数的警告
        (void)function;
        (void)thisObject;

        std::ostringstream oss;
        oss << "[JS LOG] ";

        for (size_t i = 0; i < argumentCount; ++i) {
          if (i > 0) oss << " ";
          oss << convertJSValueToString(ctx, arguments[i]);
        }

        std::cout << oss.str() << std::endl;
        return JSValueMakeUndefined(ctx);
      });

  JSObjectSetProperty(m_context, consoleObj, logName, logFunc,
                      kJSPropertyAttributeNone, nullptr);
  JSStringRelease(logName);

  // 将 console 对象添加到全局
  JSStringRef consoleName = JSStringCreateWithUTF8CString("console");
  JSObjectSetProperty(m_context, m_globalObject, consoleName, consoleObj,
                      kJSPropertyAttributeNone, nullptr);
  JSStringRelease(consoleName);
}

void JSCExecutor::installBridgeFunctions() {
  // 注入关键的 Bridge 通信函数
  // 这个函数是 React Native MessageQueue 调用 Native 的核心接口
  installGlobalFunction(
      "nativeFlushQueueImmediate",
      [](JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject,
         size_t argumentCount, const JSValueRef arguments[],
         JSValueRef *exception) -> JSValueRef {
        // 避免未使用参数的警告
        (void)function;
        (void)thisObject;
        (void)arguments;
        (void)exception;

        std::cout << "[Bridge] nativeFlushQueueImmediate called with "
                  << argumentCount << " arguments" << std::endl;

        // TODO: 在下一步实现完整的消息队列处理
        // 这里先简单打印，后续会实现完整的模块调用

        return JSValueMakeUndefined(ctx);
      });

  // 注入日志函数
  installGlobalFunction(
      "__nativeLoggingHook",
      [](JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject,
         size_t argumentCount, const JSValueRef arguments[],
         JSValueRef *exception) -> JSValueRef {
        // 避免未使用参数的警告
        (void)function;
        (void)thisObject;
        (void)exception;

        if (argumentCount >= 2) {
          /**
           * JS 侧调用示例：
           * __nativeLoggingHook('INFO', 'This is a test log from JS');
           */
          std::string level = convertJSValueToString(ctx, arguments[0]);
          std::string message = convertJSValueToString(ctx, arguments[1]);
          std::cout << "[" << level << "] " << message << std::endl;
        }

        return JSValueMakeUndefined(ctx);
      });
}

void JSCExecutor::loadApplicationScript(const std::string &script,
                                        const std::string &sourceURL) {
  JSStringRef scriptStr = JSStringCreateWithUTF8CString(script.c_str());
  JSStringRef sourceURLStr =
      sourceURL.empty() ? nullptr
                        : JSStringCreateWithUTF8CString(sourceURL.c_str());

  JSValueRef exception = nullptr;
  JSValueRef result = JSEvaluateScript(m_context, scriptStr, nullptr,
                                       sourceURLStr, 0, &exception);

  if (exception) {
    handleJSException(exception);
  } else {
    // 避免未使用变量的警告
    (void)result;
    std::cout << "[JSCExecutor] Script executed successfully" << std::endl;
  }

  JSStringRelease(scriptStr);
  if (sourceURLStr) JSStringRelease(sourceURLStr);
}

void JSCExecutor::setJSExceptionHandler(
    std::function<void(const std::string &)> handler) {
  m_exceptionHandler = handler;
}

void JSCExecutor::callJSFunction(const std::string &module,
                                 const std::string &method,
                                 const std::string &arguments) {
  // TODO: 待实现 JavaScript 函数调用
  // 避免未使用参数的警告
  (void)arguments;

  std::cout << "[JSCExecutor] Calling JS function: " << module << "." << method
            << std::endl;
}

void JSCExecutor::installGlobalFunction(
    const std::string &name,
    // https://developer.apple.com/documentation/javascriptcore/jsobjectcallasfunctioncallback/
    JSObjectCallAsFunctionCallback callback) {
  JSStringRef funcName = JSStringCreateWithUTF8CString(name.c_str());
  JSObjectRef func =
      JSObjectMakeFunctionWithCallback(m_context, funcName, callback);

  JSObjectSetProperty(m_context, m_globalObject, funcName, func,
                      kJSPropertyAttributeNone, nullptr);

  JSStringRelease(funcName);

  std::cout << "[JSCExecutor] Installed global function: " << name << std::endl;
}

void JSCExecutor::destroy() {
  if (m_context) {
    JSGlobalContextRelease(m_context);
    m_context = nullptr;
    m_globalObject = nullptr;
    std::cout << "[JSCExecutor] JavaScript context destroyed" << std::endl;
  }
}

void JSCExecutor::handleJSException(JSValueRef exception) {
  std::string errorMsg = jsValueToString(exception);
  std::cout << "[JSCExecutor] JavaScript Exception: " << errorMsg << std::endl;

  if (m_exceptionHandler) {
    m_exceptionHandler(errorMsg);
  }
}

std::string JSCExecutor::jsValueToString(JSValueRef value) {
  // 转换过程:
  // JSValueRef (JS世界) -> JSStringRef (传输格式) -> string (C++ 世界)
  // 使用统一的转换函数避免代码重复
  return convertJSValueToString(m_context, value);
}

JSValueRef JSCExecutor::stringToJSValue(const std::string &str) {
  // 转换过程：
  // string (C++ 世界) -> JSStringRef (传输格式) -> JSValueRef (JS世界)
  JSStringRef strRef = JSStringCreateWithUTF8CString(str.c_str());
  JSValueRef value = JSValueMakeString(m_context, strRef);
  JSStringRelease(strRef);
  return value;
}

}  // namespace bridge
}  // namespace mini_rn
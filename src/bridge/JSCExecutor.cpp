#include "JSCExecutor.h"

#include <iostream>
#include <sstream>

#include "../utils/JSONParser.h"

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

// 静态实例指针定义
JSCExecutor *JSCExecutor::s_currentInstance = nullptr;

JSCExecutor::JSCExecutor() : m_context(nullptr), m_globalObject(nullptr) {
  // 注册当前实例（简化的实例管理）
  s_currentInstance = this;

  // 初始化模块注册器
  m_moduleRegistry = std::make_unique<mini_rn::modules::ModuleRegistry>();

  // 设置模块回调处理器
  m_moduleRegistry->setCallbackHandler(
      [this](int callId, const std::string &result, bool isError) {
        this->handleModuleCallback(callId, result, isError);
      });

  initializeJSContext();
}

JSCExecutor::~JSCExecutor() {
  // 清理实例指针
  if (s_currentInstance == this) {
    s_currentInstance = nullptr;
  }
  destroy();
}

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
  // 完全对齐 RN 实现：nativeFlushQueueImmediate(queue)，其中 queue =
  // [moduleIds, methodIds, params, callbackIds]
  installGlobalFunction(
      "nativeFlushQueueImmediate",
      [](JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject,
         size_t argumentCount, const JSValueRef arguments[],
         JSValueRef *exception) -> JSValueRef {
        // 避免未使用参数的警告
        (void)function;
        (void)thisObject;
        (void)exception;
        (void)ctx;

        std::cout << "[Bridge] nativeFlushQueueImmediate called with "
                  << argumentCount
                  << " arguments (RN-compatible single parameter)" << std::endl;

        try {
          // 获取JSCExecutor实例（对齐RN架构）
          auto *executor = JSCExecutor::getCurrentInstance();
          if (!executor) {
            std::cout << "[Bridge] Error: No JSCExecutor instance available"
                      << std::endl;
            return JSValueMakeUndefined(ctx);
          }

          // 验证参数数量（对齐RN：单个queue参数）
          if (argumentCount != 1) {
            std::cout
                << "[Bridge] Error: Expected 1 argument (queue array), got "
                << argumentCount << std::endl;
            return JSValueMakeUndefined(ctx);
          }

          // 调用实例方法（对齐RN架构）
          executor->nativeFlushQueueImmediate(arguments[0]);

        } catch (const std::exception &e) {
          std::cout << "[Bridge] Exception in static callback: " << e.what()
                    << std::endl;
        } catch (...) {
          std::cout << "[Bridge] Unknown exception in static callback"
                    << std::endl;
        }

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
        (void)ctx;

        try {
          // 获取JSCExecutor实例（对齐RN架构）
          auto *executor = JSCExecutor::getCurrentInstance();
          if (!executor) {
            std::cout << "[Bridge] Error: No JSCExecutor instance available "
                         "for logging"
                      << std::endl;
            return JSValueMakeUndefined(ctx);
          }

          if (argumentCount >= 2) {
            // 调用实例方法（对齐RN架构）
            executor->nativeLoggingHook(arguments[0], arguments[1]);
          } else {
            std::cout << "[Bridge] Warning: nativeLoggingHook called with "
                         "insufficient arguments"
                      << std::endl;
          }

        } catch (const std::exception &e) {
          std::cout << "[Bridge] Exception in logging callback: " << e.what()
                    << std::endl;
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

std::string JSCExecutor::jsValueToJSONString(JSValueRef value) {
  // 这个方法对齐 React Native 的 JSValueToJSONString 实现
  // 使用 JavaScript 的 JSON.stringify 功能来序列化 JSValue

  try {
    // 获取全局 JSON 对象
    JSStringRef jsonName = JSStringCreateWithUTF8CString("JSON");
    JSValueRef jsonObject =
        JSObjectGetProperty(m_context, m_globalObject, jsonName, nullptr);
    JSStringRelease(jsonName);

    if (!JSValueIsObject(m_context, jsonObject)) {
      throw std::runtime_error(
          "JSON object not available in JavaScript context");
    }

    // 获取 JSON.stringify 方法
    JSObjectRef jsonObj = JSValueToObject(m_context, jsonObject, nullptr);
    JSStringRef stringifyName = JSStringCreateWithUTF8CString("stringify");
    JSValueRef stringifyMethod =
        JSObjectGetProperty(m_context, jsonObj, stringifyName, nullptr);
    JSStringRelease(stringifyName);

    if (!JSValueIsObject(m_context, stringifyMethod)) {
      throw std::runtime_error("JSON.stringify is not a function");
    }

    // 调用 JSON.stringify(value)
    JSValueRef arguments[] = {value};
    JSValueRef exception = nullptr;
    JSValueRef result =
        JSObjectCallAsFunction(m_context, (JSObjectRef)stringifyMethod,
                               nullptr,  // thisObject
                               1,        // argumentCount
                               arguments, &exception);

    if (exception) {
      handleJSException(exception);
      throw std::runtime_error("JSON.stringify failed with exception");
    }

    if (JSValueIsNull(m_context, result) ||
        JSValueIsUndefined(m_context, result)) {
      throw std::runtime_error("JSON.stringify returned null or undefined");
    }

    // 将结果转换为 C++ 字符串
    std::string jsonString = jsValueToString(result);

    std::cout << "[JSCExecutor] JSValue -> JSON conversion successful, length: "
              << jsonString.length() << std::endl;

    return jsonString;

  } catch (const std::exception &e) {
    std::cout << "[JSCExecutor] Error in jsValueToJSONString: " << e.what()
              << std::endl;

    // 降级处理：如果 JSON.stringify 失败，尝试简单的字符串转换
    std::cout << "[JSCExecutor] Falling back to simple string conversion"
              << std::endl;
    return jsValueToString(value);
  }
}

// === Bridge 成员方法实现（对齐RN架构）===

JSCExecutor *JSCExecutor::getCurrentInstance() { return s_currentInstance; }

void JSCExecutor::nativeFlushQueueImmediate(JSValueRef queue) {
  std::cout
      << "[JSCExecutor] nativeFlushQueueImmediate called (instance method)"
      << std::endl;

  try {
    // Step 1: JSValue -> JSON字符串 (对齐RN: queue.toJSONString())
    std::string queueStr = jsValueToJSONString(queue);
    std::cout << "[JSCExecutor] JSON serialization successful, length: "
              << queueStr.length() << std::endl;

    // Step 2: JSON字符串 -> BridgeMessage (替代 folly::parseJson)
    std::cout << "[JSCExecutor] Parsing JSON with SimpleBridgeJSONParser..."
              << std::endl;
    mini_rn::bridge::BridgeMessage message =
        mini_rn::utils::SimpleBridgeJSONParser::parseBridgeQueue(queueStr);

    // Step 3: 处理消息 (替代 m_delegate->callNativeModules)
    std::cout << "[JSCExecutor] Processing parsed message..." << std::endl;
    processBridgeMessage(message);

  } catch (const std::exception &e) {
    std::cout << "[JSCExecutor] Error in nativeFlushQueueImmediate: "
              << e.what() << std::endl;
  }
}

void JSCExecutor::nativeLoggingHook(JSValueRef level, JSValueRef message) {
  std::cout << "[JSCExecutor] nativeLoggingHook called (instance method)"
            << std::endl;

  try {
    std::string levelStr = jsValueToString(level);
    std::string messageStr = jsValueToString(message);
    std::cout << "[" << levelStr << "] " << messageStr << std::endl;
  } catch (const std::exception &e) {
    std::cout << "[JSCExecutor] Error in nativeLoggingHook: " << e.what()
              << std::endl;
  }
}

void JSCExecutor::processBridgeMessage(
    const mini_rn::bridge::BridgeMessage &message) {
  std::cout << "[JSCExecutor] Processing Bridge message with "
            << message.getCallCount() << " calls" << std::endl;

  // 验证消息格式
  if (!message.isValid()) {
    std::cout << "[JSCExecutor] Error: Invalid bridge message format"
              << std::endl;
    return;
  }

  // 处理每个模块调用
  for (size_t i = 0; i < message.getCallCount(); i++) {
    unsigned int moduleId = static_cast<unsigned int>(message.moduleIds[i]);
    unsigned int methodId = static_cast<unsigned int>(message.methodIds[i]);
    const std::string &params = message.params[i];
    int callId = message.callbackIds[i];

    std::cout << "[JSCExecutor] Call " << (i + 1) << "/"
              << message.getCallCount() << ": Module=" << moduleId
              << ", Method=" << methodId << ", Params=" << params
              << ", CallId=" << callId << std::endl;

    // 通过 ModuleRegistry 调用 Native 模块方法
    if (m_moduleRegistry) {
      m_moduleRegistry->callNativeMethod(moduleId, methodId, params, callId);
    } else {
      std::cout << "[JSCExecutor] Error: ModuleRegistry not initialized"
                << std::endl;
    }
  }

  std::cout << "[JSCExecutor] Bridge message processing completed" << std::endl;
}

void JSCExecutor::handleModuleCallback(int callId, const std::string &result,
                                       bool isError) {
  std::cout << "[JSCExecutor] Handling module callback - CallId: " << callId
            << ", IsError: " << (isError ? "true" : "false")
            << ", Result: " << result << std::endl;

  try {
    // TODO: 实现将结果返回给 JavaScript 的机制
    // 这需要调用 JavaScript 的回调函数，类似于：
    // - 如果是成功回调：callJSFunction("RCTDeviceEventEmitter", "emit",
    // [callId, result])
    // - 如果是错误回调：callJSFunction("RCTDeviceEventEmitter", "emit",
    // [callId, null, error])

    // 目前暂时只打印日志，在任务 3.2 实现具体模块时会完善这个机制
    if (isError) {
      std::cout << "[JSCExecutor] Module call failed: " << result << std::endl;
    } else {
      std::cout << "[JSCExecutor] Module call succeeded: " << result
                << std::endl;
    }

  } catch (const std::exception &e) {
    std::cout << "[JSCExecutor] Error in handleModuleCallback: " << e.what()
              << std::endl;
  }
}

}  // namespace bridge
}  // namespace mini_rn
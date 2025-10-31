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

  // 注意：模块配置注入延迟到模块注册后
  // injectModuleConfig() 将在 ModuleRegistry::registerModules() 后调用

  // 加载核心 Bridge JavaScript 文件
  loadCoreBridgeModules();

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

  // 注入同步调用函数 (React Native 标准)
  installGlobalFunction(
      "nativeCallSyncHook",
      [](JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject,
         size_t argumentCount, const JSValueRef arguments[],
         JSValueRef *exception) -> JSValueRef {
        // 避免未使用参数的警告
        (void)function;
        (void)thisObject;
        (void)exception;

        std::cout << "[Bridge] nativeCallSyncHook called with "
                  << argumentCount << " arguments" << std::endl;

        try {
          // 获取JSCExecutor实例
          auto *executor = JSCExecutor::getCurrentInstance();
          if (!executor) {
            std::cout << "[Bridge] Error: No JSCExecutor instance available"
                      << std::endl;
            return JSValueMakeUndefined(ctx);
          }

          // 验证参数数量：moduleID, methodID, args
          if (argumentCount != 3) {
            std::cout << "[Bridge] Error: Expected 3 arguments (moduleID, methodID, args), got "
                      << argumentCount << std::endl;
            return JSValueMakeUndefined(ctx);
          }

          // 调用实例方法处理同步调用
          return executor->nativeCallSyncHook(arguments[0], arguments[1], arguments[2]);

        } catch (const std::exception &e) {
          std::cout << "[Bridge] Exception in sync callback: " << e.what()
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

  // 尝试提取堆栈跟踪信息
  try {
    // 检查异常是否为对象（Error 对象）
    if (JSValueIsObject(m_context, exception)) {
      JSObjectRef errorObj = JSValueToObject(m_context, exception, nullptr);

      // 尝试获取 stack 属性
      JSStringRef stackPropName = JSStringCreateWithUTF8CString("stack");
      JSValueRef stackValue = JSObjectGetProperty(m_context, errorObj, stackPropName, nullptr);
      JSStringRelease(stackPropName);

      if (stackValue != nullptr && !JSValueIsUndefined(m_context, stackValue) && !JSValueIsNull(m_context, stackValue)) {
        std::string stackTrace = jsValueToString(stackValue);
        if (!stackTrace.empty()) {
          std::cout << "Stack Trace:" << std::endl;
          std::cout << stackTrace << std::endl;

          // 如果有异常处理器，将堆栈信息也包含在内
          if (m_exceptionHandler) {
            std::string fullErrorMsg = errorMsg + "\nStack Trace:\n" + stackTrace;
            m_exceptionHandler(fullErrorMsg);
            return;
          }
        }
      } else {
        std::cout << "[JSCExecutor] No stack trace available for this exception" << std::endl;
      }
    } else {
      std::cout << "[JSCExecutor] Exception is not an Error object, no stack trace available" << std::endl;
    }
  } catch (const std::exception& e) {
    std::cout << "[JSCExecutor] Error while extracting stack trace: " << e.what() << std::endl;
  }

  // 调用原有的异常处理器（如果没有堆栈信息的话）
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

JSValueRef JSCExecutor::nativeCallSyncHook(JSValueRef moduleID, JSValueRef methodID, JSValueRef args) {
  std::cout << "[JSCExecutor] nativeCallSyncHook called (instance method)" << std::endl;

  try {
    // 将JSValue参数转换为C++类型
    double moduleIdDouble = JSValueToNumber(m_context, moduleID, nullptr);
    double methodIdDouble = JSValueToNumber(m_context, methodID, nullptr);
    unsigned int moduleIdInt = static_cast<unsigned int>(moduleIdDouble);
    unsigned int methodIdInt = static_cast<unsigned int>(methodIdDouble);

    std::cout << "[JSCExecutor] Sync call - Module: " << moduleIdInt
              << ", Method: " << methodIdInt << std::endl;

    // 将参数转换为JSON字符串
    std::string argsJson = jsValueToJSONString(args);

    // 检查模块注册器是否可用
    if (!m_moduleRegistry) {
      std::cout << "[JSCExecutor] Error: ModuleRegistry not available for sync call" << std::endl;
      return JSValueMakeNull(m_context);
    }

    // 对于同步调用，我们需要立即获取结果
    // 这里我们使用一个简化的实现：直接调用模块方法并等待结果

    // 检查模块是否存在
    if (!m_moduleRegistry->hasModule(moduleIdInt)) {
      std::cout << "[JSCExecutor] Error: Module " << moduleIdInt << " not found for sync call" << std::endl;
      return JSValueMakeNull(m_context);
    }

    // 获取模块名称用于调试
    std::string moduleName = m_moduleRegistry->getModuleName(moduleIdInt);
    std::cout << "[JSCExecutor] Sync call to module: " << moduleName << std::endl;

    // 对于同步方法，我们需要特殊处理
    // 在这个简化实现中，我们只支持DeviceInfo.getSystemVersion的同步调用
    if (moduleName == "DeviceInfo" && methodIdInt == 1) {
      // getSystemVersion 方法的同步实现
      std::string result = "14.0";  // 模拟系统版本
      std::cout << "[JSCExecutor] Sync method returned: " << result << std::endl;
      return stringToJSValue(result);
    }

    // 对于其他方法，返回错误
    std::cout << "[JSCExecutor] Warning: Sync call not supported for "
              << moduleName << "." << methodIdInt << std::endl;
    return JSValueMakeNull(m_context);

  } catch (const std::exception &e) {
    std::cout << "[JSCExecutor] Error in nativeCallSyncHook: " << e.what() << std::endl;
    return JSValueMakeNull(m_context);
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
    // 实现将结果返回给 JavaScript 的机制
    // 调用 JavaScript 的 invokeCallbackAndReturnFlushedQueue 方法

    // 获取全局 __fbBatchedBridge 对象
    JSStringRef bridgeName = JSStringCreateWithUTF8CString("__fbBatchedBridge");
    JSValueRef bridgeValue = JSObjectGetProperty(m_context, m_globalObject, bridgeName, nullptr);
    JSStringRelease(bridgeName);

    if (!JSValueIsObject(m_context, bridgeValue)) {
      std::cout << "[JSCExecutor] Warning: __fbBatchedBridge not available" << std::endl;
      return;
    }

    JSObjectRef bridgeObject = JSValueToObject(m_context, bridgeValue, nullptr);

    // 获取 invokeCallbackAndReturnFlushedQueue 方法
    JSStringRef methodName = JSStringCreateWithUTF8CString("invokeCallbackAndReturnFlushedQueue");
    JSValueRef methodValue = JSObjectGetProperty(m_context, bridgeObject, methodName, nullptr);
    JSStringRelease(methodName);

    if (!JSValueIsObject(m_context, methodValue)) {
      std::cout << "[JSCExecutor] Warning: invokeCallbackAndReturnFlushedQueue not available" << std::endl;
      return;
    }

    // 准备回调参数
    // React Native 回调约定：第一个参数是错误，后续参数是结果
    std::string argsJson;
    if (isError) {
      // 错误情况：[error]
      argsJson = "[\"" + result + "\"]";
    } else {
      // 成功情况：[null, result]
      argsJson = "[null, \"" + result + "\"]";
    }

    // 创建参数数组
    JSValueRef arguments[2];
    arguments[0] = JSValueMakeNumber(m_context, callId);  // callbackID

    // 解析 JSON 参数
    JSStringRef argsStr = JSStringCreateWithUTF8CString(argsJson.c_str());
    JSValueRef argsValue = JSValueMakeFromJSONString(m_context, argsStr);
    JSStringRelease(argsStr);

    if (!argsValue) {
      // JSON 解析失败，创建简单数组
      JSValueRef simpleArgs[1];
      simpleArgs[0] = stringToJSValue(result);
      argsValue = JSObjectMakeArray(m_context, 1, simpleArgs, nullptr);
    }

    arguments[1] = argsValue;  // args

    // 调用 JavaScript 回调方法
    JSValueRef exception = nullptr;
    JSValueRef callResult = JSObjectCallAsFunction(
        m_context,
        (JSObjectRef)methodValue,
        bridgeObject,  // thisObject
        2,             // argumentCount
        arguments,     // arguments
        &exception
    );

    if (exception) {
      handleJSException(exception);
      std::cout << "[JSCExecutor] Error calling JavaScript callback" << std::endl;
    } else {
      // 避免未使用变量的警告
      (void)callResult;
      std::cout << "[JSCExecutor] JavaScript callback executed successfully" << std::endl;
    }

  } catch (const std::exception &e) {
    std::cout << "[JSCExecutor] Error in handleModuleCallback: " << e.what()
              << std::endl;
  }
}

void JSCExecutor::injectModuleConfig() {
  std::cout << "[JSCExecutor] Injecting module configuration..." << std::endl;

  try {
    // 获取所有注册的模块配置
    if (!m_moduleRegistry) {
      std::cout << "[JSCExecutor] Warning: ModuleRegistry not available" << std::endl;
      return;
    }

    // 创建 __fbBatchedBridgeConfig 对象
    // 格式：{ remoteModuleConfig: [[moduleName, constants, methods, promiseMethods, syncMethods], ...] }

    JSObjectRef bridgeConfig = JSObjectMake(m_context, nullptr, nullptr);

    // 创建 remoteModuleConfig 数组
    auto moduleNames = m_moduleRegistry->moduleNames();
    size_t moduleCount = moduleNames.size();

    if (moduleCount == 0) {
      std::cout << "[JSCExecutor] No modules registered, creating empty config" << std::endl;

      // 创建空的 remoteModuleConfig 数组
      JSValueRef emptyArray = JSObjectMakeArray(m_context, 0, nullptr, nullptr);

      // 设置 remoteModuleConfig 属性
      JSStringRef remoteModuleConfigKey = JSStringCreateWithUTF8CString("remoteModuleConfig");
      JSObjectSetProperty(m_context, bridgeConfig, remoteModuleConfigKey, emptyArray, kJSPropertyAttributeNone, nullptr);
      JSStringRelease(remoteModuleConfigKey);
    } else {
      // 创建模块配置数组
      std::vector<JSValueRef> moduleConfigs;
      moduleConfigs.reserve(moduleCount);

      for (size_t moduleId = 0; moduleId < moduleCount; moduleId++) {
        const std::string& moduleName = moduleNames[moduleId];
        auto methodNames = m_moduleRegistry->getMethodNames(moduleId);

        // 创建单个模块配置数组：[moduleName, constants, methods, promiseMethods, syncMethods]
        std::vector<JSValueRef> moduleConfigElements;
        moduleConfigElements.reserve(5);

        // 1. 模块名称
        JSStringRef moduleNameStr = JSStringCreateWithUTF8CString(moduleName.c_str());
        moduleConfigElements.push_back(JSValueMakeString(m_context, moduleNameStr));
        JSStringRelease(moduleNameStr);

        // 2. 常量 (目前设为 null)
        moduleConfigElements.push_back(JSValueMakeNull(m_context));

        // 3. 方法名数组
        std::vector<JSValueRef> methodNameValues;
        methodNameValues.reserve(methodNames.size());
        for (const auto& methodName : methodNames) {
          JSStringRef methodNameStr = JSStringCreateWithUTF8CString(methodName.c_str());
          methodNameValues.push_back(JSValueMakeString(m_context, methodNameStr));
          JSStringRelease(methodNameStr);
        }
        JSValueRef methodsArray = JSObjectMakeArray(m_context, methodNameValues.size(),
                                                   methodNameValues.empty() ? nullptr : methodNameValues.data(), nullptr);
        moduleConfigElements.push_back(methodsArray);

        // 4. Promise 方法ID数组
        std::vector<JSValueRef> promiseMethodIds;
        if (moduleName == "DeviceInfo") {
          promiseMethodIds.push_back(JSValueMakeNumber(m_context, 0)); // getUniqueId
        }
        JSValueRef promiseMethodsArray = JSObjectMakeArray(m_context, promiseMethodIds.size(),
                                                          promiseMethodIds.empty() ? nullptr : promiseMethodIds.data(), nullptr);
        moduleConfigElements.push_back(promiseMethodsArray);

        // 5. 同步方法ID数组
        std::vector<JSValueRef> syncMethodIds;
        if (moduleName == "DeviceInfo") {
          syncMethodIds.push_back(JSValueMakeNumber(m_context, 1)); // getSystemVersion
        }
        JSValueRef syncMethodsArray = JSObjectMakeArray(m_context, syncMethodIds.size(),
                                                       syncMethodIds.empty() ? nullptr : syncMethodIds.data(), nullptr);
        moduleConfigElements.push_back(syncMethodsArray);

        // 创建模块配置数组
        JSValueRef moduleConfigArray = JSObjectMakeArray(m_context, moduleConfigElements.size(),
                                                         moduleConfigElements.data(), nullptr);
        moduleConfigs.push_back(moduleConfigArray);

        std::cout << "[JSCExecutor] Created config for module: " << moduleName
                  << " with " << methodNames.size() << " methods" << std::endl;
      }

      // 创建 remoteModuleConfig 数组
      JSValueRef remoteModuleConfigArray = JSObjectMakeArray(m_context, moduleConfigs.size(),
                                                             moduleConfigs.data(), nullptr);

      // 设置 remoteModuleConfig 属性
      JSStringRef remoteModuleConfigKey = JSStringCreateWithUTF8CString("remoteModuleConfig");
      JSObjectSetProperty(m_context, bridgeConfig, remoteModuleConfigKey, remoteModuleConfigArray,
                         kJSPropertyAttributeNone, nullptr);
      JSStringRelease(remoteModuleConfigKey);
    }

    // 将 bridgeConfig 设置为 global.__fbBatchedBridgeConfig
    JSStringRef bridgeConfigKey = JSStringCreateWithUTF8CString("__fbBatchedBridgeConfig");
    JSObjectSetProperty(m_context, m_globalObject, bridgeConfigKey, bridgeConfig,
                       kJSPropertyAttributeNone, nullptr);
    JSStringRelease(bridgeConfigKey);

    std::cout << "[JSCExecutor] Module configuration injected successfully using native objects" << std::endl;

  } catch (const std::exception& e) {
    std::cout << "[JSCExecutor] Error in injectModuleConfig: " << e.what() << std::endl;
  }
}

void JSCExecutor::loadCoreBridgeModules() {
  std::cout << "[JSCExecutor] Loading core Bridge JavaScript modules..." << std::endl;

  try {
    // 简化的 require 函数实现
    // 在实际项目中，这些文件会被打包工具处理
    std::string requireFunction = R"(
      // 简化的模块加载系统
      global.modules = {};
      global.require = function(modulePath) {
        if (global.modules[modulePath]) {
          return global.modules[modulePath];
        }

        // 模拟模块缓存
        var module = { exports: {} };
        global.modules[modulePath] = module.exports;

        // 这里应该加载实际的模块代码
        // 由于我们在 C++ 环境中，暂时返回空对象
        console.log('[require] Loading module:', modulePath);

        return module.exports;
      };

      // 设置模块导出辅助
      global.module = { exports: {} };
    )";

    JSStringRef scriptStr = JSStringCreateWithUTF8CString(requireFunction.c_str());
    JSValueRef exception = nullptr;
    JSValueRef result = JSEvaluateScript(m_context, scriptStr, nullptr, nullptr, 0, &exception);

    if (exception) {
      handleJSException(exception);
      std::cout << "[JSCExecutor] Error: Failed to setup module system" << std::endl;
    } else {
      // 避免未使用变量的警告
      (void)result;
      std::cout << "[JSCExecutor] Module system setup completed" << std::endl;
    }

    JSStringRelease(scriptStr);

    // 注意：在实际的实现中，我们会在这里加载 MessageQueue.js 和 BatchedBridge.js
    // 但是由于文件系统访问的复杂性，我们在测试脚本中直接使用 require() 来加载这些模块

  } catch (const std::exception& e) {
    std::cout << "[JSCExecutor] Error in loadCoreBridgeModules: " << e.what() << std::endl;
  }
}

}  // namespace bridge
}  // namespace mini_rn
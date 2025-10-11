/**
 * JSCExecutor.cpp
 * JavaScript Core 执行器的实现
 */

#include "JSCExecutor.h"
#include "NativeModule.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <json/json.h>

namespace bridge {

// 静态实例指针
JSCExecutor* JSCExecutor::s_currentInstance = nullptr;

JSCExecutor::JSCExecutor() 
    : m_context(nullptr)
    , m_globalObject(nullptr)
    , m_nextCallbackId(1) {
    s_currentInstance = this;
    std::cout << "[JSCExecutor] 创建实例" << std::endl;
}

JSCExecutor::~JSCExecutor() {
    destroy();
    if (s_currentInstance == this) {
        s_currentInstance = nullptr;
    }
    std::cout << "[JSCExecutor] 销毁实例" << std::endl;
}

bool JSCExecutor::initialize() {
    std::cout << "[JSCExecutor] 初始化 JavaScript 引擎" << std::endl;
    
    // 创建 JavaScript 上下文
    m_context = JSGlobalContextCreate(nullptr);
    if (!m_context) {
        std::cerr << "[JSCExecutor] 创建 JS 上下文失败" << std::endl;
        return false;
    }
    
    // 获取全局对象
    m_globalObject = JSContextGetGlobalObject(m_context);
    if (!m_globalObject) {
        std::cerr << "[JSCExecutor] 获取全局对象失败" << std::endl;
        JSGlobalContextRelease(m_context);
        m_context = nullptr;
        return false;
    }
    
    // 注入 Native 函数
    injectNativeFunctions();
    
    std::cout << "[JSCExecutor] JavaScript 引擎初始化成功" << std::endl;
    return true;
}

void JSCExecutor::injectNativeFunctions() {
    std::cout << "[JSCExecutor] 注入 Native 函数到 JS 环境" << std::endl;
    
    // 创建 __nativeFlushQueuedReactWork 函数
    JSStringRef flushFuncName = JSStringCreateWithUTF8CString("__nativeFlushQueuedReactWork");
    JSObjectRef flushFunc = JSObjectMakeFunctionWithCallback(m_context, flushFuncName, nativeFlushQueuedReactWork);
    JSObjectSetProperty(m_context, m_globalObject, flushFuncName, flushFunc, kJSPropertyAttributeNone, nullptr);
    JSStringRelease(flushFuncName);
    
    // 创建 __nativeCallSyncHook 函数
    JSStringRef syncFuncName = JSStringCreateWithUTF8CString("__nativeCallSyncHook");
    JSObjectRef syncFunc = JSObjectMakeFunctionWithCallback(m_context, syncFuncName, nativeCallSyncHook);
    JSObjectSetProperty(m_context, m_globalObject, syncFuncName, syncFunc, kJSPropertyAttributeNone, nullptr);
    JSStringRelease(syncFuncName);
    
    // 创建控制台日志函数
    JSStringRef logFuncName = JSStringCreateWithUTF8CString("__nativeLoggingHook");
    JSObjectRef logFunc = JSObjectMakeFunctionWithCallback(m_context, logFuncName, nativeLoggingHook);
    JSObjectSetProperty(m_context, m_globalObject, logFuncName, logFunc, kJSPropertyAttributeNone, nullptr);
    JSStringRelease(logFuncName);
    
    // 注入基础的 console 对象
    const std::string consoleScript = R"(
        if (typeof console === 'undefined') {
            global.console = {
                log: function() {
                    var args = Array.prototype.slice.call(arguments);
                    __nativeLoggingHook('log', JSON.stringify(args));
                },
                warn: function() {
                    var args = Array.prototype.slice.call(arguments);
                    __nativeLoggingHook('warn', JSON.stringify(args));
                },
                error: function() {
                    var args = Array.prototype.slice.call(arguments);
                    __nativeLoggingHook('error', JSON.stringify(args));
                }
            };
        }
    )";
    
    executeScript(consoleScript, "console-injection");
}

JSExecuteResult JSCExecutor::executeScript(const std::string& script, const std::string& sourceURL) {
    if (!m_context) {
        return JSExecuteResult(false, "", "JavaScript 引擎未初始化");
    }
    
    std::cout << "[JSCExecutor] 执行脚本: " << sourceURL << std::endl;
    
    JSStringRef scriptStr = JSStringCreateWithUTF8CString(script.c_str());
    JSStringRef sourceStr = JSStringCreateWithUTF8CString(sourceURL.c_str());
    
    JSValueRef exception = nullptr;
    JSValueRef result = JSEvaluateScript(m_context, scriptStr, nullptr, sourceStr, 1, &exception);
    
    JSStringRelease(scriptStr);
    JSStringRelease(sourceStr);
    
    if (exception) {
        std::string error = getJSException(exception);
        logJSError(error, sourceURL);
        return JSExecuteResult(false, "", error);
    }
    
    std::string resultStr = jsValueToString(result);
    std::cout << "[JSCExecutor] 脚本执行成功: " << resultStr << std::endl;
    
    return JSExecuteResult(true, resultStr);
}

JSExecuteResult JSCExecutor::callJSFunction(const std::string& functionName, const std::string& arguments) {
    if (!m_context) {
        return JSExecuteResult(false, "", "JavaScript 引擎未初始化");
    }
    
    std::cout << "[JSCExecutor] 调用 JS 函数: " << functionName << "(" << arguments << ")" << std::endl;
    
    // 构造调用脚本
    std::ostringstream scriptStream;
    scriptStream << "if (typeof " << functionName << " === 'function') { "
                 << functionName << ".apply(null, " << arguments << "); "
                 << "} else { "
                 << "throw new Error('函数未找到: " << functionName << "'); "
                 << "}";
    
    return executeScript(scriptStream.str(), "js-function-call");
}

JSExecuteResult JSCExecutor::invokeCallback(int callbackId, const std::string& arguments) {
    std::cout << "[JSCExecutor] 调用回调: " << callbackId << "(" << arguments << ")" << std::endl;
    
    // 构造回调调用脚本
    std::ostringstream scriptStream;
    scriptStream << "if (global.__MessageQueue) { "
                 << "global.__MessageQueue.invokeCallbackAndReturnFlushedQueue("
                 << callbackId << ", " << arguments << "); "
                 << "} else { "
                 << "console.error('MessageQueue 未找到'); "
                 << "}";
    
    return executeScript(scriptStream.str(), "callback-invoke");
}

std::vector<NativeCall> JSCExecutor::flushQueue() {
    std::vector<NativeCall> calls;
    
    // 调用 JS 端的 flushedQueue 方法
    JSExecuteResult result = executeScript(
        "global.__MessageQueue ? global.__MessageQueue.flushedQueue() : null", 
        "flush-queue"
    );
    
    if (!result.success || result.result == "null") {
        return calls;
    }
    
    std::cout << "[JSCExecutor] 刷新队列结果: " << result.result << std::endl;
    
    // 解析队列数据 (简化实现)
    try {
        Json::Value root;
        Json::Reader reader;
        if (reader.parse(result.result, root) && root.isArray() && root.size() > 0) {
            Json::Value queue = root[0];
            if (queue.isArray() && queue.size() >= 3) {
                int moduleId = queue[0].asInt();
                int methodId = queue[1].asInt();
                
                std::vector<std::string> args;
                Json::Value argsArray = queue[2];
                if (argsArray.isArray()) {
                    for (const auto& arg : argsArray) {
                        args.push_back(arg.asString());
                    }
                }
                
                calls.emplace_back(moduleId, methodId, args);
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "[JSCExecutor] 解析队列数据失败: " << e.what() << std::endl;
    }
    
    return calls;
}

void JSCExecutor::registerNativeModule(int moduleId, const std::string& moduleName, 
                                       std::shared_ptr<NativeModule> module) {
    m_nativeModules[moduleId] = module;
    m_moduleNameToId[moduleName] = moduleId;
    
    std::cout << "[JSCExecutor] 注册 Native 模块: " << moduleName << " (ID: " << moduleId << ")" << std::endl;
    
    // 将模块信息注入到 JS 环境
    std::ostringstream configScript;
    configScript << "if (global.__MessageQueue) { "
                 << "global.__MessageQueue.registerModule(['"
                 << moduleName << "', {}, " 
                 << module->getMethodsJSON() << ", [], []]); "
                 << "}";
    
    executeScript(configScript.str(), "module-registration");
}

void JSCExecutor::setNativeCallCallback(NativeCallCallback callback) {
    m_nativeCallCallback = callback;
}

std::string JSCExecutor::getGlobalProperty(const std::string& propertyName) {
    if (!m_context) return "";
    
    JSStringRef propName = JSStringCreateWithUTF8CString(propertyName.c_str());
    JSValueRef value = JSObjectGetProperty(m_context, m_globalObject, propName, nullptr);
    JSStringRelease(propName);
    
    return jsValueToString(value);
}

bool JSCExecutor::setGlobalProperty(const std::string& propertyName, const std::string& value) {
    if (!m_context) return false;
    
    JSStringRef propName = JSStringCreateWithUTF8CString(propertyName.c_str());
    JSValueRef jsValue = stringToJSValue(value);
    
    JSObjectSetProperty(m_context, m_globalObject, propName, jsValue, kJSPropertyAttributeNone, nullptr);
    JSStringRelease(propName);
    
    return true;
}

void JSCExecutor::destroy() {
    if (m_context) {
        std::cout << "[JSCExecutor] 销毁 JavaScript 引擎" << std::endl;
        JSGlobalContextRelease(m_context);
        m_context = nullptr;
        m_globalObject = nullptr;
    }
    
    m_nativeModules.clear();
    m_moduleNameToId.clear();
}

std::string JSCExecutor::getDebugInfo() {
    Json::Value info;
    info["initialized"] = (m_context != nullptr);
    info["moduleCount"] = static_cast<int>(m_nativeModules.size());
    info["nextCallbackId"] = m_nextCallbackId;
    
    Json::Value modules(Json::arrayValue);
    for (const auto& pair : m_moduleNameToId) {
        Json::Value module;
        module["name"] = pair.first;
        module["id"] = pair.second;
        modules.append(module);
    }
    info["modules"] = modules;
    
    Json::StreamWriterBuilder builder;
    return Json::writeString(builder, info);
}

// 工具方法实现
JSValueRef JSCExecutor::stringToJSValue(const std::string& str) {
    JSStringRef jsStr = JSStringCreateWithUTF8CString(str.c_str());
    JSValueRef value = JSValueMakeString(m_context, jsStr);
    JSStringRelease(jsStr);
    return value;
}

std::string JSCExecutor::jsValueToString(JSValueRef value) {
    if (!value || JSValueIsNull(m_context, value) || JSValueIsUndefined(m_context, value)) {
        return "";
    }
    
    JSStringRef jsStr = JSValueToStringCopy(m_context, value, nullptr);
    if (!jsStr) return "";
    
    size_t maxSize = JSStringGetMaximumUTF8CStringSize(jsStr);
    char* buffer = new char[maxSize];
    JSStringGetUTF8CString(jsStr, buffer, maxSize);
    
    std::string result(buffer);
    delete[] buffer;
    JSStringRelease(jsStr);
    
    return result;
}

std::string JSCExecutor::getJSException(JSValueRef exception) {
    return jsValueToString(exception);
}

void JSCExecutor::logJSError(const std::string& error, const std::string& source) {
    std::cerr << "[JSCExecutor] JavaScript 错误";
    if (!source.empty()) {
        std::cerr << " (" << source << ")";
    }
    std::cerr << ": " << error << std::endl;
}

JSCExecutor* JSCExecutor::getCurrentInstance() {
    return s_currentInstance;
}

// Native 函数实现
JSValueRef JSCExecutor::nativeFlushQueuedReactWork(JSContextRef ctx, JSObjectRef function,
                                                  JSObjectRef thisObject, size_t argumentCount,
                                                  const JSValueRef arguments[], JSValueRef* exception) {
    JSCExecutor* executor = getCurrentInstance();
    if (!executor) {
        return JSValueMakeNull(ctx);
    }
    
    std::cout << "[JSCExecutor] Native 函数被调用: __nativeFlushQueuedReactWork" << std::endl;
    
    // 获取待处理的调用队列
    std::vector<NativeCall> calls = executor->flushQueue();
    
    // 处理每个调用
    for (const auto& call : calls) {
        if (executor->m_nativeCallCallback) {
            executor->m_nativeCallCallback(call);
        }
    }
    
    return JSValueMakeNull(ctx);
}

JSValueRef JSCExecutor::nativeCallSyncHook(JSContextRef ctx, JSObjectRef function,
                                          JSObjectRef thisObject, size_t argumentCount,
                                          const JSValueRef arguments[], JSValueRef* exception) {
    std::cout << "[JSCExecutor] Native 函数被调用: __nativeCallSyncHook" << std::endl;
    
    // 同步调用处理 (简化实现)
    return JSValueMakeNull(ctx);
}

JSValueRef JSCExecutor::nativeLoggingHook(JSContextRef ctx, JSObjectRef function,
                                         JSObjectRef thisObject, size_t argumentCount,
                                         const JSValueRef arguments[], JSValueRef* exception) {
    JSCExecutor* executor = getCurrentInstance();
    if (!executor || argumentCount < 2) {
        return JSValueMakeNull(ctx);
    }
    
    std::string level = executor->jsValueToString(arguments[0]);
    std::string message = executor->jsValueToString(arguments[1]);
    
    std::cout << "[JS-" << level << "] " << message << std::endl;
    
    return JSValueMakeNull(ctx);
}

} // namespace bridge

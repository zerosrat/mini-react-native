/**
 * JSCExecutor.h
 * JavaScript Core 执行器 - C++ Bridge 的核心组件
 * 负责在 C++ 层管理 JavaScript 引擎和执行 JS 代码
 */

#ifndef JSCExecutor_h
#define JSCExecutor_h

#include <JavaScriptCore/JavaScriptCore.h>
#include <string>
#include <functional>
#include <memory>
#include <vector>
#include <map>

namespace bridge {

// 前向声明
class MessageQueue;
class NativeModule;

/**
 * JavaScript 执行结果
 */
struct JSExecuteResult {
    bool success;
    std::string result;
    std::string error;
    
    JSExecuteResult(bool s, const std::string& r, const std::string& e = "")
        : success(s), result(r), error(e) {}
};

/**
 * Native 调用信息
 */
struct NativeCall {
    int moduleId;
    int methodId;
    std::vector<std::string> arguments;
    int callbackId;
    
    NativeCall(int mod, int meth, const std::vector<std::string>& args, int callback = -1)
        : moduleId(mod), methodId(meth), arguments(args), callbackId(callback) {}
};

/**
 * JSCExecutor - JavaScript Core 执行器
 * 
 * 这个类是 React Native Bridge 的核心，它：
 * 1. 管理 JavaScript 运行时环境
 * 2. 执行 JavaScript 代码
 * 3. 处理 JS 和 Native 之间的数据转换
 * 4. 管理回调和异步调用
 */
class JSCExecutor {
public:
    using NativeCallCallback = std::function<void(const NativeCall&)>;
    using JSCallback = std::function<void(const JSExecuteResult&)>;
    
    JSCExecutor();
    ~JSCExecutor();
    
    /**
     * 初始化 JavaScript 引擎
     * @return 是否初始化成功
     */
    bool initialize();
    
    /**
     * 加载并执行 JavaScript 代码
     * @param script JavaScript 代码
     * @param sourceURL 源文件 URL (用于调试)
     * @return 执行结果
     */
    JSExecuteResult executeScript(const std::string& script, const std::string& sourceURL = "");
    
    /**
     * 调用 JavaScript 函数
     * @param functionName 函数名
     * @param arguments 参数数组 (JSON 格式)
     * @return 执行结果
     */
    JSExecuteResult callJSFunction(const std::string& functionName, const std::string& arguments);
    
    /**
     * 调用 JavaScript 回调函数
     * @param callbackId 回调 ID
     * @param arguments 参数数组 (JSON 格式)
     * @return 执行结果
     */
    JSExecuteResult invokeCallback(int callbackId, const std::string& arguments);
    
    /**
     * 刷新消息队列，获取待执行的 Native 调用
     * @return Native 调用列表
     */
    std::vector<NativeCall> flushQueue();
    
    /**
     * 注册 Native 模块
     * @param moduleId 模块 ID
     * @param moduleName 模块名
     * @param module Native 模块实例
     */
    void registerNativeModule(int moduleId, const std::string& moduleName, 
                             std::shared_ptr<NativeModule> module);
    
    /**
     * 设置 Native 调用回调
     * @param callback 回调函数
     */
    void setNativeCallCallback(NativeCallCallback callback);
    
    /**
     * 获取 JavaScript 全局对象的属性
     * @param propertyName 属性名
     * @return 属性值 (JSON 格式)
     */
    std::string getGlobalProperty(const std::string& propertyName);
    
    /**
     * 设置 JavaScript 全局对象的属性
     * @param propertyName 属性名
     * @param value 属性值 (JSON 格式)
     * @return 是否设置成功
     */
    bool setGlobalProperty(const std::string& propertyName, const std::string& value);
    
    /**
     * 注入 Native 函数到 JavaScript 环境
     */
    void injectNativeFunctions();
    
    /**
     * 销毁 JavaScript 引擎
     */
    void destroy();
    
    /**
     * 获取调试信息
     * @return 调试信息 (JSON 格式)
     */
    std::string getDebugInfo();

private:
    JSGlobalContextRef m_context;
    JSObjectRef m_globalObject;
    
    // Native 模块注册表
    std::map<int, std::shared_ptr<NativeModule>> m_nativeModules;
    std::map<std::string, int> m_moduleNameToId;
    
    // 回调管理
    NativeCallCallback m_nativeCallCallback;
    int m_nextCallbackId;
    
    // 工具方法
    JSValueRef stringToJSValue(const std::string& str);
    std::string jsValueToString(JSValueRef value);
    JSValueRef jsonStringToJSValue(const std::string& jsonStr);
    std::string jsValueToJSONString(JSValueRef value);
    
    // JavaScript 异常处理
    std::string getJSException(JSValueRef exception);
    void logJSError(const std::string& error, const std::string& source = "");
    
    // Native 函数实现 (注入到 JS 环境中)
    static JSValueRef nativeFlushQueuedReactWork(JSContextRef ctx, JSObjectRef function,
                                                JSObjectRef thisObject, size_t argumentCount,
                                                const JSValueRef arguments[], JSValueRef* exception);
    
    static JSValueRef nativeCallSyncHook(JSContextRef ctx, JSObjectRef function,
                                        JSObjectRef thisObject, size_t argumentCount,
                                        const JSValueRef arguments[], JSValueRef* exception);
                                        
    static JSValueRef nativeLoggingHook(JSContextRef ctx, JSObjectRef function,
                                       JSObjectRef thisObject, size_t argumentCount,
                                       const JSValueRef arguments[], JSValueRef* exception);
    
    // 获取当前实例 (用于静态方法回调)
    static JSCExecutor* getCurrentInstance();
    static JSCExecutor* s_currentInstance;
};

} // namespace bridge

#endif /* JSCExecutor_h */

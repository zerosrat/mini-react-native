/**
 * NativeModule.h
 * Native 模块基类 - 定义了 Native 模块的接口
 */

#ifndef NativeModule_h
#define NativeModule_h

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>

namespace bridge {

// 前向声明
class JSCExecutor;

/**
 * Native 方法的参数类型
 */
enum class NativeArgType {
    String,
    Number,
    Boolean,
    Object,
    Array,
    Callback
};

/**
 * Native 方法描述
 */
struct NativeMethodDescriptor {
    std::string name;
    std::vector<NativeArgType> argTypes;
    bool hasCallback;
    bool isSync;
    
    NativeMethodDescriptor(const std::string& n, 
                          const std::vector<NativeArgType>& types = {},
                          bool callback = false, 
                          bool sync = false)
        : name(n), argTypes(types), hasCallback(callback), isSync(sync) {}
};

/**
 * Native 模块基类
 * 
 * 所有 Native 模块都应该继承这个基类，并实现相应的虚函数
 */
class NativeModule {
public:
    NativeModule(const std::string& name);
    virtual ~NativeModule() = default;
    
    /**
     * 获取模块名
     */
    const std::string& getName() const { return m_name; }
    
    /**
     * 获取模块常量 (在 JS 端可直接访问)
     * @return 常量映射表
     */
    virtual std::map<std::string, std::string> getConstants() = 0;
    
    /**
     * 获取模块方法列表
     * @return 方法描述列表
     */
    virtual std::vector<NativeMethodDescriptor> getMethods() = 0;
    
    /**
     * 调用模块方法
     * @param methodId 方法 ID
     * @param arguments 参数列表 (JSON 格式)
     * @param callback 回调函数 (用于异步方法)
     * @return 方法返回值 (同步方法) 或空字符串 (异步方法)
     */
    virtual std::string callMethod(int methodId, const std::vector<std::string>& arguments,
                                  std::function<void(const std::string&, const std::string&)> callback) = 0;
    
    /**
     * 模块初始化 (可选重写)
     * @param executor JavaScript 执行器实例
     */
    virtual void initialize(JSCExecutor* executor) {}
    
    /**
     * 模块销毁 (可选重写)
     */
    virtual void destroy() {}
    
    /**
     * 获取方法的 JSON 描述 (供 JS 端使用)
     */
    std::string getMethodsJSON();
    
    /**
     * 获取常量的 JSON 描述 (供 JS 端使用)
     */
    std::string getConstantsJSON();
    
    /**
     * 根据方法名获取方法 ID
     */
    int getMethodId(const std::string& methodName);
    
    /**
     * 根据方法 ID 获取方法描述
     */
    const NativeMethodDescriptor* getMethodDescriptor(int methodId);

protected:
    std::string m_name;
    std::vector<NativeMethodDescriptor> m_methodDescriptors;
    std::map<std::string, int> m_methodNameToId;
    JSCExecutor* m_executor;
    
    /**
     * 注册方法 (在构造函数中调用)
     */
    void registerMethod(const NativeMethodDescriptor& descriptor);
    
    /**
     * 发送事件到 JavaScript 端
     * @param eventName 事件名
     * @param eventData 事件数据 (JSON 格式)
     */
    void sendEventToJS(const std::string& eventName, const std::string& eventData);
    
    /**
     * 解析 JSON 字符串
     */
    std::string parseJsonString(const std::string& jsonStr);
    
    /**
     * 创建 JSON 字符串
     */
    std::string createJsonString(const std::map<std::string, std::string>& data);
    
    /**
     * 日志输出
     */
    void log(const std::string& level, const std::string& message);
};

/**
 * Native 模块工厂
 */
class NativeModuleFactory {
public:
    using CreateFunction = std::function<std::shared_ptr<NativeModule>()>;
    
    /**
     * 注册模块创建函数
     */
    static void registerModule(const std::string& name, CreateFunction createFunc);
    
    /**
     * 创建模块实例
     */
    static std::shared_ptr<NativeModule> createModule(const std::string& name);
    
    /**
     * 获取所有已注册的模块名
     */
    static std::vector<std::string> getRegisteredModules();

private:
    static std::map<std::string, CreateFunction> s_moduleFactories;
};

/**
 * 模块注册宏
 */
#define REGISTER_NATIVE_MODULE(ClassName, ModuleName) \
    static bool _register_##ClassName = []() { \
        NativeModuleFactory::registerModule(ModuleName, []() { \
            return std::make_shared<ClassName>(); \
        }); \
        return true; \
    }();

} // namespace bridge

#endif /* NativeModule_h */

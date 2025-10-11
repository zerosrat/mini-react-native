/**
 * DeviceInfoModule.h
 * Android 设备信息模块 - 演示如何实现 Native Module
 */

#ifndef ANDROID_DEVICEINFOMODULE_H
#define ANDROID_DEVICEINFOMODULE_H

#include "../cpp/NativeModule.h"
#include <jni.h>
#include <string>
#include <memory>

/**
 * Android 设备信息模块
 * 提供设备相关信息的查询功能
 */
class AndroidDeviceInfoModule : public bridge::NativeModule {
public:
    AndroidDeviceInfoModule();
    virtual ~AndroidDeviceInfoModule();
    
    // 实现基类的纯虚函数
    std::map<std::string, std::string> getConstants() override;
    std::vector<bridge::NativeMethodDescriptor> getMethods() override;
    std::string callMethod(int methodId, const std::vector<std::string>& arguments,
                          std::function<void(const std::string&, const std::string&)> callback) override;
    
    void initialize(bridge::JSCExecutor* executor) override;
    void destroy() override;
    
    // 设置 JNI 环境
    void setJNIEnv(JNIEnv* env, jobject context);

private:
    // JNI 相关
    JNIEnv* m_env;
    jobject m_context;
    jclass m_deviceInfoClass;
    
    // 具体的方法实现
    void getDeviceId(const std::vector<std::string>& args,
                    std::function<void(const std::string&, const std::string&)> callback);
    
    void getBatteryLevel(const std::vector<std::string>& args,
                        std::function<void(const std::string&, const std::string&)> callback);
    
    void getNetworkState(const std::vector<std::string>& args,
                        std::function<void(const std::string&, const std::string&)> callback);
    
    void getSystemInfo(const std::vector<std::string>& args,
                      std::function<void(const std::string&, const std::string&)> callback);
    
    void startBatteryMonitoring(const std::vector<std::string>& args,
                               std::function<void(const std::string&, const std::string&)> callback);
    
    void stopBatteryMonitoring(const std::vector<std::string>& args,
                              std::function<void(const std::string&, const std::string&)> callback);
    
    // JNI 工具方法
    std::string jstringToString(jstring jstr);
    jstring stringToJstring(const std::string& str);
    
    std::string callJavaMethod(const std::string& methodName, const std::string& signature);
    std::string callJavaMethodWithArgs(const std::string& methodName, const std::string& signature, ...);
    
    // 获取设备信息的 Java 方法
    std::string getSystemProperty(const std::string& key);
    std::string getAndroidId();
    std::string getDeviceModel();
    std::string getDeviceManufacturer();
    std::string getSystemVersion();
    std::string getNetworkType();
    float getBatteryLevel();
    
    bool m_isBatteryMonitoringEnabled;
};

#endif /* ANDROID_DEVICEINFOMODULE_H */

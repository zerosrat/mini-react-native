/**
 * DeviceInfoModule.h
 * iOS 设备信息模块 - 演示如何实现 Native Module
 */

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#include "../cpp/NativeModule.h"

/**
 * iOS 设备信息模块
 * 提供设备相关信息的查询功能
 */
class DeviceInfoModule : public bridge::NativeModule {
public:
    DeviceInfoModule();
    virtual ~DeviceInfoModule() = default;
    
    // 实现基类的纯虚函数
    std::map<std::string, std::string> getConstants() override;
    std::vector<bridge::NativeMethodDescriptor> getMethods() override;
    std::string callMethod(int methodId, const std::vector<std::string>& arguments,
                          std::function<void(const std::string&, const std::string&)> callback) override;
    
    void initialize(bridge::JSCExecutor* executor) override;

private:
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
    
    // 工具方法
    std::string getSystemVersion();
    std::string getDeviceModel();
    std::string getDeviceName();
    std::string generateDeviceId();
    std::string getNetworkType();
    float getCurrentBatteryLevel();
    
    // Objective-C 辅助对象
    id batteryLevelObserver;
    bool isBatteryMonitoringEnabled;
};

// Objective-C 辅助类声明
@interface DeviceInfoHelper : NSObject
+ (NSString*)getSystemVersion;
+ (NSString*)getDeviceModel;
+ (NSString*)getDeviceName;
+ (NSString*)getUniqueIdentifier;
+ (NSString*)getNetworkType;
+ (float)getBatteryLevel;
+ (void)enableBatteryMonitoring:(BOOL)enable;
@end

/**
 * DeviceInfoModule.mm
 * iOS 设备信息模块的实现
 */

#import "DeviceInfoModule.h"
#import <SystemConfiguration/SystemConfiguration.h>
#import <CoreTelephony/CTTelephonyNetworkInfo.h>
#import <CoreTelephony/CTCarrier.h>
#include <json/json.h>
#include <sstream>

// 注册模块到工厂
REGISTER_NATIVE_MODULE(DeviceInfoModule, "DeviceInfo")

DeviceInfoModule::DeviceInfoModule() 
    : NativeModule("DeviceInfo"), batteryLevelObserver(nil), isBatteryMonitoringEnabled(false) {
    log("info", "DeviceInfo 模块初始化");
}

std::map<std::string, std::string> DeviceInfoModule::getConstants() {
    std::map<std::string, std::string> constants;
    
    // 设备基本信息常量
    constants["DEVICE_TYPE"] = "ios";
    constants["OS_VERSION"] = getSystemVersion();
    constants["DEVICE_MODEL"] = getDeviceModel();
    constants["DEVICE_NAME"] = getDeviceName();
    constants["BUNDLE_ID"] = [[[NSBundle mainBundle] bundleIdentifier] UTF8String];
    constants["APP_VERSION"] = [[[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleShortVersionString"] UTF8String];
    constants["BUILD_NUMBER"] = [[[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleVersion"] UTF8String];
    
    // 屏幕信息
    CGRect screenBounds = [[UIScreen mainScreen] bounds];
    CGFloat scale = [[UIScreen mainScreen] scale];
    
    Json::Value screenInfo;
    screenInfo["width"] = screenBounds.size.width * scale;
    screenInfo["height"] = screenBounds.size.height * scale;
    screenInfo["scale"] = scale;
    
    Json::StreamWriterBuilder builder;
    constants["SCREEN_INFO"] = Json::writeString(builder, screenInfo);
    
    log("info", "返回设备常量，共 " + std::to_string(constants.size()) + " 个");
    return constants;
}

std::vector<bridge::NativeMethodDescriptor> DeviceInfoModule::getMethods() {
    return {
        bridge::NativeMethodDescriptor("getDeviceId", {}, true, false),
        bridge::NativeMethodDescriptor("getBatteryLevel", {}, true, false),
        bridge::NativeMethodDescriptor("getNetworkState", {}, true, false),
        bridge::NativeMethodDescriptor("getSystemInfo", {}, true, false),
        bridge::NativeMethodDescriptor("startBatteryMonitoring", {}, true, false),
        bridge::NativeMethodDescriptor("stopBatteryMonitoring", {}, true, false)
    };
}

std::string DeviceInfoModule::callMethod(int methodId, const std::vector<std::string>& arguments,
                                        std::function<void(const std::string&, const std::string&)> callback) {
    log("info", "调用方法 ID: " + std::to_string(methodId));
    
    switch (methodId) {
        case 0: // getDeviceId
            getDeviceId(arguments, callback);
            break;
        case 1: // getBatteryLevel  
            getBatteryLevel(arguments, callback);
            break;
        case 2: // getNetworkState
            getNetworkState(arguments, callback);
            break;
        case 3: // getSystemInfo
            getSystemInfo(arguments, callback);
            break;
        case 4: // startBatteryMonitoring
            startBatteryMonitoring(arguments, callback);
            break;
        case 5: // stopBatteryMonitoring
            stopBatteryMonitoring(arguments, callback);
            break;
        default:
            log("error", "未知方法 ID: " + std::to_string(methodId));
            if (callback) {
                callback("方法未找到", "");
            }
            break;
    }
    
    return ""; // 异步方法返回空字符串
}

void DeviceInfoModule::initialize(bridge::JSCExecutor* executor) {
    NativeModule::initialize(executor);
    log("info", "模块初始化完成");
}

// 方法实现

void DeviceInfoModule::getDeviceId(const std::vector<std::string>& args,
                                  std::function<void(const std::string&, const std::string&)> callback) {
    log("info", "获取设备 ID");
    
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        std::string deviceId = generateDeviceId();
        
        dispatch_async(dispatch_get_main_queue(), ^{
            if (callback) {
                callback("", "\"" + deviceId + "\"");
            }
        });
    });
}

void DeviceInfoModule::getBatteryLevel(const std::vector<std::string>& args,
                                      std::function<void(const std::string&, const std::string&)> callback) {
    log("info", "获取电池电量");
    
    float batteryLevel = getCurrentBatteryLevel();
    
    Json::Value result;
    result["level"] = batteryLevel;
    result["state"] = [[UIDevice currentDevice] batteryState];
    
    Json::StreamWriterBuilder builder;
    std::string jsonResult = Json::writeString(builder, result);
    
    if (callback) {
        callback("", jsonResult);
    }
}

void DeviceInfoModule::getNetworkState(const std::vector<std::string>& args,
                                      std::function<void(const std::string&, const std::string&)> callback) {
    log("info", "获取网络状态");
    
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        std::string networkType = getNetworkType();
        
        Json::Value result;
        result["type"] = networkType;
        result["isConnected"] = (networkType != "none");
        
        Json::StreamWriterBuilder builder;
        std::string jsonResult = Json::writeString(builder, result);
        
        dispatch_async(dispatch_get_main_queue(), ^{
            if (callback) {
                callback("", jsonResult);
            }
        });
    });
}

void DeviceInfoModule::getSystemInfo(const std::vector<std::string>& args,
                                    std::function<void(const std::string&, const std::string&)> callback) {
    log("info", "获取系统信息");
    
    Json::Value systemInfo;
    systemInfo["systemName"] = [[[UIDevice currentDevice] systemName] UTF8String];
    systemInfo["systemVersion"] = getSystemVersion();
    systemInfo["model"] = getDeviceModel();
    systemInfo["name"] = getDeviceName();
    systemInfo["userInterfaceIdiom"] = ([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPad) ? "pad" : "phone";
    
    // 内存信息
    NSProcessInfo *processInfo = [NSProcessInfo processInfo];
    systemInfo["physicalMemory"] = [processInfo physicalMemory];
    
    // 存储信息
    NSError *error = nil;
    NSDictionary *fileAttributes = [[NSFileManager defaultManager] attributesOfFileSystemForPath:NSHomeDirectory() error:&error];
    if (!error) {
        systemInfo["totalDiskSpace"] = [[fileAttributes objectForKey:NSFileSystemSize] longLongValue];
        systemInfo["freeDiskSpace"] = [[fileAttributes objectForKey:NSFileSystemFreeSize] longLongValue];
    }
    
    Json::StreamWriterBuilder builder;
    std::string jsonResult = Json::writeString(builder, systemInfo);
    
    if (callback) {
        callback("", jsonResult);
    }
}

void DeviceInfoModule::startBatteryMonitoring(const std::vector<std::string>& args,
                                             std::function<void(const std::string&, const std::string&)> callback) {
    log("info", "开始电池监控");
    
    if (!isBatteryMonitoringEnabled) {
        [DeviceInfoHelper enableBatteryMonitoring:YES];
        
        // 监听电池状态变化
        batteryLevelObserver = [[NSNotificationCenter defaultCenter] 
            addObserverForName:UIDeviceBatteryLevelDidChangeNotification
                        object:nil
                         queue:[NSOperationQueue mainQueue]
                    usingBlock:^(NSNotification *notification) {
                        float level = getCurrentBatteryLevel();
                        
                        Json::Value eventData;
                        eventData["level"] = level;
                        eventData["timestamp"] = [[NSDate date] timeIntervalSince1970] * 1000;
                        
                        Json::StreamWriterBuilder builder;
                        std::string jsonData = Json::writeString(builder, eventData);
                        
                        sendEventToJS("batteryLevelChanged", jsonData);
                    }];
        
        isBatteryMonitoringEnabled = true;
    }
    
    if (callback) {
        callback("", "true");
    }
}

void DeviceInfoModule::stopBatteryMonitoring(const std::vector<std::string>& args,
                                            std::function<void(const std::string&, const std::string&)> callback) {
    log("info", "停止电池监控");
    
    if (isBatteryMonitoringEnabled && batteryLevelObserver) {
        [[NSNotificationCenter defaultCenter] removeObserver:batteryLevelObserver];
        batteryLevelObserver = nil;
        
        [DeviceInfoHelper enableBatteryMonitoring:NO];
        isBatteryMonitoringEnabled = false;
    }
    
    if (callback) {
        callback("", "true");
    }
}

// 工具方法实现

std::string DeviceInfoModule::getSystemVersion() {
    return [[DeviceInfoHelper getSystemVersion] UTF8String];
}

std::string DeviceInfoModule::getDeviceModel() {
    return [[DeviceInfoHelper getDeviceModel] UTF8String];
}

std::string DeviceInfoModule::getDeviceName() {
    return [[DeviceInfoHelper getDeviceName] UTF8String];
}

std::string DeviceInfoModule::generateDeviceId() {
    return [[DeviceInfoHelper getUniqueIdentifier] UTF8String];
}

std::string DeviceInfoModule::getNetworkType() {
    return [[DeviceInfoHelper getNetworkType] UTF8String];
}

float DeviceInfoModule::getCurrentBatteryLevel() {
    return [DeviceInfoHelper getBatteryLevel];
}

// Objective-C 辅助类实现

@implementation DeviceInfoHelper

+ (NSString*)getSystemVersion {
    return [[UIDevice currentDevice] systemVersion];
}

+ (NSString*)getDeviceModel {
    struct utsname systemInfo;
    uname(&systemInfo);
    return [NSString stringWithCString:systemInfo.machine encoding:NSUTF8StringEncoding];
}

+ (NSString*)getDeviceName {
    return [[UIDevice currentDevice] name];
}

+ (NSString*)getUniqueIdentifier {
    // 使用 identifierForVendor 作为设备唯一标识
    NSUUID *uuid = [[UIDevice currentDevice] identifierForVendor];
    return [uuid UUIDString];
}

+ (NSString*)getNetworkType {
    // 简化的网络类型检测
    SCNetworkReachabilityRef reachability = SCNetworkReachabilityCreateWithName(NULL, "8.8.8.8");
    SCNetworkReachabilityFlags flags;
    
    if (SCNetworkReachabilityGetFlags(reachability, &flags)) {
        CFRelease(reachability);
        
        if (!(flags & kSCNetworkReachabilityFlagsReachable)) {
            return @"none";
        }
        
        if (!(flags & kSCNetworkReachabilityFlagsIsWWAN)) {
            return @"wifi";
        }
        
        // 检测蜂窝网络类型
        CTTelephonyNetworkInfo *networkInfo = [[CTTelephonyNetworkInfo alloc] init];
        NSString *radioType = networkInfo.currentRadioAccessTechnology;
        
        if ([radioType isEqualToString:CTRadioAccessTechnologyLTE]) {
            return @"4g";
        } else if ([radioType isEqualToString:CTRadioAccessTechnologyWCDMA] ||
                   [radioType isEqualToString:CTRadioAccessTechnologyHSDPA] ||
                   [radioType isEqualToString:CTRadioAccessTechnologyHSUPA]) {
            return @"3g";
        } else {
            return @"2g";
        }
    }
    
    CFRelease(reachability);
    return @"unknown";
}

+ (float)getBatteryLevel {
    [[UIDevice currentDevice] setBatteryMonitoringEnabled:YES];
    float batteryLevel = [[UIDevice currentDevice] batteryLevel];
    return batteryLevel >= 0.0f ? batteryLevel : -1.0f;
}

+ (void)enableBatteryMonitoring:(BOOL)enable {
    [[UIDevice currentDevice] setBatteryMonitoringEnabled:enable];
}

@end

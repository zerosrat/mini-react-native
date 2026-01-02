#include "common/modules/DeviceInfoModule.h"
#import <sys/sysctl.h>
#include <iostream>
#include <sstream>
#include "common/utils/JSONParser.h"

// 使用最小依赖，避免 UIKit 冲突
#ifdef __OBJC__
#import <Foundation/Foundation.h>
#endif

namespace mini_rn {
namespace modules {

// 构造函数
DeviceInfoModule::DeviceInfoModule() {}

// NativeModule 接口实现
std::string DeviceInfoModule::getName() const { return "DeviceInfo"; }

std::vector<std::string> DeviceInfoModule::getMethods() const {
  return {
      "getUniqueId",       // methodId = 0
      "getSystemVersion",  // methodId = 1
      "getDeviceId"        // methodId = 2
  };
}

void DeviceInfoModule::invoke(const std::string& methodName, const std::string& args, int callId) {
  try {
    if (methodName == "getUniqueId") {
      std::string uniqueId = getUniqueIdImpl();
      sendSuccessCallback(callId, uniqueId);
    } else {
      sendErrorCallback(callId, "Unknown method: " + methodName);
    }
  } catch (const std::exception& e) {
    sendErrorCallback(callId, "Method invocation failed: " + std::string(e.what()));
  }
}

// iOS 平台特定实现
std::string DeviceInfoModule::getUniqueIdImpl() const {
  @autoreleasepool {
    // iOS 简化实现：使用 NSUUID 生成唯一标识
    // 注意：这个实现每次启动都会生成新的ID，适用于MVP测试
    NSUUID* uuid = [NSUUID UUID];
    NSString* uuidString = [uuid UUIDString];
    return [uuidString UTF8String];
  }
}

std::string DeviceInfoModule::getSystemVersionImpl() const {
  @autoreleasepool {
    // iOS 简化实现：使用 NSProcessInfo 获取系统版本（避免 UIDevice 依赖）
    NSProcessInfo* processInfo = [NSProcessInfo processInfo];
    NSOperatingSystemVersion version = [processInfo operatingSystemVersion];

    std::ostringstream oss;
    oss << version.majorVersion << "." << version.minorVersion << "." << version.patchVersion;

    return oss.str();
  }
}

std::string DeviceInfoModule::getDeviceIdImpl() const {
  @autoreleasepool {
    // iOS 简化实现：直接使用 sysctl 获取设备型号（避免 UIDevice 依赖）
    size_t size = 0;
    sysctlbyname("hw.machine", nullptr, &size, nullptr, 0);

    if (size > 0) {
      std::vector<char> buffer(size);
      if (sysctlbyname("hw.machine", buffer.data(), &size, nullptr, 0) == 0) {
        return std::string(buffer.data());
      }
    }

    // 备选方案：iOS 模拟器标识
    return "iOS-Simulator";
  }
}

// 工具方法实现 - sendSuccessCallback 和 sendErrorCallback 现在由基类提供

std::string DeviceInfoModule::createSuccessResponse(const std::string& data) const {
  // React Native 回调约定：直接返回数据，不需要包装对象
  return data;
}

std::string DeviceInfoModule::createErrorResponse(const std::string& error) const {
  // React Native 回调约定：直接返回错误消息
  return error;
}

}  // namespace modules
}  // namespace mini_rn
#include "common/modules/DeviceInfoModule.h"
#import <Foundation/Foundation.h>
#import <IOKit/IOKitLib.h>
#import <sys/sysctl.h>
#include <iostream>
#include <sstream>
#include "common/utils/JSONParser.h"

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

// macOS 平台特定实现
std::string DeviceInfoModule::getUniqueIdImpl() const {
  @autoreleasepool {
    // 尝试获取硬件 UUID
    io_registry_entry_t ioRegistryRoot =
        IORegistryEntryFromPath(kIOMasterPortDefault, "IOService:/");
    CFStringRef uuidCf = (CFStringRef)IORegistryEntryCreateCFProperty(
        ioRegistryRoot, CFSTR(kIOPlatformUUIDKey), kCFAllocatorDefault, 0);
    IOObjectRelease(ioRegistryRoot);

    if (uuidCf) {
      NSString* uuid = (__bridge NSString*)uuidCf;
      std::string result = [uuid UTF8String];
      CFRelease(uuidCf);
      return result;
    }

    // 备选方案：使用系统序列号
    size_t size = 0;
    sysctlbyname("kern.uuid", nullptr, &size, nullptr, 0);
    if (size > 0) {
      std::vector<char> buffer(size);
      if (sysctlbyname("kern.uuid", buffer.data(), &size, nullptr, 0) == 0) {
        return std::string(buffer.data());
      }
    }

    // 最后备选：生成基于设备信息的标识
    return "macOS-" + getDeviceIdImpl() + "-" + getSystemVersionImpl();
  }
}

std::string DeviceInfoModule::getSystemVersionImpl() const {
  @autoreleasepool {
    // 获取单例 NSProcessInfo 实例，用于查询运行时进程/系统信息。
    NSProcessInfo* processInfo = [NSProcessInfo processInfo];
    // 调用 processInfo 的 operatingSystemVersion 方法，得到一个包含 major/minor/patch 的结构体。
    NSOperatingSystemVersion version = [processInfo operatingSystemVersion];

    // 创建一个 C++ 的输出字符串流，用来拼接版本号字符串（例如 "14.2.1"）。
    std::ostringstream oss;
    // 将主版本、次版本、补丁按 “主.次.补丁” 形式写入字符串流。
    oss << version.majorVersion << "." << version.minorVersion << "." << version.patchVersion;

    // 将 ostringstream 中的内容转为 std::string 并返回。
    return oss.str();
  }
}

std::string DeviceInfoModule::getDeviceIdImpl() const {
  @autoreleasepool {
    size_t size = 0;
    sysctlbyname("hw.model", nullptr, &size, nullptr, 0);

    if (size > 0) {
      std::vector<char> buffer(size);
      if (sysctlbyname("hw.model", buffer.data(), &size, nullptr, 0) == 0) {
        return std::string(buffer.data());
      }
    }

    // 备选方案
    return "Unknown";
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
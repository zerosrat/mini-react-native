#import <Foundation/Foundation.h>
#import <IOKit/IOKitLib.h>
#import <sys/sysctl.h>
#include "common/modules/DeviceInfoModule.h"
#include "common/utils/JSONParser.h"
#include <iostream>
#include <sstream>

namespace mini_rn {
namespace modules {

// 构造函数
DeviceInfoModule::DeviceInfoModule(CallbackHandler callbackHandler)
    : m_callbackHandler(std::move(callbackHandler)) {
}

// NativeModule 接口实现
std::string DeviceInfoModule::getName() const {
    return "DeviceInfo";
}

std::vector<std::string> DeviceInfoModule::getMethods() const {
    return {
        "getUniqueId",      // methodId = 0
        "getSystemVersion", // methodId = 1
        "getDeviceId"       // methodId = 2
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
        io_registry_entry_t ioRegistryRoot = IORegistryEntryFromPath(kIOMasterPortDefault, "IOService:/");
        CFStringRef uuidCf = (CFStringRef) IORegistryEntryCreateCFProperty(ioRegistryRoot,
                                                                           CFSTR(kIOPlatformUUIDKey),
                                                                           kCFAllocatorDefault, 0);
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
        NSProcessInfo* processInfo = [NSProcessInfo processInfo];
        NSOperatingSystemVersion version = [processInfo operatingSystemVersion];

        std::ostringstream oss;
        oss << version.majorVersion << "." << version.minorVersion << "." << version.patchVersion;

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

// 工具方法实现
void DeviceInfoModule::sendSuccessCallback(int callId, const std::string& result) {
    if (m_callbackHandler) {
        std::string response = createSuccessResponse(result);
        m_callbackHandler(callId, response, false);
    }
}

void DeviceInfoModule::sendErrorCallback(int callId, const std::string& error) {
    if (m_callbackHandler) {
        std::string response = createErrorResponse(error);
        m_callbackHandler(callId, response, true);
    }
}

std::string DeviceInfoModule::createSuccessResponse(const std::string& data) const {
    // React Native 回调约定：直接返回数据，不需要包装对象
    return data;
}

std::string DeviceInfoModule::createErrorResponse(const std::string& error) const {
    // React Native 回调约定：直接返回错误消息
    return error;
}

} // namespace modules
} // namespace mini_rn
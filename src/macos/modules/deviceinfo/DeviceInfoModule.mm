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
        "getUniqueId",
        "getSystemVersion",
        "getModel",
        "getSystemName"
    };
}

std::map<std::string, std::string> DeviceInfoModule::getConstants() const {
    std::map<std::string, std::string> constants;

    try {
        constants["systemName"] = getSystemNameImpl();
        constants["systemVersion"] = getSystemVersionImpl();
        constants["model"] = getModelImpl();
        constants["uniqueId"] = getUniqueIdImpl();
    } catch (const std::exception& e) {
        std::cerr << "[DeviceInfo] Error getting constants: " << e.what() << std::endl;
    }

    return constants;
}

void DeviceInfoModule::invoke(const std::string& methodName, const std::string& args, int callId) {
    try {
        if (methodName == "getUniqueId") {
            handleGetUniqueId(args, callId);
        } else if (methodName == "getSystemVersion") {
            handleGetSystemVersion(args, callId);
        } else if (methodName == "getModel") {
            handleGetModel(args, callId);
        } else if (methodName == "getSystemName") {
            handleGetSystemName(args, callId);
        } else {
            sendErrorCallback(callId, "Unknown method: " + methodName);
        }
    } catch (const std::exception& e) {
        sendErrorCallback(callId, "Method invocation failed: " + std::string(e.what()));
    }
}

// 方法处理实现
void DeviceInfoModule::handleGetUniqueId(const std::string& args, int callId) {
    try {
        std::string uniqueId = getUniqueIdImpl();
        sendSuccessCallback(callId, uniqueId);
    } catch (const std::exception& e) {
        sendErrorCallback(callId, "Failed to get unique ID: " + std::string(e.what()));
    }
}

void DeviceInfoModule::handleGetSystemVersion(const std::string& args, int callId) {
    try {
        std::string systemVersion = getSystemVersionImpl();
        sendSuccessCallback(callId, systemVersion);
    } catch (const std::exception& e) {
        sendErrorCallback(callId, "Failed to get system version: " + std::string(e.what()));
    }
}

void DeviceInfoModule::handleGetModel(const std::string& args, int callId) {
    try {
        std::string model = getModelImpl();
        sendSuccessCallback(callId, model);
    } catch (const std::exception& e) {
        sendErrorCallback(callId, "Failed to get model: " + std::string(e.what()));
    }
}

void DeviceInfoModule::handleGetSystemName(const std::string& args, int callId) {
    try {
        std::string systemName = getSystemNameImpl();
        sendSuccessCallback(callId, systemName);
    } catch (const std::exception& e) {
        sendErrorCallback(callId, "Failed to get system name: " + std::string(e.what()));
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
        return "macOS-" + getModelImpl() + "-" + getSystemVersionImpl();
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

std::string DeviceInfoModule::getModelImpl() const {
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
        return "Unknown Mac";
    }
}

std::string DeviceInfoModule::getSystemNameImpl() const {
    return "macOS";
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
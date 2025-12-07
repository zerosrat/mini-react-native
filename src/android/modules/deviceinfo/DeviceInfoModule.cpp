/**
 * Android DeviceInfo Module Implementation
 *
 * This file provides Android-specific implementation of the DeviceInfo module,
 * maintaining API compatibility with macOS/iOS versions while using Android
 * system APIs to gather device information.
 */

#include "../../../common/modules/DeviceInfoModule.h"
#include <android/log.h>
#include <sys/system_properties.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <vector>

#define LOG_TAG "DeviceInfoModule"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace mini_rn {
namespace modules {

// Constructor
DeviceInfoModule::DeviceInfoModule() {
    LOGD("DeviceInfoModule created for Android platform");
}

// NativeModule interface implementation
std::string DeviceInfoModule::getName() const {
    return "DeviceInfo";
}

std::vector<std::string> DeviceInfoModule::getMethods() const {
    return {
        "getUniqueId",       // methodId = 0 - Promise method
        "getSystemVersion",  // methodId = 1 - Sync method
        "getDeviceId"        // methodId = 2 - Sync method
    };
}

void DeviceInfoModule::invoke(const std::string& methodName, const std::string& args, int callId) {
    try {
        LOGD("Invoking method: %s with callId: %d", methodName.c_str(), callId);

        if (methodName == "getUniqueId") {
            // Async method - returns via callback
            std::string uniqueId = getUniqueIdImpl();
            LOGD("getUniqueId result: %s", uniqueId.c_str());
            sendSuccessCallback(callId, uniqueId);
        } else if (methodName == "getSystemVersion") {
            // Sync method - returns immediately
            std::string version = getSystemVersionImpl();
            LOGD("getSystemVersion result: %s", version.c_str());
            sendSuccessCallback(callId, version);
        } else if (methodName == "getDeviceId") {
            // Sync method - returns immediately
            std::string deviceId = getDeviceIdImpl();
            LOGD("getDeviceId result: %s", deviceId.c_str());
            sendSuccessCallback(callId, deviceId);
        } else {
            LOGE("Unknown method: %s", methodName.c_str());
            sendErrorCallback(callId, "Unknown method: " + methodName);
        }
    } catch (const std::exception& e) {
        LOGE("Method invocation failed: %s", e.what());
        sendErrorCallback(callId, "Method invocation failed: " + std::string(e.what()));
    }
}

// Android platform-specific implementations

std::string DeviceInfoModule::getUniqueIdImpl() const {
    char prop_value[PROP_VALUE_MAX];

    LOGD("Getting unique device ID for Android");

    // Primary: Try to get Android ID (Settings.Secure.ANDROID_ID equivalent)
    if (__system_property_get("ro.serialno", prop_value) > 0) {
        std::string result(prop_value);
        if (!result.empty() && result != "unknown") {
            LOGD("Using ro.serialno: %s", result.c_str());
            return result;
        }
    }

    // Secondary: Try boot serial number
    if (__system_property_get("ro.boot.serialno", prop_value) > 0) {
        std::string result(prop_value);
        if (!result.empty() && result != "unknown") {
            LOGD("Using ro.boot.serialno: %s", result.c_str());
            return result;
        }
    }

    // Tertiary: Try hardware serial
    if (__system_property_get("ro.hardware", prop_value) > 0) {
        std::string hardware(prop_value);
        if (!hardware.empty()) {
            // Combine hardware info with build info for uniqueness
            std::string combined = "android-" + hardware + "-" + getDeviceIdImpl();
            LOGD("Using hardware-based ID: %s", combined.c_str());
            return combined;
        }
    }

    // Fallback: Generate based on device info
    std::string fallback = "android-" + getDeviceIdImpl() + "-" + getSystemVersionImpl();
    LOGD("Using fallback ID: %s", fallback.c_str());
    return fallback;
}

std::string DeviceInfoModule::getSystemVersionImpl() const {
    char prop_value[PROP_VALUE_MAX];

    LOGD("Getting Android system version");

    // Get Android version (e.g., "11", "12", "13")
    if (__system_property_get("ro.build.version.release", prop_value) > 0) {
        std::string version(prop_value);
        LOGD("Android version: %s", version.c_str());
        return version;
    }

    // Fallback: Try SDK version
    if (__system_property_get("ro.build.version.sdk", prop_value) > 0) {
        std::string sdk(prop_value);
        LOGD("SDK version: %s", sdk.c_str());
        return "API " + sdk;
    }

    LOGD("Could not determine system version, using fallback");
    return "Unknown";
}

std::string DeviceInfoModule::getDeviceIdImpl() const {
    char prop_value[PROP_VALUE_MAX];

    LOGD("Getting Android device ID");

    // Primary: Device model (e.g., "Pixel 7", "SM-G991B")
    if (__system_property_get("ro.product.model", prop_value) > 0) {
        std::string model(prop_value);
        if (!model.empty()) {
            LOGD("Device model: %s", model.c_str());
            return model;
        }
    }

    // Secondary: Product name
    if (__system_property_get("ro.product.name", prop_value) > 0) {
        std::string product(prop_value);
        if (!product.empty()) {
            LOGD("Product name: %s", product.c_str());
            return product;
        }
    }

    // Tertiary: Device name
    if (__system_property_get("ro.product.device", prop_value) > 0) {
        std::string device(prop_value);
        if (!device.empty()) {
            LOGD("Device name: %s", device.c_str());
            return device;
        }
    }

    LOGD("Could not determine device ID, using fallback");
    return "Unknown";
}

// Utility methods implementation

std::string DeviceInfoModule::createSuccessResponse(const std::string& data) const {
    // React Native callback convention: return data directly, no wrapper object
    return data;
}

std::string DeviceInfoModule::createErrorResponse(const std::string& error) const {
    // React Native callback convention: return error message directly
    return error;
}

} // namespace modules
} // namespace mini_rn
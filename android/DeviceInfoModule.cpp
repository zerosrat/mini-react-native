/**
 * DeviceInfoModule.cpp
 * Android 设备信息模块的实现
 */

#include "DeviceInfoModule.h"
#include <json/json.h>
#include <sstream>
#include <android/log.h>

// Android 日志宏
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "DeviceInfoModule", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "DeviceInfoModule", __VA_ARGS__)

// 注册模块到工厂
REGISTER_NATIVE_MODULE(AndroidDeviceInfoModule, "DeviceInfo")

AndroidDeviceInfoModule::AndroidDeviceInfoModule() 
    : NativeModule("DeviceInfo")
    , m_env(nullptr)
    , m_context(nullptr)
    , m_deviceInfoClass(nullptr)
    , m_isBatteryMonitoringEnabled(false) {
    LOGI("DeviceInfo 模块初始化");
}

AndroidDeviceInfoModule::~AndroidDeviceInfoModule() {
    destroy();
}

void AndroidDeviceInfoModule::setJNIEnv(JNIEnv* env, jobject context) {
    m_env = env;
    m_context = env->NewGlobalRef(context);
    
    // 查找我们的辅助 Java 类
    m_deviceInfoClass = env->FindClass("com/bridge/DeviceInfoHelper");
    if (m_deviceInfoClass) {
        m_deviceInfoClass = (jclass)env->NewGlobalRef(m_deviceInfoClass);
        LOGI("成功找到 DeviceInfoHelper 类");
    } else {
        LOGE("未找到 DeviceInfoHelper 类");
    }
}

std::map<std::string, std::string> AndroidDeviceInfoModule::getConstants() {
    std::map<std::string, std::string> constants;
    
    if (!m_env) {
        log("error", "JNI 环境未设置");
        return constants;
    }
    
    // 设备基本信息常量
    constants["DEVICE_TYPE"] = "android";
    constants["OS_VERSION"] = getSystemVersion();
    constants["DEVICE_MODEL"] = getDeviceModel();
    constants["DEVICE_MANUFACTURER"] = getDeviceManufacturer();
    
    // 系统属性
    constants["SDK_VERSION"] = getSystemProperty("ro.build.version.sdk");
    constants["BUILD_ID"] = getSystemProperty("ro.build.id");
    constants["HARDWARE"] = getSystemProperty("ro.hardware");
    
    // 屏幕信息 (通过 JNI 获取)
    if (m_deviceInfoClass) {
        jmethodID method = m_env->GetStaticMethodID(m_deviceInfoClass, "getScreenInfo", "(Landroid/content/Context;)Ljava/lang/String;");
        if (method) {
            jstring result = (jstring)m_env->CallStaticObjectMethod(m_deviceInfoClass, method, m_context);
            if (result) {
                constants["SCREEN_INFO"] = jstringToString(result);
                m_env->DeleteLocalRef(result);
            }
        }
    }
    
    log("info", "返回设备常量，共 " + std::to_string(constants.size()) + " 个");
    return constants;
}

std::vector<bridge::NativeMethodDescriptor> AndroidDeviceInfoModule::getMethods() {
    return {
        bridge::NativeMethodDescriptor("getDeviceId", {}, true, false),
        bridge::NativeMethodDescriptor("getBatteryLevel", {}, true, false),
        bridge::NativeMethodDescriptor("getNetworkState", {}, true, false),
        bridge::NativeMethodDescriptor("getSystemInfo", {}, true, false),
        bridge::NativeMethodDescriptor("startBatteryMonitoring", {}, true, false),
        bridge::NativeMethodDescriptor("stopBatteryMonitoring", {}, true, false)
    };
}

std::string AndroidDeviceInfoModule::callMethod(int methodId, const std::vector<std::string>& arguments,
                                               std::function<void(const std::string&, const std::string&)> callback) {
    log("info", "调用方法 ID: " + std::to_string(methodId));
    
    if (!m_env) {
        log("error", "JNI 环境未设置");
        if (callback) {
            callback("JNI 环境未设置", "");
        }
        return "";
    }
    
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

void AndroidDeviceInfoModule::initialize(bridge::JSCExecutor* executor) {
    NativeModule::initialize(executor);
    log("info", "模块初始化完成");
}

void AndroidDeviceInfoModule::destroy() {
    if (m_env) {
        if (m_context) {
            m_env->DeleteGlobalRef(m_context);
            m_context = nullptr;
        }
        if (m_deviceInfoClass) {
            m_env->DeleteGlobalRef(m_deviceInfoClass);
            m_deviceInfoClass = nullptr;
        }
        m_env = nullptr;
    }
    
    log("info", "模块销毁完成");
}

// 方法实现

void AndroidDeviceInfoModule::getDeviceId(const std::vector<std::string>& args,
                                         std::function<void(const std::string&, const std::string&)> callback) {
    log("info", "获取设备 ID");
    
    std::string deviceId = getAndroidId();
    
    if (callback) {
        callback("", "\"" + deviceId + "\"");
    }
}

void AndroidDeviceInfoModule::getBatteryLevel(const std::vector<std::string>& args,
                                             std::function<void(const std::string&, const std::string&)> callback) {
    log("info", "获取电池电量");
    
    if (!m_deviceInfoClass) {
        if (callback) {
            callback("DeviceInfoHelper 类未找到", "");
        }
        return;
    }
    
    // 调用 Java 方法获取电池信息
    jmethodID method = m_env->GetStaticMethodID(m_deviceInfoClass, "getBatteryInfo", "(Landroid/content/Context;)Ljava/lang/String;");
    if (method) {
        jstring result = (jstring)m_env->CallStaticObjectMethod(m_deviceInfoClass, method, m_context);
        if (result) {
            std::string batteryInfo = jstringToString(result);
            m_env->DeleteLocalRef(result);
            
            if (callback) {
                callback("", batteryInfo);
            }
        } else {
            if (callback) {
                callback("获取电池信息失败", "");
            }
        }
    } else {
        if (callback) {
            callback("getBatteryInfo 方法未找到", "");
        }
    }
}

void AndroidDeviceInfoModule::getNetworkState(const std::vector<std::string>& args,
                                             std::function<void(const std::string&, const std::string&)> callback) {
    log("info", "获取网络状态");
    
    if (!m_deviceInfoClass) {
        if (callback) {
            callback("DeviceInfoHelper 类未找到", "");
        }
        return;
    }
    
    // 调用 Java 方法获取网络信息
    jmethodID method = m_env->GetStaticMethodID(m_deviceInfoClass, "getNetworkInfo", "(Landroid/content/Context;)Ljava/lang/String;");
    if (method) {
        jstring result = (jstring)m_env->CallStaticObjectMethod(m_deviceInfoClass, method, m_context);
        if (result) {
            std::string networkInfo = jstringToString(result);
            m_env->DeleteLocalRef(result);
            
            if (callback) {
                callback("", networkInfo);
            }
        } else {
            if (callback) {
                callback("获取网络信息失败", "");
            }
        }
    } else {
        if (callback) {
            callback("getNetworkInfo 方法未找到", "");
        }
    }
}

void AndroidDeviceInfoModule::getSystemInfo(const std::vector<std::string>& args,
                                           std::function<void(const std::string&, const std::string&)> callback) {
    log("info", "获取系统信息");
    
    Json::Value systemInfo;
    systemInfo["systemName"] = "Android";
    systemInfo["systemVersion"] = getSystemVersion();
    systemInfo["model"] = getDeviceModel();
    systemInfo["manufacturer"] = getDeviceManufacturer();
    systemInfo["hardware"] = getSystemProperty("ro.hardware");
    systemInfo["sdkVersion"] = getSystemProperty("ro.build.version.sdk");
    systemInfo["buildId"] = getSystemProperty("ro.build.id");
    
    // 通过 Java 获取更多系统信息
    if (m_deviceInfoClass) {
        jmethodID method = m_env->GetStaticMethodID(m_deviceInfoClass, "getSystemInfo", "(Landroid/content/Context;)Ljava/lang/String;");
        if (method) {
            jstring result = (jstring)m_env->CallStaticObjectMethod(m_deviceInfoClass, method, m_context);
            if (result) {
                std::string javaSystemInfo = jstringToString(result);
                m_env->DeleteLocalRef(result);
                
                // 解析并合并 Java 返回的系统信息
                Json::Value javaInfo;
                Json::Reader reader;
                if (reader.parse(javaSystemInfo, javaInfo)) {
                    for (const auto& key : javaInfo.getMemberNames()) {
                        systemInfo[key] = javaInfo[key];
                    }
                }
            }
        }
    }
    
    Json::StreamWriterBuilder builder;
    std::string jsonResult = Json::writeString(builder, systemInfo);
    
    if (callback) {
        callback("", jsonResult);
    }
}

void AndroidDeviceInfoModule::startBatteryMonitoring(const std::vector<std::string>& args,
                                                    std::function<void(const std::string&, const std::string&)> callback) {
    log("info", "开始电池监控");
    
    if (!m_deviceInfoClass) {
        if (callback) {
            callback("DeviceInfoHelper 类未找到", "");
        }
        return;
    }
    
    if (!m_isBatteryMonitoringEnabled) {
        // 调用 Java 方法开始监控
        jmethodID method = m_env->GetStaticMethodID(m_deviceInfoClass, "startBatteryMonitoring", "(Landroid/content/Context;)Z");
        if (method) {
            jboolean success = m_env->CallStaticBooleanMethod(m_deviceInfoClass, method, m_context);
            m_isBatteryMonitoringEnabled = success;
            
            if (callback) {
                callback("", success ? "true" : "false");
            }
        } else {
            if (callback) {
                callback("startBatteryMonitoring 方法未找到", "");
            }
        }
    } else {
        if (callback) {
            callback("", "true");
        }
    }
}

void AndroidDeviceInfoModule::stopBatteryMonitoring(const std::vector<std::string>& args,
                                                   std::function<void(const std::string&, const std::string&)> callback) {
    log("info", "停止电池监控");
    
    if (!m_deviceInfoClass) {
        if (callback) {
            callback("DeviceInfoHelper 类未找到", "");
        }
        return;
    }
    
    if (m_isBatteryMonitoringEnabled) {
        // 调用 Java 方法停止监控
        jmethodID method = m_env->GetStaticMethodID(m_deviceInfoClass, "stopBatteryMonitoring", "(Landroid/content/Context;)V");
        if (method) {
            m_env->CallStaticVoidMethod(m_deviceInfoClass, method, m_context);
            m_isBatteryMonitoringEnabled = false;
        }
    }
    
    if (callback) {
        callback("", "true");
    }
}

// JNI 工具方法实现

std::string AndroidDeviceInfoModule::jstringToString(jstring jstr) {
    if (!jstr || !m_env) return "";
    
    const char* chars = m_env->GetStringUTFChars(jstr, nullptr);
    std::string result(chars);
    m_env->ReleaseStringUTFChars(jstr, chars);
    
    return result;
}

jstring AndroidDeviceInfoModule::stringToJstring(const std::string& str) {
    if (!m_env) return nullptr;
    return m_env->NewStringUTF(str.c_str());
}

std::string AndroidDeviceInfoModule::getSystemProperty(const std::string& key) {
    if (!m_env) return "";
    
    // 使用 SystemProperties.get() 方法
    jclass systemPropsClass = m_env->FindClass("android/os/SystemProperties");
    if (!systemPropsClass) return "";
    
    jmethodID getMethod = m_env->GetStaticMethodID(systemPropsClass, "get", "(Ljava/lang/String;)Ljava/lang/String;");
    if (!getMethod) {
        m_env->DeleteLocalRef(systemPropsClass);
        return "";
    }
    
    jstring keyStr = stringToJstring(key);
    jstring result = (jstring)m_env->CallStaticObjectMethod(systemPropsClass, getMethod, keyStr);
    
    std::string value;
    if (result) {
        value = jstringToString(result);
        m_env->DeleteLocalRef(result);
    }
    
    m_env->DeleteLocalRef(keyStr);
    m_env->DeleteLocalRef(systemPropsClass);
    
    return value;
}

std::string AndroidDeviceInfoModule::getAndroidId() {
    if (!m_env || !m_context) return "";
    
    // 获取 Settings.Secure.ANDROID_ID
    jclass settingsClass = m_env->FindClass("android/provider/Settings$Secure");
    if (!settingsClass) return "";
    
    jmethodID getStringMethod = m_env->GetStaticMethodID(settingsClass, "getString", 
        "(Landroid/content/ContentResolver;Ljava/lang/String;)Ljava/lang/String;");
    
    if (!getStringMethod) {
        m_env->DeleteLocalRef(settingsClass);
        return "";
    }
    
    // 获取 ContentResolver
    jclass contextClass = m_env->GetObjectClass(m_context);
    jmethodID getContentResolverMethod = m_env->GetMethodID(contextClass, "getContentResolver", "()Landroid/content/ContentResolver;");
    jobject contentResolver = m_env->CallObjectMethod(m_context, getContentResolverMethod);
    
    jstring androidIdKey = stringToJstring("android_id");
    jstring result = (jstring)m_env->CallStaticObjectMethod(settingsClass, getStringMethod, contentResolver, androidIdKey);
    
    std::string androidId;
    if (result) {
        androidId = jstringToString(result);
        m_env->DeleteLocalRef(result);
    }
    
    m_env->DeleteLocalRef(androidIdKey);
    m_env->DeleteLocalRef(contentResolver);
    m_env->DeleteLocalRef(contextClass);
    m_env->DeleteLocalRef(settingsClass);
    
    return androidId;
}

std::string AndroidDeviceInfoModule::getDeviceModel() {
    return getSystemProperty("ro.product.model");
}

std::string AndroidDeviceInfoModule::getDeviceManufacturer() {
    return getSystemProperty("ro.product.manufacturer");
}

std::string AndroidDeviceInfoModule::getSystemVersion() {
    return getSystemProperty("ro.build.version.release");
}

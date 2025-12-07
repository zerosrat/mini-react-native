/**
 * Android-specific JSCExecutor implementation
 *
 * This file provides Android-specific functionality for JSCExecutor,
 * extending the common implementation with Android platform features.
 */

#include "../../common/bridge/JSCExecutor.h"
#include <android/log.h>
#include <jni.h>

#define LOG_TAG "JSCExecutorAndroid"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace mini_rn {
namespace bridge {

// Android-specific platform initialization
void JSCExecutor::platformSpecificInit() {
    LOGI("Initializing JSCExecutor for Android platform");

    // Android-specific JSC configuration
    // Note: When using org.webkit:android-jsc, most configuration is handled
    // by the library itself. We can add Android-specific optimizations here.

    LOGD("Android JSCExecutor initialization completed");
}

// Android-specific logging implementation
void JSCExecutor::platformSpecificLog(const std::string& message) {
    // Use Android's logging system for JavaScript console output
    LOGI("JS: %s", message.c_str());
}

// Android-specific error handling
void JSCExecutor::platformSpecificError(const std::string& error) {
    LOGE("JS Error: %s", error.c_str());
}

// Android-specific memory management hints
void JSCExecutor::platformSpecificMemoryWarning() {
    LOGD("Memory warning received, triggering JavaScript garbage collection");
    // TODO: Implement memory management optimizations for Android
}

} // namespace bridge
} // namespace mini_rn
/**
 * JNI implementation for JSCExecutor Java class
 *
 * This file implements the native methods declared in JSCExecutor.java,
 * providing the bridge between Java and the C++ JSCExecutor implementation.
 */

#include <jni.h>
#include <android/log.h>
#include <string>
#include <stdexcept>

// Include the common JSCExecutor header
#include "../../common/bridge/JSCExecutor.h"

#define LOG_TAG "MiniRN-JNI"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

using namespace mini_rn::bridge;

// Helper functions for JNI string conversion
namespace {
    std::string jstringToString(JNIEnv* env, jstring jstr) {
        if (!jstr) return "";
        const char* chars = env->GetStringUTFChars(jstr, nullptr);
        if (!chars) return "";
        std::string result(chars);
        env->ReleaseStringUTFChars(jstr, chars);
        return result;
    }

    jstring stringToJstring(JNIEnv* env, const std::string& str) {
        return env->NewStringUTF(str.c_str());
    }

    void throwJavaException(JNIEnv* env, const std::string& message) {
        jclass exceptionClass = env->FindClass("java/lang/RuntimeException");
        if (exceptionClass) {
            env->ThrowNew(exceptionClass, message.c_str());
        }
    }
}

extern "C" {

/**
 * Create a new JSCExecutor instance and return its pointer as a long handle
 */
JNIEXPORT jlong JNICALL
Java_com_minirn_android_JSCExecutor_createContext(JNIEnv *env, jobject thiz) {
    try {
        LOGD("Creating new JSCExecutor instance");
        auto* executor = new JSCExecutor();
        LOGD("JSCExecutor created successfully: %p", executor);
        return reinterpret_cast<jlong>(executor);
    } catch (const std::exception& e) {
        LOGE("Failed to create JSCExecutor: %s", e.what());
        throwJavaException(env, std::string("Failed to create JSCExecutor: ") + e.what());
        return 0;
    }
}

/**
 * Load JavaScript script into the executor context
 */
JNIEXPORT void JNICALL
Java_com_minirn_android_JSCExecutor_loadScript(JNIEnv *env, jobject thiz,
                                                jlong context, jstring script) {
    try {
        auto* executor = reinterpret_cast<JSCExecutor*>(context);
        if (!executor) {
            throwJavaException(env, "Invalid JSCExecutor context");
            return;
        }

        std::string scriptStr = jstringToString(env, script);
        LOGD("Loading script: %zu characters", scriptStr.length());

        executor->loadApplicationScript(scriptStr);
        LOGD("Script loaded successfully");
    } catch (const std::exception& e) {
        LOGE("Failed to load script: %s", e.what());
        throwJavaException(env, std::string("Failed to load script: ") + e.what());
    }
}

/**
 * Call a native module method
 */
JNIEXPORT jstring JNICALL
Java_com_minirn_android_JSCExecutor_callNativeMethod(JNIEnv *env, jobject thiz,
                                                      jlong context, jstring module,
                                                      jstring method, jstring args) {
    try {
        auto* executor = reinterpret_cast<JSCExecutor*>(context);
        if (!executor) {
            throwJavaException(env, "Invalid JSCExecutor context");
            return stringToJstring(env, "{\"error\": \"Invalid context\"}");
        }

        std::string moduleStr = jstringToString(env, module);
        std::string methodStr = jstringToString(env, method);
        std::string argsStr = jstringToString(env, args);

        LOGD("Calling native method: %s.%s(%s)", moduleStr.c_str(), methodStr.c_str(), argsStr.c_str());

        // TODO: Implement actual native method calling through ModuleRegistry
        // For now, return a placeholder response
        std::string result = "{\"result\": \"Method call not yet implemented\"}";

        LOGD("Native method call result: %s", result.c_str());
        return stringToJstring(env, result);
    } catch (const std::exception& e) {
        LOGE("Failed to call native method: %s", e.what());
        std::string error = "{\"error\": \"" + std::string(e.what()) + "\"}";
        return stringToJstring(env, error);
    }
}

/**
 * Register native modules with the executor
 */
JNIEXPORT void JNICALL
Java_com_minirn_android_JSCExecutor_registerModules(JNIEnv *env, jobject thiz,
                                                     jlong context, jobjectArray modules) {
    try {
        auto* executor = reinterpret_cast<JSCExecutor*>(context);
        if (!executor) {
            throwJavaException(env, "Invalid JSCExecutor context");
            return;
        }

        jsize moduleCount = env->GetArrayLength(modules);
        LOGD("Registering %d modules", moduleCount);

        // TODO: Implement actual module registration
        // For now, just log that modules would be registered
        for (jsize i = 0; i < moduleCount; i++) {
            jobject module = env->GetObjectArrayElement(modules, i);
            // Process module registration
            LOGD("Would register module at index %d", i);
            env->DeleteLocalRef(module);
        }

        LOGD("Module registration completed");
    } catch (const std::exception& e) {
        LOGE("Failed to register modules: %s", e.what());
        throwJavaException(env, std::string("Failed to register modules: ") + e.what());
    }
}

/**
 * Get the global JavaScript object
 */
JNIEXPORT jobject JNICALL
Java_com_minirn_android_JSCExecutor_getGlobalObject(JNIEnv *env, jobject thiz, jlong context) {
    try {
        auto* executor = reinterpret_cast<JSCExecutor*>(context);
        if (!executor) {
            throwJavaException(env, "Invalid JSCExecutor context");
            return nullptr;
        }

        LOGD("Getting global JavaScript object");

        // TODO: Implement actual global object access
        // For now, return null as placeholder
        return nullptr;
    } catch (const std::exception& e) {
        LOGE("Failed to get global object: %s", e.what());
        throwJavaException(env, std::string("Failed to get global object: ") + e.what());
        return nullptr;
    }
}

/**
 * Destroy the JSCExecutor context and clean up resources
 */
JNIEXPORT void JNICALL
Java_com_minirn_android_JSCExecutor_destroyContext(JNIEnv *env, jobject thiz, jlong context) {
    try {
        auto* executor = reinterpret_cast<JSCExecutor*>(context);
        if (!executor) {
            LOGD("Attempted to destroy null context");
            return;
        }

        LOGD("Destroying JSCExecutor instance: %p", executor);
        delete executor;
        LOGD("JSCExecutor destroyed successfully");
    } catch (const std::exception& e) {
        LOGE("Failed to destroy JSCExecutor: %s", e.what());
        // Don't throw exception in destructor-like function
    }
}

} // extern "C"
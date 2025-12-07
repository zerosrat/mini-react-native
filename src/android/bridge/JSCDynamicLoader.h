#include <dlfcn.h>
#include <android/log.h>
#include <string>
#include <cstddef>

namespace {
    // 动态加载 JSC 库的函数指针
    static void* (*JSContextCreate)(void*, void*) = nullptr;
    static void* (*JSContextGetGlobalObject)(void*) = nullptr;
    static void (*JSContextRelease)(void*) = nullptr;
    static void* (*JSValueMakeString)(void*, void*) = nullptr;
    static void* (*JSStringCreateWithUTF8CString)(void*, const char*) = nullptr;
    static void* (*JSObjectSetProperty)(void*, void*, void*, void*, const void*) = nullptr;
    static void* (*JSObjectCallAsFunction)(void*, void*, size_t, const void**) = nullptr;
    static void* (*JSValueToNumber)(void*, double) = nullptr;
    static void* (*JSValueToStringCopy)(void*, void*) = nullptr;
    static size_t (*JSStringGetMaximumUTF8CStringSize)(void*) = nullptr;
    static char* (*JSStringGetUTF8CString)(void*, char*, size_t) = nullptr;
    static void* (*JSValueMakeString)(void*, void*) = nullptr;
    static bool (*JSCheckScriptSyntax)(void*, void*, void**) = nullptr;

    bool loadJSCFromAPK() {
        // 尝试从 APK 中加载 JSC 库
        void* handle = dlopen("libjsc.so", RTLD_NOW | RTLD_GLOBAL);
        if (!handle) {
            ALOGE("Failed to load libjsc.so: %s", dlerror());
            return false;
        }

        // 清除之前的错误
        dlerror();

        // 加载需要的符号
        JSContextCreate = (void* (*)(JSGlobalContextRef, JSGlobalContextClassRef))
                dlsym(handle, "JSGlobalContextCreate");
        JSContextGetGlobalObject = (JSObjectRef (*)(JSContextRef))
                dlsym(handle, "JSContextGetGlobalObject");
        JSContextRelease = (void (*)(JSGlobalContextRef))
                dlsym(handle, "JSGlobalContextRelease");
        JSStringCreateWithUTF8CString = (JSStringRef (*)(JSContextRef, const char*))
                dlsym(handle, "JSStringCreateWithUTF8CString");
        JSObjectSetProperty = (JSObjectRef (*)(JSContextRef, JSObjectRef, JSStringRef, JSValueRef, const JSPropertyAttributes*))
                dlsym(handle, "JSObjectSetProperty");
        JSObjectCallAsFunction = (JSObjectRef (*)(JSContextRef, JSValueRef, size_t, const JSValueRef[]))
                dlsym(handle, "JSObjectCallAsFunction");
        JSValueToNumber = (JSValueRef (*)(JSContextRef, double))
                dlsym(handle, "JSValueToNumber");
        JSValueToStringCopy = (JSStringRef (*)(JSContextRef, JSValueRef))
                dlsym(handle, "JSValueToStringCopy");
        JSStringGetMaximumUTF8CStringSize = (size_t (*)(JSStringRef))
                dlsym(handle, "JSStringGetMaximumUTF8CStringSize");
        JSStringGetUTF8CString = (char* (*)(JSStringRef, char*, size_t))
                dlsym(handle, "JSStringGetUTF8CString");
        JSValueMakeString = (JSValueRef (*)(JSContextRef, JSStringRef))
                dlsym(handle, "JSValueMakeString");
        JSCheckScriptSyntax = (bool (*)(JSContextRef, JSStringRef, JSValueRef*))
                dlsym(handle, "JSCheckScriptSyntax");

        // 检查所有符号是否加载成功
        if (!JSContextCreate || !JSContextGetGlobalObject || !JSContextRelease ||
            !JSStringCreateWithUTF8CString || !JSObjectSetProperty ||
            !JSObjectCallAsFunction || !JSValueToNumber ||
            !JSValueToStringCopy || !JSStringGetMaximumUTF8CStringSize ||
            !JSStringGetUTF8CString || !JSValueMakeString ||
            !JSCheckScriptSyntax) {
            ALOGE("Failed to resolve all JSC symbols");
            dlclose(handle);
            return false;
        }

        ALOGI("Successfully loaded JSC library from APK");
        return true;
    }
}
package com.minirn.android;

import android.util.Log;

/**
 * JSCExecutor - Java wrapper for native JavaScriptCore functionality
 *
 * This class provides Java interface to the native C++ JSCExecutor implementation,
 * enabling JavaScript execution and native module communication from Android.
 */
public class JSCExecutor {
    private static final String TAG = "JSCExecutor";
    private long nativeContext;
    private static boolean libraryLoaded = false;

    static {
        try {
            System.loadLibrary("mini_react_native");
            libraryLoaded = true;
            Log.i(TAG, "Native library mini_react_native loaded successfully");
        } catch (UnsatisfiedLinkError e) {
            Log.e(TAG, "Failed to load native library mini_react_native.so", e);
            libraryLoaded = false;
        }
    }

    /**
     * Create native JavaScript execution context
     * @return native context handle
     */
    public JSCExecutor() {
        if (!libraryLoaded) {
            throw new RuntimeException("Native library not loaded. Cannot create context.");
        }
        nativeContext = createContext();
    }

    /**
     * Load JavaScript bundle into the context
     * @param script JavaScript code to execute
     */
    public void loadScript(String script) {
        if (script == null) {
            throw new IllegalArgumentException("Script cannot be null");
        }
        if (!libraryLoaded) {
            Log.e(TAG, "Cannot load script: native library not loaded");
            return;
        }
        loadScript(nativeContext, script);
    }

    /**
     * Call a native module method from JavaScript
     * @param module Module name
     * @param method Method name
     * @param args JSON arguments string
     * @return JSON result string
     */
    public String callNativeMethod(String module, String method, String args) {
        if (module == null || method == null) {
            throw new IllegalArgumentException("Module and method cannot be null");
        }
        if (!libraryLoaded) {
            Log.e(TAG, "Cannot call native method: native library not loaded");
            return "{\"error\": \"Native library not loaded\"}";
        }
        return callNativeMethod(nativeContext, module, method, args);
    }

    /**
     * Register native modules with the executor
     * @param modules Array of module configurations
     */
    public void registerModules(Object[] modules) {
        if (modules == null) {
            throw new IllegalArgumentException("Modules array cannot be null");
        }
        if (!libraryLoaded) {
            Log.e(TAG, "Cannot register modules: native library not loaded");
            return;
        }
        registerModules(nativeContext, modules);
    }

    /**
     * Get the global JavaScript object
     * @return Global object reference
     */
    public Object getGlobalObject() {
        if (!libraryLoaded) {
            Log.e(TAG, "Cannot get global object: native library not loaded");
            return null;
        }
        return getGlobalObject(nativeContext);
    }

    /**
     * Destroy the native context and clean up resources
     */
    public void destroy() {
        if (!libraryLoaded) {
            Log.e(TAG, "Cannot destroy context: native library not loaded");
            return;
        }
        if (nativeContext != 0) {
            destroyContext(nativeContext);
            nativeContext = 0;
        }
    }

    // Native methods
    private native long createContext();
    private native void loadScript(long context, String script);
    private native String callNativeMethod(long context, String module, String method, String args);
    private native void registerModules(long context, Object[] modules);
    private native Object getGlobalObject(long context);
    private native void destroyContext(long context);

    @Override
    protected void finalize() throws Throwable {
        try {
            destroy();
        } finally {
            super.finalize();
        }
    }
}
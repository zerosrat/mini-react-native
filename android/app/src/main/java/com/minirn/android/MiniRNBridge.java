package com.minirn.android;

import android.content.Context;
import android.util.Log;

/**
 * MiniRNBridge - Bridge lifecycle manager for Mini React Native
 *
 * This class manages the lifecycle of the JavaScript execution environment,
 * including module registration, script loading, and resource cleanup.
 */
public class MiniRNBridge {
    private static final String TAG = "MiniRNBridge";
    private JSCExecutor executor;
    private boolean isInitialized = false;
    private Context applicationContext;

    public MiniRNBridge(Context context) {
        this.applicationContext = context.getApplicationContext();
        this.executor = new JSCExecutor();
    }

    /**
     * Initialize the bridge with native modules
     */
    public void initialize() {
        if (isInitialized) {
            Log.w(TAG, "Bridge already initialized");
            return;
        }

        try {
            Log.i(TAG, "Initializing MiniRN Bridge...");

            // Register native modules
            Object[] modules = getNativeModules();
            executor.registerModules(modules);

            isInitialized = true;
            Log.i(TAG, "Bridge initialization completed successfully");

        } catch (Exception e) {
            Log.e(TAG, "Failed to initialize bridge", e);
            throw new RuntimeException("Bridge initialization failed", e);
        }
    }

    /**
     * Load JavaScript bundle
     * @param scriptPath Path to JavaScript bundle
     */
    public void loadScript(String scriptPath) {
        if (!isInitialized) {
            throw new IllegalStateException("Bridge not initialized");
        }

        try {
            Log.i(TAG, "Loading JavaScript bundle: " + scriptPath);

            // For now, load a simple test script
            // In production, this would load from assets
            String script = getTestScript();
            executor.loadScript(script);

            Log.i(TAG, "JavaScript bundle loaded successfully");

        } catch (Exception e) {
            Log.e(TAG, "Failed to load JavaScript bundle", e);
            throw new RuntimeException("Script loading failed", e);
        }
    }

    /**
     * Call a native module method
     * @param module Module name
     * @param method Method name
     * @param args JSON arguments
     * @return JSON result
     */
    public String callNativeMethod(String module, String method, String args) {
        if (!isInitialized) {
            throw new IllegalStateException("Bridge not initialized");
        }

        try {
            Log.d(TAG, String.format("Calling %s.%s(%s)", module, method, args));
            String result = executor.callNativeMethod(module, method, args);
            Log.d(TAG, String.format("Result: %s", result));
            return result;

        } catch (Exception e) {
            Log.e(TAG, String.format("Failed to call %s.%s", module, method), e);
            throw new RuntimeException("Native method call failed", e);
        }
    }

    /**
     * Get array of native modules to register
     * @return Array of module configurations
     */
    private Object[] getNativeModules() {
        // For now, return empty array
        // Will be populated with actual modules later
        return new Object[0];
    }

    /**
     * Get test JavaScript script for development
     * @return Simple test script
     */
    private String getTestScript() {
        return "console.log('Mini React Native Bridge initialized successfully!');";
    }

    /**
     * Check if bridge is initialized
     * @return initialization status
     */
    public boolean isInitialized() {
        return isInitialized;
    }

    /**
     * Cleanup resources
     */
    public void destroy() {
        if (executor != null) {
            try {
                executor.destroy();
                Log.i(TAG, "Bridge destroyed successfully");
            } catch (Exception e) {
                Log.e(TAG, "Error during bridge cleanup", e);
            } finally {
                executor = null;
                isInitialized = false;
            }
        }
    }

    @Override
    protected void finalize() throws Throwable {
        try {
            destroy();
        } finally {
            super.finalize();
        }
    }
}
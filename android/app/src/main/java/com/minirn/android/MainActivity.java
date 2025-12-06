package com.minirn.android;

import androidx.appcompat.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.widget.TextView;
import android.widget.ScrollView;

/**
 * MainActivity - Demo application for Mini React Native
 *
 * This activity demonstrates the Android integration of Mini React Native
 * by initializing the bridge and executing JavaScript code.
 */
public class MainActivity extends AppCompatActivity {
    private static final String TAG = "MainActivity";
    private MiniRNBridge bridge;
    private TextView outputText;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        outputText = findViewById(R.id.output_text);
        ScrollView scrollView = findViewById(R.id.scroll_view);

        try {
            Log.i(TAG, "Initializing Mini React Native bridge...");

            // Initialize the bridge
            bridge = new MiniRNBridge(this);
            bridge.initialize();

            // Load and execute JavaScript
            bridge.loadScript("demo.js");

            // Test native module call
            testNativeModuleCall();

            Log.i(TAG, "Mini React Native demo completed successfully");

        } catch (Exception e) {
            Log.e(TAG, "Failed to initialize Mini React Native", e);
            updateOutput("Error: " + e.getMessage());
        }
    }

    /**
     * Test native module functionality
     */
    private void testNativeModuleCall() {
        try {
            // This will test the DeviceInfo module when implemented
            String result = bridge.callNativeMethod("DeviceInfo", "getUniqueId", "[]");
            updateOutput("Device Unique ID: " + result);

        } catch (Exception e) {
            Log.e(TAG, "Native module call failed", e);
            updateOutput("Native call failed: " + e.getMessage());
        }
    }

    /**
     * Update output display
     * @param message Message to display
     */
    private void updateOutput(String message) {
        runOnUiThread(() -> {
            String currentText = outputText.getText().toString();
            String newText = currentText + "\n" + message;
            outputText.setText(newText);
        });
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        if (bridge != null) {
            try {
                bridge.destroy();
                Log.i(TAG, "Bridge destroyed successfully");
            } catch (Exception e) {
                Log.e(TAG, "Error destroying bridge", e);
            }
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        Log.i(TAG, "Activity resumed");
    }

    @Override
    protected void onPause() {
        super.onPause();
        Log.i(TAG, "Activity paused");
    }
}
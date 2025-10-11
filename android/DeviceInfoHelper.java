/**
 * DeviceInfoHelper.java
 * Android 设备信息辅助类 - 提供 JNI 调用的 Java 方法
 */

package com.bridge;

import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.BroadcastReceiver;
import android.os.BatteryManager;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.telephony.TelephonyManager;
import android.util.DisplayMetrics;
import android.view.WindowManager;
import android.os.Build;
import android.app.ActivityManager;
import android.os.StatFs;
import android.os.Environment;
import org.json.JSONObject;
import org.json.JSONException;

/**
 * 设备信息辅助类
 * 为 Native 层提供 Android 系统信息访问
 */
public class DeviceInfoHelper {
    
    private static BroadcastReceiver batteryReceiver = null;
    private static boolean isBatteryMonitoringEnabled = false;
    
    /**
     * 获取屏幕信息
     */
    public static String getScreenInfo(Context context) {
        try {
            WindowManager wm = (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);
            DisplayMetrics metrics = new DisplayMetrics();
            wm.getDefaultDisplay().getMetrics(metrics);
            
            JSONObject screenInfo = new JSONObject();
            screenInfo.put("width", metrics.widthPixels);
            screenInfo.put("height", metrics.heightPixels);
            screenInfo.put("density", metrics.density);
            screenInfo.put("densityDpi", metrics.densityDpi);
            screenInfo.put("scaledDensity", metrics.scaledDensity);
            screenInfo.put("xdpi", metrics.xdpi);
            screenInfo.put("ydpi", metrics.ydpi);
            
            return screenInfo.toString();
        } catch (JSONException e) {
            return "{}";
        }
    }
    
    /**
     * 获取电池信息
     */
    public static String getBatteryInfo(Context context) {
        try {
            IntentFilter ifilter = new IntentFilter(Intent.ACTION_BATTERY_CHANGED);
            Intent batteryStatus = context.registerReceiver(null, ifilter);
            
            if (batteryStatus == null) {
                return "{}";
            }
            
            // 电池电量 (0-100)
            int level = batteryStatus.getIntExtra(BatteryManager.EXTRA_LEVEL, -1);
            int scale = batteryStatus.getIntExtra(BatteryManager.EXTRA_SCALE, -1);
            float batteryPct = level * 100f / (float) scale;
            
            // 充电状态
            int status = batteryStatus.getIntExtra(BatteryManager.EXTRA_STATUS, -1);
            boolean isCharging = status == BatteryManager.BATTERY_STATUS_CHARGING ||
                               status == BatteryManager.BATTERY_STATUS_FULL;
            
            // 充电方式
            int chargePlug = batteryStatus.getIntExtra(BatteryManager.EXTRA_PLUGGED, -1);
            boolean usbCharge = chargePlug == BatteryManager.BATTERY_PLUGGED_USB;
            boolean acCharge = chargePlug == BatteryManager.BATTERY_PLUGGED_AC;
            boolean wirelessCharge = false;
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR1) {
                wirelessCharge = chargePlug == BatteryManager.BATTERY_PLUGGED_WIRELESS;
            }
            
            // 电池健康状态
            int health = batteryStatus.getIntExtra(BatteryManager.EXTRA_HEALTH, -1);
            
            // 电池温度 (摄氏度)
            int temperature = batteryStatus.getIntExtra(BatteryManager.EXTRA_TEMPERATURE, -1);
            float tempCelsius = temperature / 10.0f;
            
            // 电池电压 (毫伏)
            int voltage = batteryStatus.getIntExtra(BatteryManager.EXTRA_VOLTAGE, -1);
            
            JSONObject batteryInfo = new JSONObject();
            batteryInfo.put("level", batteryPct);
            batteryInfo.put("isCharging", isCharging);
            batteryInfo.put("usbCharge", usbCharge);
            batteryInfo.put("acCharge", acCharge);
            batteryInfo.put("wirelessCharge", wirelessCharge);
            batteryInfo.put("health", health);
            batteryInfo.put("temperature", tempCelsius);
            batteryInfo.put("voltage", voltage);
            batteryInfo.put("status", status);
            
            return batteryInfo.toString();
        } catch (JSONException e) {
            return "{}";
        }
    }
    
    /**
     * 获取网络信息
     */
    public static String getNetworkInfo(Context context) {
        try {
            ConnectivityManager connectivityManager = 
                (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
            
            NetworkInfo activeNetworkInfo = connectivityManager.getActiveNetworkInfo();
            
            JSONObject networkInfo = new JSONObject();
            
            if (activeNetworkInfo == null) {
                networkInfo.put("type", "none");
                networkInfo.put("isConnected", false);
                return networkInfo.toString();
            }
            
            boolean isConnected = activeNetworkInfo.isConnected();
            int networkType = activeNetworkInfo.getType();
            String typeName = activeNetworkInfo.getTypeName();
            String subTypeName = activeNetworkInfo.getSubtypeName();
            
            networkInfo.put("isConnected", isConnected);
            networkInfo.put("type", getNetworkTypeName(networkType));
            networkInfo.put("typeName", typeName);
            networkInfo.put("subTypeName", subTypeName);
            
            // 如果是移动网络，获取更详细的信息
            if (networkType == ConnectivityManager.TYPE_MOBILE) {
                TelephonyManager telephonyManager = 
                    (TelephonyManager) context.getSystemService(Context.TELEPHONY_SERVICE);
                
                int networkClass = getNetworkClass(telephonyManager.getNetworkType());
                networkInfo.put("networkGeneration", getNetworkGeneration(networkClass));
                networkInfo.put("carrierName", telephonyManager.getNetworkOperatorName());
                networkInfo.put("countryCode", telephonyManager.getNetworkCountryIso());
            }
            
            return networkInfo.toString();
        } catch (JSONException e) {
            return "{}";
        }
    }
    
    /**
     * 获取系统信息
     */
    public static String getSystemInfo(Context context) {
        try {
            JSONObject systemInfo = new JSONObject();
            
            // 内存信息
            ActivityManager activityManager = 
                (ActivityManager) context.getSystemService(Context.ACTIVITY_SERVICE);
            ActivityManager.MemoryInfo memoryInfo = new ActivityManager.MemoryInfo();
            activityManager.getMemoryInfo(memoryInfo);
            
            systemInfo.put("totalMemory", memoryInfo.totalMem);
            systemInfo.put("availableMemory", memoryInfo.availMem);
            systemInfo.put("isLowMemory", memoryInfo.lowMemory);
            systemInfo.put("memoryThreshold", memoryInfo.threshold);
            
            // 存储信息
            StatFs internalStat = new StatFs(Environment.getDataDirectory().getPath());
            long internalTotal = internalStat.getTotalBytes();
            long internalFree = internalStat.getFreeBytes();
            
            systemInfo.put("internalStorageTotal", internalTotal);
            systemInfo.put("internalStorageFree", internalFree);
            
            // 外部存储信息
            if (Environment.getExternalStorageState().equals(Environment.MEDIA_MOUNTED)) {
                StatFs externalStat = new StatFs(Environment.getExternalStorageDirectory().getPath());
                long externalTotal = externalStat.getTotalBytes();
                long externalFree = externalStat.getFreeBytes();
                
                systemInfo.put("externalStorageTotal", externalTotal);
                systemInfo.put("externalStorageFree", externalFree);
            }
            
            // CPU 信息
            systemInfo.put("cpuAbi", Build.CPU_ABI);
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
                systemInfo.put("supportedAbis", Build.SUPPORTED_ABIS);
            }
            
            // 系统版本信息
            systemInfo.put("sdkInt", Build.VERSION.SDK_INT);
            systemInfo.put("release", Build.VERSION.RELEASE);
            systemInfo.put("codename", Build.VERSION.CODENAME);
            systemInfo.put("incremental", Build.VERSION.INCREMENTAL);
            
            // 设备信息
            systemInfo.put("board", Build.BOARD);
            systemInfo.put("bootloader", Build.BOOTLOADER);
            systemInfo.put("brand", Build.BRAND);
            systemInfo.put("device", Build.DEVICE);
            systemInfo.put("display", Build.DISPLAY);
            systemInfo.put("fingerprint", Build.FINGERPRINT);
            systemInfo.put("hardware", Build.HARDWARE);
            systemInfo.put("host", Build.HOST);
            systemInfo.put("id", Build.ID);
            systemInfo.put("manufacturer", Build.MANUFACTURER);
            systemInfo.put("model", Build.MODEL);
            systemInfo.put("product", Build.PRODUCT);
            systemInfo.put("serial", Build.SERIAL);
            systemInfo.put("tags", Build.TAGS);
            systemInfo.put("type", Build.TYPE);
            systemInfo.put("user", Build.USER);
            
            return systemInfo.toString();
        } catch (JSONException e) {
            return "{}";
        }
    }
    
    /**
     * 开始电池监控
     */
    public static boolean startBatteryMonitoring(Context context) {
        if (isBatteryMonitoringEnabled) {
            return true;
        }
        
        batteryReceiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                // 这里可以通过 JNI 回调到 C++ 层
                // 发送电池状态变化事件
                String batteryInfo = getBatteryInfo(context);
                // nativeBatteryLevelChanged(batteryInfo);
            }
        };
        
        IntentFilter filter = new IntentFilter();
        filter.addAction(Intent.ACTION_BATTERY_CHANGED);
        filter.addAction(Intent.ACTION_BATTERY_LOW);
        filter.addAction(Intent.ACTION_BATTERY_OKAY);
        filter.addAction(Intent.ACTION_POWER_CONNECTED);
        filter.addAction(Intent.ACTION_POWER_DISCONNECTED);
        
        context.registerReceiver(batteryReceiver, filter);
        isBatteryMonitoringEnabled = true;
        
        return true;
    }
    
    /**
     * 停止电池监控
     */
    public static void stopBatteryMonitoring(Context context) {
        if (isBatteryMonitoringEnabled && batteryReceiver != null) {
            context.unregisterReceiver(batteryReceiver);
            batteryReceiver = null;
            isBatteryMonitoringEnabled = false;
        }
    }
    
    // 工具方法
    
    private static String getNetworkTypeName(int networkType) {
        switch (networkType) {
            case ConnectivityManager.TYPE_WIFI:
                return "wifi";
            case ConnectivityManager.TYPE_MOBILE:
                return "mobile";
            case ConnectivityManager.TYPE_ETHERNET:
                return "ethernet";
            case ConnectivityManager.TYPE_BLUETOOTH:
                return "bluetooth";
            default:
                return "unknown";
        }
    }
    
    private static int getNetworkClass(int networkType) {
        switch (networkType) {
            case TelephonyManager.NETWORK_TYPE_GPRS:
            case TelephonyManager.NETWORK_TYPE_EDGE:
            case TelephonyManager.NETWORK_TYPE_CDMA:
            case TelephonyManager.NETWORK_TYPE_1xRTT:
            case TelephonyManager.NETWORK_TYPE_IDEN:
                return 1; // 2G
                
            case TelephonyManager.NETWORK_TYPE_UMTS:
            case TelephonyManager.NETWORK_TYPE_EVDO_0:
            case TelephonyManager.NETWORK_TYPE_EVDO_A:
            case TelephonyManager.NETWORK_TYPE_HSDPA:
            case TelephonyManager.NETWORK_TYPE_HSUPA:
            case TelephonyManager.NETWORK_TYPE_HSPA:
            case TelephonyManager.NETWORK_TYPE_EVDO_B:
            case TelephonyManager.NETWORK_TYPE_EHRPD:
            case TelephonyManager.NETWORK_TYPE_HSPAP:
                return 2; // 3G
                
            case TelephonyManager.NETWORK_TYPE_LTE:
                return 3; // 4G
                
            default:
                return 0; // Unknown
        }
    }
    
    private static String getNetworkGeneration(int networkClass) {
        switch (networkClass) {
            case 1: return "2G";
            case 2: return "3G";
            case 3: return "4G";
            default: return "Unknown";
        }
    }
    
    // JNI 回调方法 (需要在 C++ 中实现对应的 native 方法)
    // private static native void nativeBatteryLevelChanged(String batteryInfo);
}

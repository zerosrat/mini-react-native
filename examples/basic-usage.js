/**
 * basic-usage.js
 * 基本使用示例 - 演示如何使用 Bridge 进行 JS 和 Native 通信
 */

// 导入我们的 Bridge 组件
const { Bridge, bridge, bridgeReady } = require('../js/Bridge');
const { NativeModules, DeviceInfo, AsyncStorage, UIManager } = require('../js/NativeModules');

/**
 * 示例 1: 基本的 Native 方法调用
 */
async function basicNativeCall() {
    console.log('=== 示例 1: 基本的 Native 方法调用 ===');
    
    try {
        // 等待 Bridge 就绪
        await bridgeReady;
        console.log('Bridge 已就绪');
        
        // 调用设备信息模块的方法
        const deviceId = await DeviceInfo.getDeviceId();
        console.log('设备 ID:', deviceId);
        
        const batteryLevel = await DeviceInfo.getBatteryLevel();
        console.log('电池电量:', batteryLevel);
        
        const rtn = await AsyncStorage.clear();
        console.log('AsyncStorage.clear:', rtn);
        
        // const systemInfo = await DeviceInfo.getSystemInfo();
        // console.log('系统信息:', systemInfo);
        
    } catch (error) {
        console.error('调用失败:', error);
    }
}

/**
 * 示例 2: 异步存储操作
 */
async function asyncStorageExample() {
    console.log('=== 示例 2: 异步存储操作 ===');
    
    try {
        // 存储数据
        await AsyncStorage.setItem('user_name', 'React Native Bridge Demo');
        console.log('数据已存储');
        
        // 读取数据
        const userName = await AsyncStorage.getItem('user_name');
        console.log('读取的数据:', userName);
        
        // 获取所有 key
        const allKeys = await AsyncStorage.getAllKeys();
        console.log('所有 key:', allKeys);
        
        // 删除数据
        await AsyncStorage.removeItem('user_name');
        console.log('数据已删除');
        
    } catch (error) {
        console.error('存储操作失败:', error);
    }
}

/**
 * 示例 3: UI 交互
 */
async function uiInteractionExample() {
    console.log('=== 示例 3: UI 交互 ===');
    
    try {
        // 显示 Toast
        await UIManager.showToast('Hello from JavaScript!');
        
        // 显示加载中
        await UIManager.showLoading('Processing...');
        
        // 模拟一些异步操作
        setTimeout(async () => {
            await UIManager.hideLoading();
            await UIManager.showAlert('操作完成', '异步操作已完成！');
        }, 2000);
        
    } catch (error) {
        console.error('UI 操作失败:', error);
    }
}

/**
 * 示例 4: 事件监听
 */
function eventListeningExample() {
    console.log('=== 示例 4: 事件监听 ===');
    
    // 监听电池电量变化事件
    bridge.addEventListener('batteryLevelChanged', (data) => {
        console.log('电池电量变化:', data);
    });
    
    // 开始电池监控
    DeviceInfo.startBatteryMonitoring()
        .then(() => {
            console.log('电池监控已开始');
            
            // 10 秒后停止监控
            setTimeout(async () => {
                await DeviceInfo.stopBatteryMonitoring();
                console.log('电池监控已停止');
            }, 10000);
        })
        .catch(error => {
            console.error('启动电池监控失败:', error);
        });
}

/**
 * 示例 5: 双向通信通道
 */
function bidirectionalChannelExample() {
    console.log('=== 示例 5: 双向通信通道 ===');
    
    // 创建一个通信通道
    const dataChannel = bridge.createChannel('dataSync');
    
    // 监听来自 Native 的消息
    dataChannel.onMessage((data) => {
        console.log('收到 Native 消息:', data);
        
        // 回复消息
        dataChannel.send({
            type: 'reply',
            message: 'Message received by JavaScript',
            timestamp: Date.now()
        });
    });
    
    // 发送初始消息到 Native
    dataChannel.send({
        type: 'greeting',
        message: 'Hello from JavaScript!',
        timestamp: Date.now()
    });
    
    // 5 秒后关闭通道
    setTimeout(() => {
        dataChannel.close();
        console.log('通信通道已关闭');
    }, 5000);
}

/**
 * 示例 6: 错误处理和调试
 */
async function errorHandlingExample() {
    console.log('=== 示例 6: 错误处理和调试 ===');
    
    try {
        // 尝试调用不存在的方法
        await bridge.callNative('NonExistentModule', 'nonExistentMethod', []);
    } catch (error) {
        console.error('预期的错误:', error.message);
    }
    
    // 获取 Bridge 状态信息
    const bridgeStatus = bridge.getStatus();
    console.log('Bridge 状态:', bridgeStatus);
    
    // 获取 NativeModules 调试信息
    const debugInfo = NativeModules.getDebugInfo();
    console.log('NativeModules 调试信息:', debugInfo);
}

/**
 * 示例 7: 批量操作
 */
async function batchOperationsExample() {
    console.log('=== 示例 7: 批量操作 ===');
    
    try {
        // 并发执行多个 Native 调用
        const promises = [
            DeviceInfo.getDeviceId(),
            DeviceInfo.getBatteryLevel(),
            DeviceInfo.getNetworkState()
        ];
        
        const results = await Promise.all(promises);
        console.log('批量操作结果:', results);
        
        // 批量存储操作
        const storagePromises = [
            AsyncStorage.setItem('key1', 'value1'),
            AsyncStorage.setItem('key2', 'value2'),
            AsyncStorage.setItem('key3', 'value3')
        ];
        
        await Promise.all(storagePromises);
        console.log('批量存储完成');
        
        // 批量读取
        const keys = await AsyncStorage.getAllKeys();
        console.log('存储的所有 key:', keys);
        
    } catch (error) {
        console.error('批量操作失败:', error);
    }
}

/**
 * 主函数 - 运行所有示例
 */
async function runAllExamples() {
    console.log('开始运行 React Native Bridge 示例...\n');
    
    // 按顺序运行示例
    await basicNativeCall();
    console.log('\n');
    
    await asyncStorageExample();
    console.log('\n');
    
    await uiInteractionExample();
    console.log('\n');
    
    eventListeningExample();
    console.log('\n');
    
    bidirectionalChannelExample();
    console.log('\n');
    
    await errorHandlingExample();
    console.log('\n');
    
    await batchOperationsExample();
    console.log('\n');
    
    console.log('所有示例运行完成！');
}

// 模块导出
module.exports = {
    basicNativeCall,
    asyncStorageExample,
    uiInteractionExample,
    eventListeningExample,
    bidirectionalChannelExample,
    errorHandlingExample,
    batchOperationsExample,
    runAllExamples
};

// 如果直接运行此文件，执行所有示例
if (require.main === module) {
    runAllExamples().catch(console.error);
}

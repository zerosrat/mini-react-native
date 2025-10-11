/**
 * index.js
 * React Native Bridge 主入口文件
 */

// 导出核心模块
const { Bridge, bridge, bridgeReady } = require('./js/Bridge');
const { NativeModules, DeviceInfo, AsyncStorage, NetworkManager, UIManager } = require('./js/NativeModules');
const MessageQueue = require('./js/MessageQueue');

// 导出示例和工具
const examples = require('./examples/basic-usage');

/**
 * 主要的 Bridge 接口
 */
const ReactNativeBridge = {
    // 核心组件
    Bridge,
    bridge,
    bridgeReady,
    NativeModules,
    MessageQueue,
    
    // 预定义模块
    DeviceInfo,
    AsyncStorage,
    NetworkManager,
    UIManager,
    
    // 工具方法
    registerModule: (name, config) => NativeModules.registerModule(name, config),
    callNativeMethod: (module, method, ...args) => NativeModules.callMethod(module, method, ...args),
    
    // 示例
    examples,
    
    // 版本信息
    version: '1.0.0',
    
    // 调试工具
    debug: {
        getBridgeStatus: () => bridge.getStatus(),
        getNativeModulesInfo: () => NativeModules.getDebugInfo(),
        enableVerboseLogging: () => {
            global.__BRIDGE_DEBUG = true;
            console.log('[Bridge] 详细日志已启用');
        },
        disableVerboseLogging: () => {
            global.__BRIDGE_DEBUG = false;
            console.log('[Bridge] 详细日志已禁用');
        }
    }
};

// 在全局对象上挂载 (可选)
if (typeof global !== 'undefined') {
    global.ReactNativeBridge = ReactNativeBridge;
}

// 导出模块
module.exports = ReactNativeBridge;

// 如果直接运行此文件，执行演示
if (require.main === module) {
    console.log('🚀 React Native Bridge 传统架构演示');
    console.log('==========================================');
    console.log('');
    
    console.log('📚 这是一个教学演示项目，展示了传统 React Native Bridge 的工作原理');
    console.log('');
    console.log('🏗️  架构层次:');
    console.log('   JavaScript Layer (React Native)');
    console.log('         ↕');
    console.log('   JavaScript Bridge (MessageQueue)');
    console.log('         ↕');
    console.log('   C++ Bridge Layer (JSCExecutor)');
    console.log('         ↕');
    console.log('   Native Module (iOS/Android)');
    console.log('');
    
    console.log('📖 使用说明:');
    console.log('   1. 查看 examples/ 目录的示例代码');
    console.log('   2. 打开 examples/bridge-demo.html 进行交互演示');
    console.log('   3. 阅读 docs/ 目录的详细文档');
    console.log('   4. 运行 `make demo` 启动演示服务器');
    console.log('');
    
    console.log('🔧 开发命令:');
    console.log('   make all       - 完整构建和测试');
    console.log('   make demo      - 启动演示服务器'); 
    console.log('   make test      - 运行测试');
    console.log('   make help      - 查看所有命令');
    console.log('');
    
    // 运行基础演示
    bridgeReady.then(() => {
        console.log('✅ Bridge 已就绪，开始运行基础演示...');
        console.log('');
        
        // 运行一个简单的演示
        examples.basicNativeCall().then(() => {
            console.log('');
            console.log('🎉 基础演示完成！');
            console.log('📝 更多示例请查看 examples/basic-usage.js');
            console.log('🌐 或者运行 `make demo` 打开交互式演示');
        }).catch(error => {
            console.error('❌ 演示运行失败:', error);
        });
    }).catch(error => {
        console.error('❌ Bridge 初始化失败:', error);
    });
}

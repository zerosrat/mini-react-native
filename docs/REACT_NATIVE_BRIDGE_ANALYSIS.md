# React Native Bridge 调用链路分析

## 概述

本文档详细记录了 RN EventEmitter 的 Native 到 JS 和 JS 到 Native 的完整调用链路，包含具体的文件路径和行号信息，便于代码追踪和调试。

---

## 一、Native 到 JS 事件传递链路

### 1. Native EventEmitter 层

#### RCTAppState 示例 (`React/Modules/RCTAppState.m`)

**继承关系：**
```objective-c
@interface RCTAppState : RCTEventEmitter  // Line 33
```

**支持的事件类型：**
```objective-c
- (NSArray<NSString *> *)supportedEvents {  // Line 57
  return @[@"appStateDidChange", @"memoryWarning"];  // Line 59
}
```

**开始监听原生事件：**
```objective-c
- (void)startObserving {  // Line 62
  for (NSString *name in @[UIApplicationDidBecomeActiveNotification,  // Line 64
                           UIApplicationDidEnterBackgroundNotification,  // Line 65
                           UIApplicationDidFinishLaunchingNotification,  // Line 66
                           UIApplicationWillResignActiveNotification,  // Line 67
                           UIApplicationWillEnterForegroundNotification]) {  // Line 68
    [[NSNotificationCenter defaultCenter] addObserver:self  // Line 70
                                             selector:@selector(handleAppStateDidChange:)  // Line 71
                                                 name:name  // Line 72
                                               object:nil];  // Line 73
  }
}
```

**事件处理和发送：**
```objective-c
- (void)handleAppStateDidChange:(NSNotification *)notification {  // Line 94
  NSString *newState = RCTCurrentAppBackgroundState();  // Line 103

  if (![newState isEqualToString:_lastKnownState]) {  // Line 106
    _lastKnownState = newState;  // Line 107
    [self sendEventWithName:@"appStateDidChange"  // Line 108
                       body:@{@"app_state": _lastKnownState}];  // Line 109
  }
}
```

### 2. RCTEventEmitter 基类层

#### 核心发送机制 (`React/Modules/RCTEventEmitter.m`)

```objective-c
- (void)sendEventWithName:(NSString *)eventName body:(id)body {  // Line 36
  RCTAssert(_bridge != nil, @"Bridge is not set");  // Line 38-41

  if (RCT_DEBUG && ![[self supportedEvents] containsObject:eventName]) {  // Line 43
    RCTLogError(@"`%@` is not a supported event type", eventName);  // Line 44-45
  }

  if (_listenerCount > 0) {  // Line 47
    [_bridge enqueueJSCall:@"RCTDeviceEventEmitter"  // Line 48
                    method:@"emit"  // Line 49
                      args:body ? @[eventName, body] : @[eventName]  // Line 50
                completion:NULL];  // Line 51
  } else {
    RCTLogWarn(@"Sending `%@` with no listeners registered.", eventName);  // Line 53
  }
}
```

#### 监听器生命周期管理：

```objective-c
RCT_EXPORT_METHOD(addListener:(NSString *)eventName) {  // Line 74
  _listenerCount++;  // Line 80
  if (_listenerCount == 1) {  // Line 81
    [self startObserving];  // Line 82
  }
}

RCT_EXPORT_METHOD(removeListeners:(double)count) {  // Line 86
  _listenerCount = MAX(_listenerCount - currentCount, 0);  // Line 92
  if (_listenerCount == 0) {  // Line 93
    [self stopObserving];  // Line 94
  }
}
```

### 3. Bridge 代理层

#### RCTBridge 代理 (`React/Base/RCTBridge.m`)

```objective-c
- (Class)bridgeClass {  // Line 未确定具体行号
  return [RCTCxxBridge class];
}

- (void)enqueueJSCall:(NSString *)module method:(NSString *)method
                 args:(NSArray *)args completion:(dispatch_block_t)completion {
  [self.batchedBridge enqueueJSCall:module method:method args:args completion:completion];
}
```

#### RCTCxxBridge 实现 (`React/CxxBridge/RCTCxxBridge.mm`)

```objective-c
- (void)enqueueJSCall:(NSString *)module method:(NSString *)method
                 args:(NSArray *)args completion:(dispatch_block_t)completion {
  if (!self.valid) {
    return;
  }

  __weak __typeof(self) weakSelf = self;
  [self _runAfterLoad:^(){
    __strong __typeof(weakSelf) strongSelf = weakSelf;
    if (!strongSelf) return;

    if (strongSelf->_reactInstance) {
      strongSelf->_reactInstance->callJSFunction(
        [module UTF8String],
        [method UTF8String],
        convertIdToFollyDynamic(args ?: @[])
      );
    }
  }];
}
```

### 4. C++ Bridge 层

#### Instance 层 (`ReactCommon/cxxreact/Instance.cpp`)

```cpp
void Instance::callJSFunction(std::string &&module, std::string &&method,
                              folly::dynamic &&params) {
  callback_->incrementPendingJSCalls();
  nativeToJsBridge_->callFunction(std::move(module), std::move(method),
                                  std::move(params));
}
```

#### NativeToJsBridge 层 (`ReactCommon/cxxreact/NativeToJsBridge.cpp`)

```cpp
void NativeToJsBridge::callFunction(
    std::string&& module,
    std::string&& method,
    folly::dynamic&& arguments) {

  int systraceCookie = m_systraceCookie++;
  FbSystraceAsyncFlow::begin(TRACE_TAG_REACT_CXX_BRIDGE, "JSCall", systraceCookie);

  runOnExecutorQueue([this, module = std::move(module), method = std::move(method),
                      arguments = std::move(arguments), systraceCookie] {
    if (m_applicationScriptHasFailure) {
      LOG(ERROR) << "Attempting to call JS function on a bad application bundle";
      throw std::runtime_error("Bad application bundle");
    }

    executor->callFunction(module, method, arguments);
  });
}
```

### 5. JavaScript 执行层

#### MessageQueue 处理 (`Libraries/BatchedBridge/MessageQueue.js`)

```javascript
callFunctionReturnFlushedQueue(module: string, method: string, args: any[]) {  // Line 具体行号待确认
  this.__guard(() => {
    this.__callFunction(module, method, args);
  });
  return this.flushedQueue();
}

__callFunction(module: string, method: string, args: any[]): any {
  this._lastFlush = Date.now();
  this._eventLoopStartTime = this._lastFlush;

  Systrace.beginEvent(`${module}.${method}(...)`);

  if (this.__spy) {
    this.__spy({type: TO_JS, module, method, args});
  }

  const moduleMethods = this.getCallableModule(module);
  const method = moduleMethods[method];
  return method.apply(moduleMethods, args);
}
```

#### RCTDeviceEventEmitter 事件分发 (`Libraries/EventEmitter/RCTDeviceEventEmitter.js`)

```javascript
// RCTDeviceEventEmitter 继承自 EventEmitter
class RCTDeviceEventEmitter extends EventEmitter {  // Line 48
  constructor() {
    const sharedSubscriber = new EventSubscriptionVendor();  // Line 52
    super(sharedSubscriber);  // Line 53
    this.sharedSubscriber = sharedSubscriber;  // Line 54
  }
}

module.exports = new RCTDeviceEventEmitter();  // Line 84
```

#### EventEmitter.emit() (`Libraries/vendor/emitter/EventEmitter.js`)

```javascript
emit(eventType: string) {  // Line 179
  const subscriptions = this._subscriber.getSubscriptionsForType(eventType);  // Line 180-182
  if (subscriptions) {  // Line 183
    for (let i = 0, l = subscriptions.length; i < l; i++) {  // Line 184
      const subscription = subscriptions[i];  // Line 185
      if (subscription) {  // Line 188
        this._currentSubscription = subscription;  // Line 189
        subscription.listener.apply(  // Line 190
          subscription.context,  // Line 191
          Array.prototype.slice.call(arguments, 1)  // Line 192
        );
      }
    }
    this._currentSubscription = null;  // Line 196
  }
}
```

---

## 二、JS 到 Native 调用链路

### 1. JavaScript 发起调用

#### NativeEventEmitter 调用 (`Libraries/EventEmitter/NativeEventEmitter.js`)

```javascript
class NativeEventEmitter extends EventEmitter {  // Line 30
  constructor(nativeModule: ?NativeModule) {
    super(RCTDeviceEventEmitter.sharedSubscriber);  // Line 34
    if (Platform.OS === 'ios') {  // Line 35
      this._nativeModule = nativeModule;  // Line 37
    }
  }

  addListener(eventType: string, listener: Function, context: ?Object): EmitterSubscription {  // Line 41
    if (this._nativeModule != null) {
      this._nativeModule.addListener(eventType);  // Line 47 - 调用 Native 方法
    }
    return super.addListener(eventType, listener, context);  // Line 49
  }
}
```

### 2. MessageQueue 处理 JS 到 Native 调用

#### 调用队列机制 (`Libraries/BatchedBridge/MessageQueue.js`)

```javascript
class MessageQueue {  // Line 40
  _queue: [number[], number[], any[], number];  // Line 42 - 调用队列结构

  constructor() {
    this._queue = [[], [], [], 0];  // Line 58 - [moduleIDs, methodIDs, params, callID]
  }
}
```

### 3. Native 端接收调用

#### RCTEventEmitter 接收 JS 调用 (`React/Modules/RCTEventEmitter.m`)

```objective-c
RCT_EXPORT_METHOD(addListener:(NSString *)eventName) {  // Line 74
  if (RCT_DEBUG && ![[self supportedEvents] containsObject:eventName]) {  // Line 76
    RCTLogError(@"`%@` is not a supported event type", eventName);  // Line 77-78
  }
  _listenerCount++;  // Line 80
  if (_listenerCount == 1) {  // Line 81
    [self startObserving];  // Line 82 - 开始监听原生事件
  }
}
```

---

## 三、关键数据结构和接口

### 1. EventSubscriptionVendor (`Libraries/vendor/emitter/EventSubscriptionVendor.js`)

```javascript
class EventSubscriptionVendor {  // Line 21
  _subscriptionsForType: Object;  // Line 22 - 按事件类型存储订阅

  addSubscription(eventType: string, subscription: EventSubscription): EventSubscription {  // Line 36
    if (!this._subscriptionsForType[eventType]) {  // Line 44
      this._subscriptionsForType[eventType] = [];  // Line 45
    }
    const key = this._subscriptionsForType[eventType].length;  // Line 47
    this._subscriptionsForType[eventType].push(subscription);  // Line 48
    subscription.eventType = eventType;  // Line 49
    subscription.key = key;  // Line 50
    return subscription;  // Line 51
  }
}
```

### 2. EmitterSubscription (`Libraries/vendor/emitter/EmitterSubscription.js`)

```javascript
class EmitterSubscription extends EventSubscription {  // Line 21
  emitter: EventEmitter;  // Line 22
  listener: Function;  // Line 23
  context: ?Object;  // Line 24

  remove() {  // Line 54
    this.emitter.removeSubscription(this);  // Line 55
  }
}
```

### 3. BatchedBridge 全局对象 (`Libraries/BatchedBridge/BatchedBridge.js`)

```javascript
const BatchedBridge = new MessageQueue();  // Line 15

Object.defineProperty(global, '__fbBatchedBridge', {  // Line 23
  configurable: true,
  value: BatchedBridge,  // Line 25
});
```

---

## 四、性能优化机制

### 1. 监听器计数优化
- **文件**: `React/Modules/RCTEventEmitter.m:80-82`
- **机制**: 只在第一个监听器添加时开始监听原生事件

### 2. 事件验证
- **文件**: `React/Modules/RCTEventEmitter.m:43-45`
- **机制**: 开发模式下验证事件类型

### 3. 线程安全
- **文件**: `React/CxxBridge/RCTCxxBridge.mm`
- **机制**: weak/strong 引用模式避免循环引用

### 4. 数据序列化
- **文件**: `React/CxxUtils/RCTFollyConvert.h:15-16`
- **机制**: `convertIdToFollyDynamic` 高效数据转换

---

## 五、调试和监控

### 1. Systrace 集成
- **文件**: `ReactCommon/cxxreact/NativeToJsBridge.cpp`
- **功能**: 性能追踪和调试

### 2. MessageQueue Spy
- **文件**: `Libraries/BatchedBridge/MessageQueue.js:88-100`
- **功能**: 调用监控和调试

### 3. 错误处理
- **文件**: `React/Modules/RCTEventEmitter.m:38-41`
- **功能**: Bridge 状态检查和错误报告

---

## 六、完整调用堆栈示例

### Native 到 JS 事件流程（以 AppState 为例）

```
1. iOS 系统通知
   UIApplicationDidEnterBackgroundNotification

2. RCTAppState.handleAppStateDidChange:
   文件: React/Modules/RCTAppState.m:94

3. RCTEventEmitter.sendEventWithName:body:
   文件: React/Modules/RCTEventEmitter.m:36

4. RCTBridge.enqueueJSCall:method:args:completion:
   文件: React/Base/RCTBridge.m

5. RCTCxxBridge.enqueueJSCall:method:args:completion:
   文件: React/CxxBridge/RCTCxxBridge.mm

6. Instance.callJSFunction:
   文件: ReactCommon/cxxreact/Instance.cpp

7. NativeToJsBridge.callFunction:
   文件: ReactCommon/cxxreact/NativeToJsBridge.cpp

8. JSExecutor.callFunction:
   文件: ReactCommon/cxxreact/JSCExecutor.cpp:624 (callNativeModules)

9. __fbBatchedBridge.callFunctionReturnFlushedQueue:
   文件: Libraries/BatchedBridge/MessageQueue.js

10. RCTDeviceEventEmitter.emit:
    文件: Libraries/EventEmitter/RCTDeviceEventEmitter.js:84

11. EventEmitter.emit:
    文件: Libraries/vendor/emitter/EventEmitter.js:179

12. 应用层监听器回调执行
```

### JS 到 Native 调用流程

```
1. NativeEventEmitter.addListener:
   文件: Libraries/EventEmitter/NativeEventEmitter.js:47

2. MessageQueue 队列处理:
   文件: Libraries/BatchedBridge/MessageQueue.js:42

3. Native Bridge 接收:
   通过 RCT_EXPORT_METHOD 宏

4. RCTEventEmitter.addListener:
   文件: React/Modules/RCTEventEmitter.m:74

5. RCTEventEmitter.startObserving:
   文件: React/Modules/RCTEventEmitter.m:82
```

---

## 七、注意事项

1. **线程安全**: 所有 Bridge 调用都需要考虑线程安全
2. **内存管理**: 注意监听器的正确移除，避免内存泄漏
3. **性能考虑**: 避免频繁的 Bridge 调用
4. **错误处理**: 正确处理 Bridge 断开和重连的情况

---

*文档创建时间: 2025-11-02*
*基于 React Native 版本: 0.57.8*
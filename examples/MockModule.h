#ifndef MOCKMODULE_H
#define MOCKMODULE_H

#include <iostream>

#include "common/modules/NativeModule.h"

/**
 * MockModule - 测试用的模拟Native模块
 *
 * 这是一个简单的测试模块，用于验证模块框架的正确性。
 * 它实现了几个基础方法来测试：
 * 1. 模块注册和ID分配
 * 2. 方法调用和参数传递
 * 3. 回调机制
 * 4. 错误处理
 */
class MockModule : public mini_rn::modules::NativeModule {
 public:
  /**
   * 获取模块名称
   */
  std::string getName() const override { return "MockModule"; }

  /**
   * 获取模块导出的方法列表
   */
  std::vector<std::string> getMethods() const override {
    return {"testMethod", "echoMessage", "throwError", "asyncMethod"};
  }

  /**
   * 获取模块导出的常量
   */
  // std::map<std::string, std::string> getConstants() const override {
  //     return {
  //         {"TEST_CONSTANT", "\"test_value\""},
  //         {"VERSION", "\"1.0.0\""},
  //         {"PLATFORM", "\"macOS\""}
  //     };
  // }

  /**
   * 调用模块方法
   */
  void invoke(const std::string& methodName, const std::string& args,
              int callId) override {
    std::cout << "[MockModule] Invoking method '" << methodName
              << "' with args: " << args << ", callId: " << callId << std::endl;

    if (methodName == "testMethod") {
      handleTestMethod(args, callId);
    } else if (methodName == "echoMessage") {
      handleEchoMessage(args, callId);
    } else if (methodName == "throwError") {
      handleThrowError(args, callId);
    } else if (methodName == "asyncMethod") {
      handleAsyncMethod(args, callId);
    } else {
      // 方法不存在，返回错误
      if (callbackHandler_) {
        std::string error =
            "Method '" + methodName + "' not found in MockModule";
        callbackHandler_(callId, error, true);
      }
    }
  }

  /**
   * 设置回调处理器
   * 这个方法不是NativeModule接口的一部分，但为了测试方便添加
   */
  void setCallbackHandler(
      std::function<void(int, const std::string&, bool)> handler) {
    callbackHandler_ = handler;
  }

 private:
  std::function<void(int, const std::string&, bool)> callbackHandler_;

  /**
   * 处理 testMethod 调用
   */
  void handleTestMethod(const std::string& args, int callId) {
    std::cout << "[MockModule] Handling testMethod" << std::endl;

    // 简单的测试方法，返回成功消息
    std::string result =
        "{\"status\": \"success\", \"message\": \"testMethod called\", "
        "\"args\": " +
        args + "}";

    if (callbackHandler_) {
      callbackHandler_(callId, result, false);
    }
  }

  /**
   * 处理 echoMessage 调用
   */
  void handleEchoMessage(const std::string& args, int callId) {
    std::cout << "[MockModule] Handling echoMessage" << std::endl;

    // 回显消息
    std::string result = "{\"echo\": " + args + "}";

    if (callbackHandler_) {
      callbackHandler_(callId, result, false);
    }
  }

  /**
   * 处理 throwError 调用
   */
  void handleThrowError(const std::string& args, int callId) {
    std::cout << "[MockModule] Handling throwError" << std::endl;

    // 故意返回错误
    std::string error = "Intentional error for testing";

    if (callbackHandler_) {
      callbackHandler_(callId, error, true);
    }
  }

  /**
   * 处理 asyncMethod 调用
   */
  void handleAsyncMethod(const std::string& args, int callId) {
    std::cout << "[MockModule] Handling asyncMethod" << std::endl;

    // 模拟异步操作
    std::string result = "{\"async_result\": \"completed\", \"timestamp\": \"" +
                         std::to_string(time(nullptr)) + "\"}";

    if (callbackHandler_) {
      callbackHandler_(callId, result, false);
    }
  }
};

#endif  // MOCKMODULE_H
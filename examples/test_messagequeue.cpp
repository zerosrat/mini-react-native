#include <fstream>
#include <iostream>
#include <sstream>

#include "bridge/JSCExecutor.h"

using namespace mini_rn::bridge;

std::string readFile(const std::string& filename) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cerr << "Failed to open: " << filename << std::endl;
    return "";
  }
  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

int main() {
  std::cout << "=== Mini React Native - MessageQueue Validation ==="
            << std::endl;

  try {
    JSCExecutor executor;
    executor.setJSExceptionHandler([](const std::string& error) {
      std::cout << "[JS Exception] " << error << std::endl;
    });

    // 加载 MessageQueue.js
    std::cout << "Loading MessageQueue.js..." << std::endl;
    std::string messageQueueJS = readFile("src/js/MessageQueue.js");
    if (!messageQueueJS.empty()) {
      executor.loadApplicationScript(messageQueueJS, "MessageQueue.js");
      std::cout << "MessageQueue.js loaded successfully" << std::endl;
    }

    // 加载 BatchedBridge.js
    std::cout << "Loading BatchedBridge.js..." << std::endl;
    std::string batchedBridgeJS = readFile("src/js/BatchedBridge.js");
    if (!batchedBridgeJS.empty()) {
      executor.loadApplicationScript(batchedBridgeJS, "BatchedBridge.js");
      std::cout << "BatchedBridge.js loaded successfully" << std::endl;
    }

    // 运行测试
    std::cout << "Running MessageQueue tests..." << std::endl;
    std::string testScript = readFile("examples/scripts/test_messagequeue.js");
    if (!testScript.empty()) {
      executor.loadApplicationScript(testScript, "test_messagequeue.js");
    }

    std::cout << "\n=== Test Execution Completed ===" << std::endl;

  } catch (const std::exception& e) {
    std::cout << "Test failed: " << e.what() << std::endl;
  }

  return 0;
}
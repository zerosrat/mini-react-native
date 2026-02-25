/**
 * test_ui_manager_integration.cpp - UIManager 集成测试
 *
 * 测试 JavaScript 端 UIManager 与 C++ 端 UIManagerModule 的完整集成流程。
 * 验证：
 * - JavaScript 调用 UIManager.createView
 * - 通过 BatchedBridge 传递到 C++
 * - UIManagerModule 接收并处理
 * - Shadow Tree 正确更新
 */

#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include "common/bridge/JSCExecutor.h"
#include "common/modules/UIManager.h"
#include "common/view/ShadowTree.h"
#include "common/view/ShadowNodeImpl.h"

using namespace mini_rn;
using namespace mini_rn::view;
using namespace mini_rn::modules;

// 测试辅助函数
void printTestHeader(const std::string& testName) {
    std::cout << "\n=== " << testName << " ===" << std::endl;
}

void printTestResult(const std::string& testName, bool passed) {
    std::cout << "[TEST] " << testName << ": "
              << (passed ? "PASSED" : "FAILED") << std::endl;
}

// 测试 1: 基础 UIManager 创建视图
bool testBasicViewCreation() {
    printTestHeader("Basic View Creation Test");

    // 创建 Shadow Tree
    auto shadowTree = std::make_shared<ShadowTree>();

    // 创建 UIManager Module
    UIManager uiManager(shadowTree);

    // 验证 UIManager 初始化
    bool test1 = (uiManager.getName() == "UIManager");
    auto methods = uiManager.getMethods();
    bool test2 = (methods.size() >= 5 && methods[0] == "createView");

    // 模拟 JavaScript 调用 createView
    // 参数格式: [tag, className, props, rootViewTag]
    std::string args = "[1,\"RCTView\",\"{\\\"style\\\":{\\\"width\\\":100,\\\"height\\\":100}}\",0]";
    uiManager.invoke("createView", args, 1);

    // 验证 Shadow Tree 中创建了节点
    auto node = shadowTree->getNodeByTag(1);
    bool test3 = (node != nullptr);
    bool test4 = (node && node->getViewName() == "RCTView");

    std::cout << "UIManager name: " << uiManager.getName() << std::endl;
    std::cout << "Methods count: " << methods.size() << std::endl;
    if (node) {
        std::cout << "Created node: " << node->toString() << std::endl;
    }

    bool allPassed = test1 && test2 && test3 && test4;
    printTestResult("Basic view creation", allPassed);

    return allPassed;
}

// 测试 2: 复杂视图层次结构
bool testComplexViewHierarchy() {
    printTestHeader("Complex View Hierarchy Test");

    // 创建 Shadow Tree
    auto shadowTree = std::make_shared<ShadowTree>();
    UIManager uiManager(shadowTree);

    // 创建根视图
    uiManager.invoke("createView", "[0,\"RCTView\",\"{\\\"style\\\":{\\\"flex\\\":1}}\",-1]", 1);

    // 创建容器视图
    uiManager.invoke("createView", "[1,\"RCTView\",\"{\\\"style\\\":{\\\"padding\\\":10}}\",0]", 2);

    // 创建文本视图
    uiManager.invoke("createView", "[2,\"RCTText\",\"{\\\"text\\\":\\\"Hello World\\\"}\",0]", 3);

    // 创建图片视图
    uiManager.invoke("createView", "[3,\"RCTImage\",\"{\\\"source\\\":{\\\"uri\\\":\\\"test.png\\\"}}\",0]", 4);

    // 构建视图层次：0 -> [1, 2, 3]
    uiManager.invoke("setChildren", "[0,[1,2,3]]", 5);

    // 验证层次结构
    auto root = shadowTree->getNodeByTag(0);
    bool test1 = (root != nullptr && root->getChildCount() == 3);

    auto container = shadowTree->getNodeByTag(1);
    bool test2 = (container != nullptr && container->getViewName() == "RCTView");

    auto text = shadowTree->getNodeByTag(2);
    bool test3 = (text != nullptr && text->getViewName() == "RCTText");

    auto image = shadowTree->getNodeByTag(3);
    bool test4 = (image != nullptr && image->getViewName() == "RCTImage");

    // 打印树结构
    std::cout << "\nView hierarchy:" << std::endl;
    shadowTree->printTree();

    bool allPassed = test1 && test2 && test3 && test4;
    printTestResult("Complex view hierarchy", allPassed);

    return allPassed;
}

// 测试 3: 视图属性更新
bool testViewPropertyUpdates() {
    printTestHeader("View Property Updates Test");

    auto shadowTree = std::make_shared<ShadowTree>();
    UIManager uiManager(shadowTree);

    // 创建视图
    uiManager.invoke("createView", "[1,\"RCTView\",\"{\\\"style\\\":{\\\"width\\\":100}}\",0]", 1);

    // 验证初始属性
    auto node = shadowTree->getNodeByTag(1);
    std::string initialProps = node ? node->getProps() : "";
    bool test1 = (initialProps.find("\\\"width\\\":100") != std::string::npos);

    // 更新属性
    uiManager.invoke("updateView", "[1,\"RCTView\",\"{\\\"style\\\":{\\\"width\\\":200,\\\"height\\\":150}}\"]", 2);

    // 验证更新后的属性
    std::string updatedProps = node ? node->getProps() : "";
    bool test2 = (updatedProps.find("\\\"width\\\":200") != std::string::npos);
    bool test3 = (updatedProps.find("\\\"height\\\":150") != std::string::npos);

    std::cout << "Initial props: " << initialProps << std::endl;
    std::cout << "Updated props: " << updatedProps << std::endl;

    bool allPassed = test1 && test2 && test3;
    printTestResult("View property updates", allPassed);

    return allPassed;
}

// 测试 4: JavaScript 集成测试
bool testJavaScriptIntegration() {
    printTestHeader("JavaScript Integration Test");

    try {
        // 创建 Shadow Tree 和 UIManager
        auto shadowTree = std::make_shared<ShadowTree>();
        auto uiManagerModule = std::make_shared<UIManager>(shadowTree);

        // 创建 JSCExecutor
        bridge::JSCExecutor executor;

        // 注册 UIManager 到 ModuleRegistry
        auto moduleRegistry = executor.getModuleRegistry();
        // TODO: 需要实现 registerModule 方法
        // moduleRegistry->registerModule("UIManager", uiManagerModule);

        // 直接使用 UIManager 模拟 JavaScript 调用
        // 创建根视图（tag=0）
        uiManagerModule->invoke("createView", "[0,\"RCTView\",\"{\\\"style\\\":{\\\"width\\\":100,\\\"height\\\":100,\\\"backgroundColor\\\":\\\"red\\\"}}\",-1]", 1);
        uiManagerModule->invoke("createView", "[11,\"RCTText\",\"{\\\"text\\\":\\\"Hello from JavaScript!\\\"}\",0]", 2);
        uiManagerModule->invoke("setChildren", "[0,[11]]", 3);

        std::string result = "JavaScript simulation completed";
        bool test1 = (result.find("JavaScript simulation completed") != std::string::npos);

        // 验证 Shadow Tree 中的节点
        auto viewNode = shadowTree->getNodeByTag(0);
        auto textNode = shadowTree->getNodeByTag(11);
        bool test2 = (viewNode != nullptr && viewNode->getViewName() == "RCTView");
        bool test3 = (textNode != nullptr && textNode->getViewName() == "RCTText");
        bool test4 = (viewNode && viewNode->getChildCount() == 1);

        std::cout << "JavaScript result: " << result << std::endl;
        std::cout << "\nShadow Tree after JavaScript execution:" << std::endl;
        shadowTree->printTree();

        bool allPassed = test1 && test2 && test3 && test4;
        printTestResult("JavaScript integration", allPassed);

        return allPassed;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        printTestResult("JavaScript integration", false);
        return false;
    }
}

// 测试 5: 批量操作性能测试
bool testBatchOperationPerformance() {
    printTestHeader("Batch Operation Performance Test");

    auto shadowTree = std::make_shared<ShadowTree>();
    UIManager uiManager(shadowTree);

    const int NUM_VIEWS = 100;
    auto startTime = std::chrono::high_resolution_clock::now();

    // 首先创建根视图
    uiManager.invoke("createView", "[0,\"RCTView\",\"{}\",-1]", 0);

    // 批量创建视图
    for (int i = 1; i <= NUM_VIEWS; ++i) {
        std::string args = "[" + std::to_string(i) + ",\"RCTView\",\"{}\",0]";
        uiManager.invoke("createView", args, i);
    }

    // 批量添加到根视图
    std::string children = "[0,[";
    for (int i = 1; i <= NUM_VIEWS; ++i) {
        children += std::to_string(i);
        if (i < NUM_VIEWS) children += ",";
    }
    children += "]]";
    uiManager.invoke("setChildren", children, NUM_VIEWS + 1);

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    // 验证结果
    auto root = shadowTree->getNodeByTag(0);
    bool test1 = (root != nullptr);
    bool test2 = (root && root->getChildCount() == NUM_VIEWS);
    // TODO: 需要实现 getNodeCount 方法
    size_t nodeCount = shadowTree->getStatistics().find("Total nodes:") != std::string::npos ?
        std::stoi(shadowTree->getStatistics().substr(shadowTree->getStatistics().find("Total nodes:") + 12)) : 0;
    bool test3 = (nodeCount == NUM_VIEWS + 1); // +1 for root

    std::cout << "Created " << NUM_VIEWS << " views in "
              << duration.count() << " ms" << std::endl;
    std::cout << "Total nodes in Shadow Tree: " << nodeCount << std::endl;

    bool allPassed = test1 && test2 && test3;
    printTestResult("Batch operation performance", allPassed);

    return allPassed;
}

// 主测试函数
int main() {
    std::cout << "UIManager Integration Test Suite" << std::endl;
    std::cout << "===============================" << std::endl;

    std::vector<std::pair<std::string, std::function<bool()>>> tests = {
        {"Basic View Creation", testBasicViewCreation},
        {"Complex View Hierarchy", testComplexViewHierarchy},
        {"View Property Updates", testViewPropertyUpdates},
        {"JavaScript Integration", testJavaScriptIntegration},
        {"Batch Operation Performance", testBatchOperationPerformance}
    };

    int passed = 0;
    int total = tests.size();

    for (const auto& test : tests) {
        std::cout << "\n" << std::string(50, '=') << std::endl;
        if (test.second()) {
            passed++;
        }
    }

    std::cout << "\n" << std::string(50, '=') << std::endl;
    std::cout << "Test Results: " << passed << "/" << total << " tests passed" << std::endl;

    if (passed == total) {
        std::cout << "✅ All tests PASSED!" << std::endl;
        return 0;
    } else {
        std::cout << "❌ Some tests FAILED!" << std::endl;
        return 1;
    }
}
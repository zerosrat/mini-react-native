/**
 * test_shadow_node.cpp - Shadow Tree 基础功能测试
 *
 * 测试 ShadowNode 和 ShadowTree 的核心功能：
 * - 节点创建和属性管理
 * - 树结构操作
 * - Diff 算法和更新指令生成
 * - 批量更新机制
 */

#include <iostream>
#include <memory>
#include <vector>
#include "common/view/ShadowNode.h"
#include "common/view/ShadowNodeImpl.h"
#include "common/view/ShadowTree.h"

using namespace mini_rn::view;

// 测试辅助函数
void printTestHeader(const std::string& testName) {
    std::cout << "\n=== " << testName << " ===" << std::endl;
}

void printTestResult(const std::string& testName, bool passed) {
    std::cout << "[TEST] " << testName << ": "
              << (passed ? "PASSED" : "FAILED") << std::endl;
}

// 测试 1: ShadowNode 基础功能
bool testShadowNodeBasics() {
    printTestHeader("ShadowNode Basics Test");

    // 创建节点
    auto node = std::make_shared<ShadowNodeImpl>(1, "RCTView", "{\"style\":{\"width\":100}}");

    // 验证基础属性
    bool test1 = (node->getTag() == 1);
    bool test2 = (node->getViewName() == "RCTView");
    bool test3 = (node->getProps() == "{\"style\":{\"width\":100}}");
    bool test4 = (node->getChildCount() == 0);
    bool test5 = (node->isRoot());

    // 更新属性
    node->updateProps("{\"style\":{\"width\":200}}");
    bool test6 = (node->getProps() == "{\"style\":{\"width\":200}}");

    // 打印节点信息
    std::cout << "Node info: " << node->toString() << std::endl;

    bool allPassed = test1 && test2 && test3 && test4 && test5 && test6;
    printTestResult("Basic properties", allPassed);

    return allPassed;
}

// 测试 2: ShadowNode 树结构
bool testShadowNodeTree() {
    printTestHeader("ShadowNode Tree Structure Test");

    // 创建父子节点
    auto parent = std::make_shared<ShadowNodeImpl>(1, "RCTView", "{}");
    auto child1 = std::make_shared<ShadowNodeImpl>(2, "RCTText", "{\"text\":\"Hello\"}");
    auto child2 = std::make_shared<ShadowNodeImpl>(3, "RCTText", "{\"text\":\"World\"}");

    // 添加子节点
    parent->addChild(child1);
    parent->addChild(child2);

    // 验证树结构
    bool test1 = (parent->getChildCount() == 2);
    bool test2 = (parent->getChildAt(0)->getTag() == 2);
    bool test3 = (parent->getChildAt(1)->getTag() == 3);
    bool test4 = (!child1->isRoot());
    bool test5 = (child1->getParent().lock() == parent);

    // 查找子节点
    auto found = parent->findChildByTag(2);
    bool test6 = (found && found->getTag() == 2);

    // 移除子节点
    bool removed = parent->removeChild(2);
    bool test7 = (removed && parent->getChildCount() == 1);
    bool test8 = (parent->getChildAt(0)->getTag() == 3);

    // 打印树结构
    std::cout << "Tree structure:" << std::endl;
    parent->printTree();

    bool allPassed = test1 && test2 && test3 && test4 && test5 && test6 && test7 && test8;
    printTestResult("Tree structure operations", allPassed);

    return allPassed;
}

// 测试 3: ShadowTree 基础功能
bool testShadowTreeBasics() {
    printTestHeader("ShadowTree Basics Test");

    ShadowTree tree;

    // 创建节点
    auto node1 = tree.createNode(1, "RCTView", "{}");
    auto node2 = tree.createNode(2, "RCTText", "{\"text\":\"Hello\"}");

    // 验证节点创建
    bool test1 = (node1 != nullptr);
    bool test2 = (node2 != nullptr);
    bool test3 = (tree.getNodeByTag(1) == node1);
    bool test4 = (tree.getNodeByTag(2) == node2);

    // 设置根节点
    tree.setRootNode(node1);
    bool test5 = (tree.getRootNode() == node1);

    // 添加子节点
    bool added = tree.appendChild(1, 2);
    bool test6 = (added && node1->getChildCount() == 1);

    // 更新属性
    bool updated = tree.updateProps(2, "{\"text\":\"Hello World\"}");
    bool test7 = (updated && node2->getProps() == "{\"text\":\"Hello World\"}");

    // 打印统计信息
    std::cout << tree.getStatistics() << std::endl;
    std::cout << "Tree structure:" << std::endl;
    tree.printTree();

    bool allPassed = test1 && test2 && test3 && test4 && test5 && test6 && test7;
    printTestResult("ShadowTree basic operations", allPassed);

    return allPassed;
}

// 测试 4: Diff 算法
bool testDiffAlgorithm() {
    printTestHeader("Diff Algorithm Test");

    ShadowTree tree;

    // 创建初始树
    auto root1 = tree.createNode(1, "RCTView", "{\"style\":{\"flex\":1}}");
    auto child1 = tree.createNode(2, "RCTText", "{\"text\":\"Hello\"}");
    auto child2 = tree.createNode(3, "RCTView", "{\"style\":{\"width\":100}}");
    auto grandChild1 = tree.createNode(4, "RCTText", "{\"text\":\"World\"}");

    // 构建树结构
    tree.appendChild(1, 2);
    tree.appendChild(1, 3);
    tree.appendChild(3, 4);
    tree.setRootNode(root1);

    std::cout << "Original tree:" << std::endl;
    tree.printTree();

    // 创建新树（模拟更新）
    ShadowTree newTree;
    auto root2 = newTree.createNode(1, "RCTView", "{\"style\":{\"flex\":1}}");  // 相同
    auto child1_2 = newTree.createNode(2, "RCTText", "{\"text\":\"Hello Updated\"}");  // 属性更新
    auto child2_2 = newTree.createNode(5, "RCTView", "{\"style\":{\"width\":200}}");  // 新节点
    auto grandChild2 = newTree.createNode(6, "RCTText", "{\"text\":\"New\"}");  // 新节点

    newTree.appendChild(1, 2);
    newTree.appendChild(1, 5);  // 使用新标签
    newTree.appendChild(5, 6);

    // 计算 diff
    auto commands = tree.calculateUpdates(root2);

    std::cout << "\nGenerated commands (" << commands.size() << "):" << std::endl;
    for (const auto& cmd : commands) {
        std::cout << "  - Type: " << static_cast<int>(cmd.type)
                  << ", Tag: " << cmd.tag;
        if (!cmd.props.empty()) {
            std::cout << ", Props: " << cmd.props.substr(0, 30) << "...";
        }
        std::cout << std::endl;
    }

    // 验证命令
    bool test1 = (!commands.empty());

    // 应该有更新命令（属性变化）
    bool hasUpdate = false;
    for (const auto& cmd : commands) {
        if (cmd.type == UIManagerCommandType::UPDATE_VIEW) {
            hasUpdate = true;
            break;
        }
    }
    bool test2 = hasUpdate;

    bool allPassed = test1 && test2;
    printTestResult("Diff algorithm", allPassed);

    return allPassed;
}

// 测试 5: 批量更新
bool testBatchUpdates() {
    printTestHeader("Batch Updates Test");

    ShadowTree tree;

    // 设置初始状态
    auto root = tree.createNode(1, "RCTView", "{}");
    tree.setRootNode(root);

    // 创建批量更新命令
    std::vector<UIManagerCommand> commands;

    // 创建多个视图
    commands.push_back(UIManagerCommand::createView(2, "RCTText", "{\"text\":\"Child 1\"}", 1));
    commands.push_back(UIManagerCommand::createView(3, "RCTText", "{\"text\":\"Child 2\"}", 1));
    commands.push_back(UIManagerCommand::createView(4, "RCTView", "{\"style\":{\"width\":100}}", 1));

    // 设置子节点
    commands.push_back(UIManagerCommand::setChildren(1, {2, 3, 4}));

    // 更新属性
    commands.push_back(UIManagerCommand::updateView(2, "RCTText", "{\"text\":\"Updated Child 1\"}"));

    std::cout << "Applying " << commands.size() << " batch commands..." << std::endl;

    // 提交批量更新
    tree.commitUpdates(commands);

    // 验证结果
    bool test1 = (root->getChildCount() == 3);
    bool test2 = (tree.getNodeByTag(2) != nullptr);
    bool test3 = (tree.getNodeByTag(3) != nullptr);
    bool test4 = (tree.getNodeByTag(4) != nullptr);

    auto child1 = tree.getNodeByTag(2);
    bool test5 = (child1 && child1->getProps() == "{\"text\":\"Updated Child 1\"}");

    std::cout << "\nResulting tree:" << std::endl;
    tree.printTree();

    bool allPassed = test1 && test2 && test3 && test4 && test5;
    printTestResult("Batch updates", allPassed);

    return allPassed;
}

// 测试 6: 树验证
bool testTreeValidation() {
    printTestHeader("Tree Validation Test");

    ShadowTree tree;

    // 创建正常树
    auto root = tree.createNode(1, "RCTView", "{}");
    auto child = tree.createNode(2, "RCTText", "{}");
    tree.appendChild(1, 2);
    tree.setRootNode(root);

    bool test1 = tree.validate();
    std::cout << "Valid tree validation: " << (test1 ? "PASSED" : "FAILED") << std::endl;

    // 创建孤立节点
    auto orphan = tree.createNode(3, "RCTView", "{}");
    // 不添加到树中，形成孤立节点

    bool test2 = !tree.validate();  // 应该验证失败
    std::cout << "Orphaned node detection: " << (test2 ? "PASSED" : "FAILED") << std::endl;

    bool allPassed = test1 && test2;
    printTestResult("Tree validation", allPassed);

    return allPassed;
}

// 主测试函数
int main() {
    std::cout << "Shadow Tree Test Suite" << std::endl;
    std::cout << "======================" << std::endl;

    std::vector<std::pair<std::string, std::function<bool()>>> tests = {
        {"ShadowNode Basics", testShadowNodeBasics},
        {"ShadowNode Tree", testShadowNodeTree},
        {"ShadowTree Basics", testShadowTreeBasics},
        {"Diff Algorithm", testDiffAlgorithm},
        {"Batch Updates", testBatchUpdates},
        {"Tree Validation", testTreeValidation}
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
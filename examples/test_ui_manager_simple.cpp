/**
 * test_ui_manager_simple.cpp - Simple UIManager test
 *
 * Simple test to verify UIManager basic functionality
 */

#include <iostream>
#include <memory>
#include "common/modules/UIManager.h"
#include "common/view/ShadowTree.h"

using namespace mini_rn;
using namespace mini_rn::view;
using namespace mini_rn::modules;

int main() {
    std::cout << "Simple UIManager Test" << std::endl;
    std::cout << "====================" << std::endl;

    // Create Shadow Tree
    auto shadowTree = std::make_shared<ShadowTree>();

    // Create UIManager
    UIManager uiManager(shadowTree);

    std::cout << "UIManager name: " << uiManager.getName() << std::endl;

    // Test creating a view
    std::cout << "\nCreating view with tag 1..." << std::endl;
    uiManager.invoke("createView", "[1,\"RCTView\",\"{}\",0]", 1);

    // Check if node was created
    auto node = shadowTree->getNodeByTag(1);
    if (node) {
        std::cout << "✅ Node created successfully!" << std::endl;
        std::cout << "   Tag: " << node->getTag() << std::endl;
        std::cout << "   View Name: " << node->getViewName() << std::endl;
        std::cout << "   Props: " << node->getProps() << std::endl;

        // Set it as root node
        shadowTree->setRootNode(node);
        std::cout << "   Set as root node" << std::endl;
    } else {
        std::cout << "❌ Failed to create node!" << std::endl;
        return 1;
    }

    // Test creating a text node
    std::cout << "\nCreating text view with tag 2..." << std::endl;
    uiManager.invoke("createView", "[2,\"RCTText\",\"{\\\"text\\\":\\\"Hello\\\"}\",0]", 2);

    auto textNode = shadowTree->getNodeByTag(2);
    if (textNode) {
        std::cout << "✅ Text node created successfully!" << std::endl;
        std::cout << "   Tag: " << textNode->getTag() << std::endl;
        std::cout << "   View Name: " << textNode->getViewName() << std::endl;
        std::cout << "   Props: " << textNode->getProps() << std::endl;
    } else {
        std::cout << "❌ Failed to create text node!" << std::endl;
        return 1;
    }

    // Test setting children
    std::cout << "\nSetting children: view 1 as parent of view 2..." << std::endl;
    uiManager.invoke("setChildren", "[1,[2]]", 3);

    auto parentNode = shadowTree->getNodeByTag(1);
    if (parentNode && parentNode->getChildCount() == 1) {
        std::cout << "✅ Children set successfully!" << std::endl;
        std::cout << "   Parent has " << parentNode->getChildCount() << " child" << std::endl;
    } else {
        std::cout << "❌ Failed to set children!" << std::endl;
        return 1;
    }

    // Print tree structure
    std::cout << "\nFinal tree structure:" << std::endl;
    shadowTree->printTree();

    std::cout << "\n✅ All tests passed!" << std::endl;
    return 0;
}
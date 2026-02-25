#include "ShadowTree.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <unordered_set>

#include "ShadowNodeImpl.h"

namespace mini_rn {
namespace view {

// 静态成员初始化
Tag ShadowTree::s_nextTag = 1;

ShadowTree::ShadowTree() {
  // 初始化时创建一个空的根节点占位符
  // 实际的根节点会在应用启动时通过 setRootNode 设置
}

ShadowNodePtr ShadowTree::createNode(Tag tag, const std::string& viewName,
                                     const std::string& props) {
  // 检查标签是否已被使用
  if (nodeRegistry_.find(tag) != nodeRegistry_.end()) {
    std::cerr << "[ShadowTree] Warning: Tag " << tag << " already exists"
              << std::endl;
    return nullptr;
  }

  // 创建 ShadowNode 实例
  // 使用默认实现类，实际使用中应该根据 viewName 创建对应的派生类
  auto node = std::make_shared<ShadowNodeImpl>(tag, viewName, props);

  // 注册节点
  registerNode(node);

  return node;
}

ShadowNodePtr ShadowTree::getNodeByTag(Tag tag) const {
  auto it = nodeRegistry_.find(tag);
  return (it != nodeRegistry_.end()) ? it->second : nullptr;
}

void ShadowTree::setRootNode(ShadowNodePtr root) {
  if (rootNode_) {
    // 清理旧的根节点
    cleanupNode(rootNode_);
  }

  rootNode_ = root;
  if (root) {
    registerNode(root);
  }
}

bool ShadowTree::appendChild(Tag parentTag, Tag childTag) {
  auto parent = getNodeByTag(parentTag);
  auto child = getNodeByTag(childTag);

  if (!parent || !child) {
    std::cerr << "[ShadowTree] appendChild failed: parent or child not found"
              << std::endl;
    return false;
  }

  parent->addChild(child);
  return true;
}

bool ShadowTree::insertChildAt(Tag parentTag, Tag childTag, size_t index) {
  auto parent = getNodeByTag(parentTag);
  auto child = getNodeByTag(childTag);

  if (!parent || !child) {
    std::cerr << "[ShadowTree] insertChildAt failed: parent or child not found"
              << std::endl;
    return false;
  }

  parent->addChild(child, index);
  return true;
}

bool ShadowTree::removeChild(Tag parentTag, Tag childTag) {
  auto parent = getNodeByTag(parentTag);

  if (!parent) {
    std::cerr << "[ShadowTree] removeChild failed: parent not found"
              << std::endl;
    return false;
  }

  bool removed = parent->removeChild(childTag);
  if (removed) {
    unregisterNode(childTag);
  }

  return removed;
}

bool ShadowTree::updateProps(Tag tag, const std::string& props) {
  auto node = getNodeByTag(tag);

  if (!node) {
    std::cerr << "[ShadowTree] updateProps failed: node not found" << std::endl;
    return false;
  }

  node->updateProps(props);
  return true;
}

bool ShadowTree::setChildren(Tag parentTag, const std::vector<Tag>& childTags) {
  auto parent = getNodeByTag(parentTag);

  if (!parent) {
    std::cerr << "[ShadowTree] setChildren failed: parent not found"
              << std::endl;
    return false;
  }

  // 移除所有现有子节点
  parent->removeAllChildren();

  // 添加新的子节点
  for (Tag childTag : childTags) {
    auto child = getNodeByTag(childTag);
    if (child) {
      parent->addChild(child);
    } else {
      std::cerr << "[ShadowTree] Warning: Child node " << childTag
                << " not found" << std::endl;
    }
  }

  return true;
}

std::vector<UIManagerCommand> ShadowTree::calculateUpdates(
    ShadowNodePtr newRoot) {
  std::vector<UIManagerCommand> commands;

  if (!newRoot) {
    // 如果新根为空，删除整个树
    if (rootNode_) {
      commands.push_back(UIManagerCommand::deleteView(rootNode_->getTag()));
    }
    return commands;
  }

  if (!rootNode_) {
    // 如果没有旧根，直接创建新树
    commands.push_back(UIManagerCommand::createView(
        newRoot->getTag(), newRoot->getViewName(), newRoot->getProps()));

    // 递归创建所有子节点
    std::function<void(const ShadowNodePtr&)> createChildren =
        [&](const ShadowNodePtr& node) {
          for (const auto& child : node->getChildren()) {
            if (child) {
              commands.push_back(UIManagerCommand::createView(
                  child->getTag(), child->getViewName(), child->getProps(),
                  node->getTag()));
              createChildren(child);
            }
          }
        };

    createChildren(newRoot);

    // 最后设置根节点的子节点
    if (!newRoot->getChildren().empty()) {
      std::vector<int> childTags;
      for (const auto& child : newRoot->getChildren()) {
        if (child) {
          childTags.push_back(child->getTag());
        }
      }
      commands.push_back(
          UIManagerCommand::setChildren(newRoot->getTag(), childTags));
    }
  } else {
    // 执行 diff 算法
    diffNodes(rootNode_, newRoot, commands);
  }

  return commands;
}

void ShadowTree::commitUpdates(const std::vector<UIManagerCommand>& commands) {
  // 应用命令到当前树
  // 注意：这里简化处理，实际实现中需要考虑事务性

  for (const auto& cmd : commands) {
    switch (cmd.type) {
      case UIManagerCommandType::CREATE_VIEW: {
        auto node = createNode(cmd.tag, cmd.className, cmd.props);
        if (node && cmd.parentTag != -1) {
          appendChild(cmd.parentTag, cmd.tag);
        }
        break;
      }

      case UIManagerCommandType::UPDATE_VIEW:
        updateProps(cmd.tag, cmd.props);
        break;

      case UIManagerCommandType::DELETE_VIEW: {
        auto node = getNodeByTag(cmd.tag);
        if (node) {
          auto parent = node->getParent().lock();
          if (parent) {
            parent->removeChild(cmd.tag);
          }
          cleanupNode(node);
        }
        break;
      }

      case UIManagerCommandType::SET_CHILDREN:
        setChildren(cmd.parentTag, cmd.childTags);
        break;

      default:
        std::cerr << "[ShadowTree] Unsupported command type: "
                  << static_cast<int>(cmd.type) << std::endl;
    }
  }
}

std::string ShadowTree::getStatistics() const {
  std::ostringstream oss;
  oss << "ShadowTree Statistics:\n";
  oss << "  Total nodes: " << nodeRegistry_.size() << "\n";

  if (rootNode_) {
    std::function<int(const ShadowNodePtr&)> countNodes =
        [&](const ShadowNodePtr& node) -> int {
      int count = 1;
      for (const auto& child : node->getChildren()) {
        if (child) {
          count += countNodes(child);
        }
      }
      return count;
    };

    int treeSize = countNodes(rootNode_);
    oss << "  Tree size: " << treeSize << "\n";
    oss << "  Orphaned nodes: " << (nodeRegistry_.size() - treeSize) << "\n";
  }

  return oss.str();
}

void ShadowTree::printTree() const {
  if (rootNode_) {
    std::cout << "ShadowTree Structure:\n";
    rootNode_->printTree(0);
  } else {
    std::cout << "ShadowTree is empty\n";
  }
}

bool ShadowTree::validate() const {
  // 检查注册表中的所有节点是否都能通过树访问到
  std::unordered_set<Tag> treeNodes;

  if (rootNode_) {
    std::function<void(const ShadowNodePtr&)> collectTags =
        [&](const ShadowNodePtr& node) {
          treeNodes.insert(node->getTag());
          for (const auto& child : node->getChildren()) {
            if (child) {
              collectTags(child);
            }
          }
        };

    collectTags(rootNode_);
  }

  // 检查是否有孤立节点
  bool hasOrphans = false;
  for (const auto& pair : nodeRegistry_) {
    if (treeNodes.find(pair.first) == treeNodes.end()) {
      std::cerr << "[ShadowTree] Validation Error: Orphaned node " << pair.first
                << std::endl;
      hasOrphans = true;
    }
  }

  return !hasOrphans;
}

void ShadowTree::diffNodes(const ShadowNodePtr& oldNode,
                           const ShadowNodePtr& newNode,
                           std::vector<UIManagerCommand>& commands) {
  if (!oldNode && !newNode) {
    return;
  }

  if (!oldNode && newNode) {
    // 新节点被创建
    commands.push_back(UIManagerCommand::createView(
        newNode->getTag(), newNode->getViewName(), newNode->getProps()));

    // 递归处理子节点
    for (const auto& child : newNode->getChildren()) {
      if (child) {
        diffNodes(nullptr, child, commands);
      }
    }
    return;
  }

  if (oldNode && !newNode) {
    // 节点被删除
    commands.push_back(UIManagerCommand::deleteView(oldNode->getTag()));
    return;
  }

  // 比较现有节点
  if (oldNode->getTag() != newNode->getTag()) {
    // 标签不同，删除旧节点，创建新节点
    diffNodes(oldNode, nullptr, commands);
    diffNodes(nullptr, newNode, commands);
    return;
  }

  if (!oldNode->isSameType(newNode)) {
    // 类型不同，替换节点
    commands.push_back(UIManagerCommand::deleteView(oldNode->getTag()));
    commands.push_back(UIManagerCommand::createView(
        newNode->getTag(), newNode->getViewName(), newNode->getProps()));
  } else if (oldNode->getProps() != newNode->getProps()) {
    // 属性不同，更新属性
    commands.push_back(UIManagerCommand::updateView(
        newNode->getTag(), newNode->getViewName(), newNode->getProps()));
  }

  // 处理子节点差异
  diffChildren(oldNode, newNode, commands);
}

void ShadowTree::diffChildren(const ShadowNodePtr& oldParent,
                              const ShadowNodePtr& newParent,
                              std::vector<UIManagerCommand>& commands) {
  const auto& oldChildren = oldParent->getChildren();
  const auto& newChildren = newParent->getChildren();

  // 简化实现：如果子节点列表不同，直接替换
  // 实际的 React Native 实现有更复杂的 key-based diff 算法

  if (oldChildren.size() != newChildren.size()) {
    // 子节点数量不同，直接替换
    generateSetChildrenCommand(newParent->getTag(), newChildren, commands);

    // 递归处理新子节点
    // for (size_t i = 0; i < newChildren.size(); ++i) {
    //     if (i < oldChildren.size()) {
    //         diffNodes(oldChildren[i], newChildren[i], commands);
    //     } else {
    //         diffNodes(nullptr, newChildren[i], commands);
    //     }
    // }
  } else {
    // 子节点数量相同，逐个比较
    for (size_t i = 0; i < newChildren.size(); ++i) {
      diffNodes(oldChildren[i], newChildren[i], commands);
    }
  }
}

void ShadowTree::generateSetChildrenCommand(
    Tag parentTag, const std::vector<ShadowNodePtr>& children,
    std::vector<UIManagerCommand>& commands) {
  std::vector<int> childTags;
  for (const auto& child : children) {
    if (child) {
      childTags.push_back(child->getTag());
    }
  }
  commands.push_back(UIManagerCommand::setChildren(parentTag, childTags));
}

void ShadowTree::registerNode(const ShadowNodePtr& node) {
  if (node) {
    nodeRegistry_[node->getTag()] = node;
  }
}

void ShadowTree::unregisterNode(Tag tag) { nodeRegistry_.erase(tag); }

void ShadowTree::cleanupNode(const ShadowNodePtr& node) {
  if (!node) {
    return;
  }

  // 递归清理所有子节点
  for (const auto& child : node->getChildren()) {
    if (child) {
      cleanupNode(child);
    }
  }

  // 从注册表中移除
  unregisterNode(node->getTag());
}

bool ShadowTree::isNodeInTree(Tag tag) const {
  if (!rootNode_) {
    return false;
  }

  std::function<bool(const ShadowNodePtr&, Tag)> search =
      [&](const ShadowNodePtr& node, Tag target) -> bool {
    if (!node) {
      return false;
    }

    if (node->getTag() == target) {
      return true;
    }

    for (const auto& child : node->getChildren()) {
      if (search(child, target)) {
        return true;
      }
    }

    return false;
  };

  return search(rootNode_, tag);
}

}  // namespace view
}  // namespace mini_rn
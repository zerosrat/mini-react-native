#include "ShadowNode.h"
#include <iostream>
#include <sstream>
#include <algorithm>

namespace mini_rn {
namespace view {

ShadowNode::ShadowNode(Tag tag, const std::string& viewName, const std::string& props)
    : tag_(tag), viewName_(viewName), props_(props) {
    // 初始化布局结果为空对象
    layoutResult_ = "{\"x\":0,\"y\":0,\"width\":0,\"height\":0}";
}

void ShadowNode::addChild(ShadowNodePtr child, size_t index) {
    if (!child) {
        std::cerr << "[ShadowNode] Warning: Attempting to add null child" << std::endl;
        return;
    }

    // 检查是否已经是子节点
    auto existingParent = child->getParent().lock();
    if (existingParent && existingParent.get() == this) {
        std::cerr << "[ShadowNode] Warning: Child is already attached to this node" << std::endl;
        return;
    }

    // 从旧父节点移除
    if (existingParent) {
        existingParent->removeChild(child->getTag());
    }

    // 添加到指定位置
    if (index >= children_.size() || index == SIZE_MAX) {
        children_.push_back(child);
    } else {
        children_.insert(children_.begin() + index, child);
    }

    // 更新子节点的父引用
    updateChildParent(child, shared_from_this());
}

bool ShadowNode::removeChild(Tag childTag) {
    auto it = std::find_if(children_.begin(), children_.end(),
        [childTag](const ShadowNodePtr& child) {
            return child && child->getTag() == childTag;
        });

    if (it != children_.end()) {
        // 清除子节点的父引用
        if (*it) {
            (*it)->parent_.reset();
        }
        children_.erase(it);
        return true;
    }

    return false;
}

bool ShadowNode::removeChildAt(size_t index) {
    if (index >= children_.size()) {
        return false;
    }

    if (children_[index]) {
        children_[index]->parent_.reset();
    }
    children_.erase(children_.begin() + index);
    return true;
}

ShadowNodePtr ShadowNode::getChildAt(size_t index) const {
    if (index >= children_.size()) {
        return nullptr;
    }
    return children_[index];
}

ShadowNodePtr ShadowNode::findChildByTag(Tag childTag) const {
    auto it = std::find_if(children_.begin(), children_.end(),
        [childTag](const ShadowNodePtr& child) {
            return child && child->getTag() == childTag;
        });

    return (it != children_.end()) ? *it : nullptr;
}

void ShadowNode::removeAllChildren() {
    for (auto& child : children_) {
        if (child) {
            child->parent_.reset();
        }
    }
    children_.clear();
}

int ShadowNode::getIndexInParent() const {
    auto parent = parent_.lock();
    if (!parent) {
        return -1;
    }

    const auto& siblings = parent->getChildren();
    auto it = std::find_if(siblings.begin(), siblings.end(),
        [this](const ShadowNodePtr& sibling) {
            return sibling && sibling->getTag() == tag_;
        });

    if (it != siblings.end()) {
        return static_cast<int>(std::distance(siblings.begin(), it));
    }

    return -1;
}

void ShadowNode::updateProps(const std::string& newProps) {
    props_ = newProps;
}

bool ShadowNode::isSameType(const ShadowNodePtr& other) const {
    if (!other) {
        return false;
    }
    return viewName_ == other->getViewName();
}

void ShadowNode::setLayoutResult(float x, float y, float width, float height) {
    std::ostringstream oss;
    oss << "{\"x\":" << x << ",\"y\":" << y
        << ",\"width\":" << width << ",\"height\":" << height << "}";
    layoutResult_ = oss.str();
}

std::string ShadowNode::toString() const {
    std::ostringstream oss;
    oss << "ShadowNode{tag=" << tag_
        << ", viewName=\"" << viewName_ << "\""
        << ", children=" << children_.size()
        << ", props=" << (props_.length() > 50 ? props_.substr(0, 50) + "..." : props_)
        << "}";
    return oss.str();
}

void ShadowNode::printTree(int depth) const {
    // 打印缩进
    std::string indent(depth * 2, ' ');

    // 打印当前节点
    std::cout << indent << "- " << toString() << std::endl;

    // 递归打印子节点
    for (const auto& child : children_) {
        if (child) {
            child->printTree(depth + 1);
        }
    }
}

void ShadowNode::updateChildParent(ShadowNodePtr child, const std::weak_ptr<ShadowNode>& newParent) {
    if (child) {
        child->parent_ = newParent;
    }
}

} // namespace view
} // namespace mini_rn
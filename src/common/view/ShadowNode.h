#ifndef SHADOWNODE_H
#define SHADOWNODE_H

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <functional>

namespace mini_rn {
namespace view {

// 前向声明
class ShadowNode;

// 类型别名
using Tag = int;
using ShadowNodePtr = std::shared_ptr<ShadowNode>;

/**
 * ShadowNode - Shadow Tree 虚拟 DOM 节点基类
 *
 * 这是 React Native Shadow Tree 的核心抽象，代表虚拟 DOM 中的一个节点。
 * 每个 ShadowNode 对应一个 React 组件实例，但不包含实际的视图渲染逻辑。
 *
 * 设计原则：
 * - 轻量级：只存储必要的元数据，不包含重量级的视图对象
 * - 不可变性：属性更新通过创建新节点实现，保证 diff 算法正确性
 * - 树形结构：维护父子关系，支持高效的树操作
 * - 类型安全：使用强类型标签，避免运行时错误
 *
 * 使用示例：
 * ```cpp
 * // 创建节点
 * auto node = std::make_shared<ShadowNode>(1, "RCTView", "{\"style\":{\"width\":100}}");
 *
 * // 添加子节点
 * node->addChild(childNode);
 *
 * // 更新属性
 * node->updateProps("{\"style\":{\"width\":200}}");
 * ```
 */
class ShadowNode : public std::enable_shared_from_this<ShadowNode> {
public:
    /**
     * 构造函数
     *
     * @param tag 节点的唯一标识符（React Node Tag）
     * @param viewName 视图类型名称（如 "RCTView", "RCTText"）
     * @param props 组件属性，JSON 字符串格式
     */
    ShadowNode(Tag tag, const std::string& viewName, const std::string& props);

    /**
     * 虚析构函数
     * 确保派生类可以正确析构
     */
    virtual ~ShadowNode() = default;

    // ========================================================================
    // 基础属性访问
    // ========================================================================

    /**
     * 获取节点标签
     * @return 节点的唯一标识符
     */
    Tag getTag() const { return tag_; }

    /**
     * 获取视图名称
     * @return 视图类型名称
     */
    const std::string& getViewName() const { return viewName_; }

    /**
     * 获取组件属性
     * @return 属性的 JSON 字符串
     */
    const std::string& getProps() const { return props_; }

    /**
     * 获取父节点
     * @return 父节点的弱引用
     */
    const std::weak_ptr<ShadowNode>& getParent() const { return parent_; }

    /**
     * 获取所有子节点
     * @return 子节点列表
     */
    const std::vector<ShadowNodePtr>& getChildren() const { return children_; }

    /**
     * 检查是否为根节点
     * @return 如果没有父节点则返回 true
     */
    bool isRoot() const { return parent_.expired(); }

    /**
     * 获取子节点数量
     * @return 子节点数量
     */
    size_t getChildCount() const { return children_.size(); }

    // ========================================================================
    // 树结构管理
    // ========================================================================

    /**
     * 添加子节点
     * @param child 要添加的子节点
     * @param index 插入位置，默认添加到末尾
     */
    virtual void addChild(ShadowNodePtr child, size_t index = SIZE_MAX);

    /**
     * 移除子节点
     * @param childTag 要移除的子节点标签
     * @return 是否成功移除
     */
    virtual bool removeChild(Tag childTag);

    /**
     * 移除指定位置的子节点
     * @param index 子节点位置
     * @return 是否成功移除
     */
    virtual bool removeChildAt(size_t index);

    /**
     * 获取指定位置的子节点
     * @param index 子节点位置
     * @return 子节点指针，如果索引无效则返回 nullptr
     */
    virtual ShadowNodePtr getChildAt(size_t index) const;

    /**
     * 根据标签查找子节点
     * @param childTag 子节点标签
     * @return 子节点指针，如果未找到则返回 nullptr
     */
    virtual ShadowNodePtr findChildByTag(Tag childTag) const;

    /**
     * 移除所有子节点
     */
    virtual void removeAllChildren();

    /**
     * 获取节点在父节点中的索引
     * @return 索引位置，如果是根节点则返回 -1
     */
    virtual int getIndexInParent() const;

    // ========================================================================
    // 属性和状态管理
    // ========================================================================

    /**
     * 更新节点属性
     * @param newProps 新的属性，JSON 字符串格式
     */
    virtual void updateProps(const std::string& newProps);

    /**
     * 检查两个节点是否类型相同
     * 用于 diff 算法判断是否可以复用节点
     * @param other 另一个节点
     * @return 如果类型相同则返回 true
     */
    virtual bool isSameType(const ShadowNodePtr& other) const;

    /**
     * 克隆节点
     * 创建一个具有相同属性的新节点实例
     * @return 新节点的智能指针
     */
    virtual ShadowNodePtr clone() const = 0;

    // ========================================================================
    // 布局相关（为 Yoga 集成预留）
    // ========================================================================

    /**
     * 计算布局
     * 子类可以重写此方法实现自定义布局逻辑
     */
    virtual void calculateLayout() {}

    /**
     * 设置布局结果
     * @param x X 坐标
     * @param y Y 坐标
     * @param width 宽度
     * @param height 高度
     */
    virtual void setLayoutResult(float x, float y, float width, float height);

    /**
     * 获取布局结果
     * @return 布局结果的 JSON 字符串
     */
    virtual const std::string& getLayoutResult() const { return layoutResult_; }

    // ========================================================================
    // 调试和诊断
    // ========================================================================

    /**
     * 获取节点的字符串表示
     * 用于调试输出
     * @return 描述节点的字符串
     */
    virtual std::string toString() const;

    /**
     * 打印节点树
     * 用于调试，递归打印整个子树
     * @param depth 缩进深度
     */
    virtual void printTree(int depth = 0) const;

protected:
    Tag tag_;                        // 节点唯一标识
    std::string viewName_;           // 视图类型名称
    std::string props_;              // 组件属性（JSON 字符串）
    std::string layoutResult_;       // 布局结果（JSON 字符串）

    std::vector<ShadowNodePtr> children_;  // 子节点列表
    std::weak_ptr<ShadowNode> parent_;     // 父节点的弱引用

private:
    /**
     * 更新子节点的父引用
     * @param child 子节点
     * @param newParent 新的父节点
     */
    void updateChildParent(ShadowNodePtr child, const std::weak_ptr<ShadowNode>& newParent);
};

} // namespace view
} // namespace mini_rn

#endif // SHADOWNODE_H
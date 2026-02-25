#ifndef SHADOWTREE_H
#define SHADOWTREE_H

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include "ShadowNode.h"

namespace mini_rn {
namespace view {

// 前向声明
class ShadowNode;

/**
 * UIManager 命令类型枚举
 * 定义了所有支持的原生视图操作指令
 */
enum class UIManagerCommandType {
    CREATE_VIEW,        // 创建视图
    UPDATE_VIEW,        // 更新视图属性
    DELETE_VIEW,        // 删除视图
    SET_CHILDREN,       // 设置子视图列表
    MANAGE_CHILDREN,    // 管理子视图（增删移动）
    UPDATE_LAYOUT,      // 更新布局
    // 后续可扩展
    // SET_PROPS,
    // REMOVE_PROPS,
};

/**
 * UIManager 命令结构
 * 封装了视图操作的所有必要信息
 */
struct UIManagerCommand {
    UIManagerCommandType type;    // 命令类型
    int tag;                      // 目标视图标签
    std::string className;        // 视图类名（用于创建）
    std::string props;            // 视图属性（JSON）
    int parentTag;                // 父视图标签
    std::vector<int> childTags;   // 子视图标签列表
    std::vector<int> indices;     // 子视图索引列表

    // 构造函数
    UIManagerCommand(UIManagerCommandType t, int tg)
        : type(t), tag(tg), parentTag(-1) {}

    static UIManagerCommand createView(int tag, const std::string& className,
                                     const std::string& props, int parentTag = -1) {
        UIManagerCommand cmd(UIManagerCommandType::CREATE_VIEW, tag);
        cmd.className = className;
        cmd.props = props;
        cmd.parentTag = parentTag;
        return cmd;
    }

    static UIManagerCommand updateView(int tag, const std::string& className,
                                     const std::string& props) {
        UIManagerCommand cmd(UIManagerCommandType::UPDATE_VIEW, tag);
        cmd.className = className;
        cmd.props = props;
        return cmd;
    }

    static UIManagerCommand deleteView(int tag) {
        return UIManagerCommand(UIManagerCommandType::DELETE_VIEW, tag);
    }

    static UIManagerCommand setChildren(int parentTag, const std::vector<int>& childTags) {
        UIManagerCommand cmd(UIManagerCommandType::SET_CHILDREN, parentTag);
        cmd.childTags = childTags;
        cmd.parentTag = parentTag;
        return cmd;
    }

    static UIManagerCommand manageChildren(int parentTag,
                                         const std::vector<int>& /*moveFrom*/,
                                         const std::vector<int>& /*moveTo*/,
                                         const std::vector<int>& addChildTags,
                                         const std::vector<int>& addAtIndices,
                                         const std::vector<int>& /*removeFrom*/,
                                         const std::vector<int>& /*removeAtIndices*/) {
        UIManagerCommand cmd(UIManagerCommandType::MANAGE_CHILDREN, parentTag);
        cmd.childTags = addChildTags;
        cmd.indices = addAtIndices;
        // 注意：moveFrom/moveTo/removeFrom/removeAtIndices 需要额外的字段支持
        // 这里简化处理，实际实现时可能需要扩展结构
        return cmd;
    }
};

/**
 * ShadowTree - Shadow Tree 管理器
 *
 * 管理整个虚拟 DOM 树，负责节点的创建、更新、删除和 diff 计算。
 * 这是 React Native 渲染流程的核心组件，连接 JavaScript 端的 React 组件树
 * 和原生端的实际视图树。
 *
 * 主要职责：
 * - 维护节点注册表，支持通过标签快速查找节点
 * - 执行 reconciliation 算法，计算新旧树的差异
 * - 生成最小化的 UIManager 命令序列
 * - 提供树操作的事务性支持
 *
 * 设计原则：
 * - 高效 diff：O(n) 时间复杂度的树差异计算
 * - 增量更新：只更新真正发生变化的节点
 * - 批量操作：支持批量提交更新，提高性能
 * - 线程安全：为后续的多线程渲染预留接口
 */
class ShadowTree {
public:
    /**
     * 构造函数
     */
    ShadowTree();

    /**
     * 析构函数
     */
    ~ShadowTree() = default;

    // ========================================================================
    // 节点管理
    // ========================================================================

    /**
     * 创建新节点
     *
     * @param tag 节点标签
     * @param viewName 视图类型名称
     * @param props 节点属性（JSON 字符串）
     * @return 创建的节点指针
     */
    virtual ShadowNodePtr createNode(Tag tag, const std::string& viewName,
                                   const std::string& props);

    /**
     * 根据标签获取节点
     *
     * @param tag 节点标签
     * @return 节点指针，如果未找到则返回 nullptr
     */
    virtual ShadowNodePtr getNodeByTag(Tag tag) const;

    /**
     * 获取根节点
     *
     * @return 根节点指针
     */
    virtual ShadowNodePtr getRootNode() const { return rootNode_; }

    /**
     * 设置根节点
     * 通常在应用启动时调用
     *
     * @param root 新的根节点
     */
    virtual void setRootNode(ShadowNodePtr root);

    // ========================================================================
    // 树操作
    // ========================================================================

    /**
     * 添加子节点
     *
     * @param parentTag 父节点标签
     * @param childTag 子节点标签
     * @return 操作是否成功
     */
    virtual bool appendChild(Tag parentTag, Tag childTag);

    /**
     * 在指定位置插入子节点
     *
     * @param parentTag 父节点标签
     * @param childTag 子节点标签
     * @param index 插入位置
     * @return 操作是否成功
     */
    virtual bool insertChildAt(Tag parentTag, Tag childTag, size_t index);

    /**
     * 移除子节点
     *
     * @param parentTag 父节点标签
     * @param childTag 子节点标签
     * @return 操作是否成功
     */
    virtual bool removeChild(Tag parentTag, Tag childTag);

    /**
     * 更新节点属性
     *
     * @param tag 节点标签
     * @param props 新的属性（JSON 字符串）
     * @return 操作是否成功
     */
    virtual bool updateProps(Tag tag, const std::string& props);

    /**
     * 设置节点的子节点列表
     * 替换所有现有的子节点
     *
     * @param parentTag 父节点标签
     * @param childTags 子节点标签列表
     * @return 操作是否成功
     */
    virtual bool setChildren(Tag parentTag, const std::vector<Tag>& childTags);

    // ========================================================================
    // Diff 和更新
    // ========================================================================

    /**
     * 计算新旧树的差异
     * 这是 reconciliation 算法的核心入口
     *
     * @param newRoot 新的根节点
     * @return UIManager 命令序列
     */
    virtual std::vector<UIManagerCommand> calculateUpdates(ShadowNodePtr newRoot);

    /**
     * 提交更新命令
     * 将计算出的命令应用到当前树
     *
     * @param commands UIManager 命令序列
     */
    virtual void commitUpdates(const std::vector<UIManagerCommand>& commands);

    // ========================================================================
    // 调试和诊断
    // ========================================================================

    /**
     * 获取树的统计信息
     *
     * @return 包含节点数量等信息的字符串
     */
    virtual std::string getStatistics() const;

    /**
     * 打印整个树结构
     * 用于调试
     */
    virtual void printTree() const;

    /**
     * 验证树的完整性
     * 检查循环引用、孤立节点等问题
     *
     * @return 验证是否通过
     */
    virtual bool validate() const;

protected:
    ShadowNodePtr rootNode_;  // 根节点
    std::unordered_map<Tag, ShadowNodePtr> nodeRegistry_;  // 节点注册表

    // 递增的标签计数器，用于自动生成标签
    static Tag s_nextTag;

private:
    // ========================================================================
    // Diff 算法实现
    // ========================================================================

    /**
     * 比较两个节点的差异
     *
     * @param oldNode 旧节点
     * @param newNode 新节点
     * @param commands 输出的命令序列
     */
    virtual void diffNodes(const ShadowNodePtr& oldNode,
                          const ShadowNodePtr& newNode,
                          std::vector<UIManagerCommand>& commands);

    /**
     * 处理子节点列表的差异
     *
     * @param oldParent 旧父节点
     * @param newParent 新父节点
     * @param commands 输出的命令序列
     */
    virtual void diffChildren(const ShadowNodePtr& oldParent,
                             const ShadowNodePtr& newParent,
                             std::vector<UIManagerCommand>& commands);

    /**
     * 生成 SET_CHILDREN 命令
     *
     * @param parentTag 父节点标签
     * @param children 子节点列表
     * @param commands 输出的命令序列
     */
    virtual void generateSetChildrenCommand(Tag parentTag,
                                          const std::vector<ShadowNodePtr>& children,
                                          std::vector<UIManagerCommand>& commands);

    // ========================================================================
    // 辅助方法
    // ========================================================================

    /**
     * 将节点添加到注册表
     *
     * @param node 要注册的节点
     */
    virtual void registerNode(const ShadowNodePtr& node);

    /**
     * 从注册表移除节点
     *
     * @param tag 要移除的节点标签
     */
    virtual void unregisterNode(Tag tag);

    /**
     * 递归清理节点及其子节点
     *
     * @param node 要清理的节点
     */
    virtual void cleanupNode(const ShadowNodePtr& node);

    /**
     * 检查节点是否在树中
     *
     * @param tag 节点标签
     * @return 是否在树中
     */
    virtual bool isNodeInTree(Tag tag) const;
};

} // namespace view
} // namespace mini_rn

#endif // SHADOWTREE_H
#ifndef SHADOWNODEIMPL_H
#define SHADOWNODEIMPL_H

#include "ShadowNode.h"

namespace mini_rn {
namespace view {

/**
 * ShadowNodeImpl - ShadowNode 的默认实现类
 *
 * 这是一个具体的 ShadowNode 实现，用于创建基本的视图节点。
 * 在实际应用中，不同的视图类型（如 View、Text、Image）会有
 * 自己的派生类，但这个基础实现可以用于通用场景。
 */
class ShadowNodeImpl : public ShadowNode {
public:
    /**
     * 构造函数
     */
    ShadowNodeImpl(Tag tag, const std::string& viewName, const std::string& props);

    /**
     * 析构函数
     */
    ~ShadowNodeImpl() = default;

    /**
     * 克隆节点
     * 创建一个具有相同属性的新节点实例
     * @return 新节点的智能指针
     */
    ShadowNodePtr clone() const override;
};

} // namespace view
} // namespace mini_rn

#endif // SHADOWNODEIMPL_H
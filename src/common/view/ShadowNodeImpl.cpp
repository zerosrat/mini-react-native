#include "ShadowNodeImpl.h"

namespace mini_rn {
namespace view {

ShadowNodeImpl::ShadowNodeImpl(Tag tag, const std::string& viewName, const std::string& props)
    : ShadowNode(tag, viewName, props) {
}

ShadowNodePtr ShadowNodeImpl::clone() const {
    return std::make_shared<ShadowNodeImpl>(tag_, viewName_, props_);
}

} // namespace view
} // namespace mini_rn
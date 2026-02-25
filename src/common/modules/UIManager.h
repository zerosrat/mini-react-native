#ifndef UIMANAGER_H
#define UIMANAGER_H

#include <memory>
#include <vector>
#include <string>
#include "NativeModule.h"
#include "common/view/ShadowTree.h"

namespace mini_rn {
namespace modules {

/**
 * UIManager - React Native 兼容的视图管理模块
 *
 * 这个模块负责管理 Shadow Tree 和视图更新，是 React Native 渲染系统的核心。
 * 它接收来自 JavaScript 端的视图操作指令，并维护虚拟 DOM 树。
 *
 * 支持的方法：
 * - createView(tag, className, props, rootViewTag): 创建新视图
 * - updateView(tag, className, props): 更新视图属性
 * - manageChildren(containerViewTag, moveFrom, moveTo, addChildTags, addAtIndices, removeFrom, removeAtIndices): 管理子视图
 * - setChildren(containerViewTag, reactTags): 设置子视图列表
 * - removeRootView(rootViewTag): 移除根视图
 *
 * JavaScript 使用示例：
 * ```javascript
 * import { UIManager } from 'react-native';
 *
 * // 创建视图
 * UIManager.createView(1, 'RCTView', {
 *   style: { width: 100, height: 100 }
 * }, 0);
 *
 * // 设置子视图
 * UIManager.setChildren(0, [1]);
 * ```
 */
class UIManager : public NativeModule {
 public:
  /**
   * 构造函数
   * @param shadowTree Shadow Tree 实例，用于管理虚拟 DOM
   */
  explicit UIManager(std::shared_ptr<view::ShadowTree> shadowTree);

  /**
   * 析构函数
   */
  ~UIManager() override = default;

  // NativeModule 接口实现
  std::string getName() const override;
  std::vector<std::string> getMethods() const override;
  void invoke(const std::string& methodName, const std::string& args,
              int callId) override;

 private:
  /**
   * Shadow Tree 实例
   */
  std::shared_ptr<view::ShadowTree> shadowTree_;

  /**
   * 解析 JSON 数组为字符串数组
   */
  std::vector<int> parseJSONArray(const std::string& json) const;

  /**
   * 解析属性 JSON
   */
  std::string parseProps(const std::string& args) const;

  /**
   * 核心方法实现
   */
  void createView(const std::string& args);
  void updateView(const std::string& args);
  void manageChildren(const std::string& args);
  void setChildren(const std::string& args);
  void removeRootView(const std::string& args);
};

}  // namespace modules
}  // namespace mini_rn

#endif  // UIMANAGER_H
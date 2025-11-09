#ifndef MODULEREGISTRY_H
#define MODULEREGISTRY_H

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "NativeModule.h"

namespace mini_rn {
namespace modules {

/**
 * ModuleRegistry - React Native 兼容的模块注册器
 *
 * 这是 React Native Bridge 架构中的核心组件，负责：
 * 1. Native 模块的注册和管理
 * 2. 模块 ID 和方法 ID 的分配与映射
 * 3. JavaScript 到 Native 的方法调用路由
 * 4. 异步调用结果的回调处理
 *
 * 设计原则：
 * - API 设计严格基于 React Native 的 ModuleRegistry
 * - 模块 ID 分配与 RN 保持一致（数组索引）
 * - 支持动态模块注册和方法调用
 * - 保持与 RN Bridge 消息格式的兼容性
 *
 * 架构说明：
 * - modules_: 存储所有注册的模块实例
 * - modulesByName_: 模块名称到索引的映射，用于快速查找
 * - callbackHandler_: 回调处理器，用于将结果返回给 JavaScript
 */
class ModuleRegistry {
 public:
  /**
   * 回调处理器类型定义
   * 用于将 Native 方法的执行结果返回给 JavaScript
   *
   * @param callId 调用标识符，与 JavaScript 调用时的 callId 对应
   * @param result 执行结果，JSON 格式的字符串
   * @param isError 是否为错误结果，true 表示调用失败，false 表示调用成功
   */
  using CallbackHandler =
      std::function<void(int callId, const std::string& result, bool isError)>;

  /**
   * 构造函数
   * 基于 React Native ModuleRegistry API 设计
   *
   * @param modules 初始模块列表，可以为空
   */
  explicit ModuleRegistry(
      std::vector<std::unique_ptr<NativeModule>> modules = {});

  /**
   * 注册模块
   * 基于 React Native ModuleRegistry::registerModules API
   *
   * @param modules 要注册的模块列表
   */
  void registerModules(std::vector<std::unique_ptr<NativeModule>> modules);

  /**
   * 获取所有模块名称
   * 基于 React Native ModuleRegistry::moduleNames API
   *
   * @return 模块名称列表，按注册顺序排列
   */
  std::vector<std::string> moduleNames();

  /**
   * 调用 Native 方法（异步）
   * 基于 React Native ModuleRegistry::callNativeMethod API
   *
   * 这是模块调用的核心入口点。当 JavaScript 通过 Bridge 调用 Native 方法时，
   * JSCExecutor 会解析消息并调用这个方法来执行具体的模块方法。
   *
   * @param moduleId 模块 ID（对应 modules_ 数组的索引）
   * @param methodId 方法 ID（对应模块方法列表的索引）
   * @param params JSON 格式的参数字符串
   * @param callId 调用标识符，用于异步返回结果
   */
  void callNativeMethod(unsigned int moduleId, unsigned int methodId,
                        const std::string& params, int callId);

  /**
   * 设置回调处理器
   * 设置用于将 Native 方法执行结果返回给 JavaScript 的回调函数
   * 注意：回调处理器只能设置一次，重复设置会被忽略以防止意外覆盖
   *
   * @param handler 回调处理器函数
   * @return 如果成功设置返回 true，如果已经设置过则返回 false
   */
  bool setCallbackHandler(CallbackHandler handler);

  /**
   * 获取模块数量
   * @return 当前注册的模块总数
   */
  size_t getModuleCount() const { return modules_.size(); }

  /**
   * 检查模块是否存在
   * @param moduleId 模块 ID
   * @return 如果模块存在返回 true，否则返回 false
   */
  bool hasModule(unsigned int moduleId) const;

  /**
   * 获取模块名称（通过 ID）
   * @param moduleId 模块 ID
   * @return 模块名称，如果模块不存在返回空字符串
   */
  std::string getModuleName(unsigned int moduleId) const;

  /**
   * 获取模块方法数量
   * @param moduleId 模块 ID
   * @return 模块的方法数量，如果模块不存在返回 0
   */
  size_t getModuleMethodCount(unsigned int moduleId) const;

  /**
   * 获取模块的方法名称列表
   * @param moduleId 模块 ID
   * @return 模块的方法名称列表，如果模块不存在返回空列表
   */
  std::vector<std::string> getMethodNames(unsigned int moduleId) const;

  /**
   * 调用可序列化的 Native Hook 方法（同步调用）
   * 基于 React Native callSerializableNativeHook API
   *
   * 这个方法用于同步调用 Native 模块方法，直接返回结果而不通过回调。
   * 主要用于需要立即返回结果的场景，如同步获取设备信息等。
   *
   * @param moduleId 模块 ID（对应 modules_ 数组的索引）
   * @param methodId 方法 ID（对应模块方法列表的索引）
   * @param params JSON 格式的参数字符串
   * @return 方法执行结果的字符串，如果失败返回空字符串
   */
  std::string callSerializableNativeHook(unsigned int moduleId,
                                         unsigned int methodId,
                                         const std::string& params);

  /**
   * 发送成功回调
   * 供 NativeModule 调用，将成功结果返回给 JavaScript
   *
   * @param callId 调用标识符
   * @param result 执行结果
   */
  void sendSuccessCallback(int callId, const std::string& result);

  /**
   * 发送错误回调
   * 供 NativeModule 调用，将错误信息返回给 JavaScript
   *
   * @param callId 调用标识符
   * @param error 错误信息
   */
  void sendErrorCallback(int callId, const std::string& error);

 private:
  /**
   * 模块存储
   * 基于 React Native 的设计，使用 vector 存储模块，索引即为模块 ID
   */
  std::vector<std::unique_ptr<NativeModule>> modules_;

  /**
   * 模块名称映射
   * 基于 React Native 的设计，用于快速根据名称查找模块
   * 键：模块名称，值：模块在 modules_ 中的索引
   */
  std::unordered_map<std::string, size_t> modulesByName_;

  /**
   * 回调处理器
   * 用于将 Native 方法的执行结果返回给 JavaScript
   */
  CallbackHandler callbackHandler_;

  /**
   * 回调处理器是否已设置的标志
   * 防止意外覆盖回调处理器
   */
  bool callbackHandlerSet_ = false;

  /**
   * 更新模块名称映射
   * 基于 React Native ModuleRegistry::updateModuleNamesFromIndex 的设计
   *
   * @param startIndex 开始更新的索引位置
   */
  void updateModuleNamesFromIndex(size_t startIndex);

  /**
   * 验证模块和方法 ID 的有效性
   * @param moduleId 模块 ID
   * @param methodId 方法 ID
   * @return 如果有效返回 true，否则返回 false
   */
  bool validateIds(unsigned int moduleId, unsigned int methodId) const;
};

}  // namespace modules
}  // namespace mini_rn

#endif  // MODULEREGISTRY_H
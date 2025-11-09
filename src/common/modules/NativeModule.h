#ifndef NATIVEMODULE_H
#define NATIVEMODULE_H

#include <map>
#include <string>
#include <vector>

namespace mini_rn {
namespace modules {

// 前向声明
class ModuleRegistry;

/**
 * NativeModule - React Native 兼容的 Native 模块基类
 *
 * 这是所有 Native 模块的基类，严格遵循 React Native 的 NativeModule 接口设计。
 * 每个具体的 Native 模块（如 DeviceInfo、AsyncStorage 等）都需要继承这个基类。
 *
 * 设计原则：
 * - 接口设计与 React Native 的 NativeModule 完全一致
 * - 支持方法导出、常量导出和异步调用
 * - 通过 callId 标识异步调用，结果通过 Bridge 返回
 * - 回调功能通过 ModuleRegistry 统一管理，模块无需直接持有回调处理器
 *
 * 使用示例：
 * ```cpp
 * class DeviceInfoModule : public NativeModule {
 * public:
 *     std::string getName() const override { return "DeviceInfo"; }
 *     std::vector<std::string> getMethods() const override {
 *         return {"getUniqueId", "getSystemVersion"};
 *     }
 *     void invoke(const std::string& methodName, const std::string& args, int
 * callId) override {
 *         // 实现具体的方法调用逻辑
 *         // 使用 sendSuccessCallback 或 sendErrorCallback 返回结果
 *     }
 * };
 * ```
 */
class NativeModule {
 public:
  /**
   * 获取模块名称
   * @return 模块的唯一标识名称，用于在 JavaScript 中引用此模块
   */
  virtual std::string getName() const = 0;

  /**
   * 获取模块导出的方法列表
   * @return 方法名称列表，这些方法可以从 JavaScript 调用
   */
  virtual std::vector<std::string> getMethods() const = 0;

  /**
   * 获取模块导出的常量
   * @return 常量键值对，这些常量在模块初始化时暴露给 JavaScript
   */
  // virtual std::map<std::string, std::string> getConstants() const = 0;

  /**
   * 调用模块方法
   *
   * 这是模块的核心调用接口。当 JavaScript 调用模块方法时，Bridge 会解析
   * 调用信息并通过这个方法分发到具体的模块实现。
   *
   * @param methodName 要调用的方法名称
   * @param args JSON 格式的参数字符串
   * @param callId 调用标识符，用于异步返回结果到 JavaScript
   *
   * 注意：
   * - 方法实现应该是异步的，通过 callId 返回结果
   * - 参数 args 是 JSON 格式的字符串，需要解析后使用
   * - 如果方法执行成功，应该调用 sendSuccessCallback 将结果返回给 JavaScript
   * - 如果方法执行失败，应该调用 sendErrorCallback 返回错误信息
   */
  virtual void invoke(const std::string& methodName, const std::string& args,
                      int callId) = 0;

  /**
   * 设置模块注册器引用
   * 由 ModuleRegistry 在注册模块时自动调用，模块无需手动调用
   *
   * @param registry ModuleRegistry 实例的指针
   */
  virtual void setModuleRegistry(ModuleRegistry* registry);

  /**
   * 发送成功回调到 JavaScript
   *
   * @param callId 调用标识符
   * @param result 执行结果
   */
  void sendSuccessCallback(int callId, const std::string& result);

  /**
   * 发送错误回调到 JavaScript
   *
   * @param callId 调用标识符
   * @param error 错误信息
   */
  void sendErrorCallback(int callId, const std::string& error);

  /**
   * 虚析构函数
   * 确保派生类对象可以正确析构
   */
  virtual ~NativeModule() = default;

protected:
  /**
   * ModuleRegistry 指针，用于访问回调功能
   * 由 setModuleRegistry 设置
   */
  ModuleRegistry* m_moduleRegistry = nullptr;
};

}  // namespace modules
}  // namespace mini_rn

#endif  // NATIVEMODULE_H
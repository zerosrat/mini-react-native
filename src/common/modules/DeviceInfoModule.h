#ifndef DEVICEINFOMODULE_H
#define DEVICEINFOMODULE_H

#include <functional>

#include "NativeModule.h"

namespace mini_rn {
namespace modules {

/**
 * DeviceInfoModule - React Native 兼容的设备信息模块
 *
 * 这个模块提供与 react-native-device-info 兼容的设备信息接口。
 * 支持获取设备的基本信息，如系统版本、设备型号、唯一标识等。
 *
 * 支持的方法：
 * - getUniqueId(): 获取设备唯一标识
 * - getSystemVersion(): 获取系统版本
 * - getDeviceId(): 获取设备硬件型号标识 (如 "Mac16,7")
 *
 * 常量导出：
 * - systemName: 系统名称 (macOS/iOS/Android)
 * - systemVersion: 系统版本
 * - model: 设备型号
 *
 * JavaScript 使用示例：
 * ```javascript
 * import DeviceInfo from 'react-native-device-info';
 *
 * // 异步方法
 * DeviceInfo.getUniqueId().then((uniqueId) => {
 *   console.log('Device ID:', uniqueId);
 * });
 *
 * // 常量访问
 * console.log('System:', DeviceInfo.getConstants().systemName);
 * ```
 */
class DeviceInfoModule : public NativeModule {
 public:
  /**
   * 构造函数
   * 回调功能通过基类的 ModuleRegistry 引用自动处理
   */
  DeviceInfoModule();

  /**
   * 析构函数
   */
  ~DeviceInfoModule() override = default;

  // NativeModule 接口实现
  std::string getName() const override;
  std::vector<std::string> getMethods() const override;
  //   std::map<std::string, std::string> getConstants() const override;
  void invoke(const std::string& methodName, const std::string& args,
              int callId) override;

  /**
   * 平台特定的设备信息获取接口
   * 这些方法由平台特定的实现文件提供 (如 DeviceInfoModule.mm)
   * 公开这些方法以支持调用
   */
  std::string getUniqueIdImpl() const;
  std::string getSystemVersionImpl() const;
  std::string getDeviceIdImpl() const;

 private:
  /**
   * 工具方法
   */
  std::string createSuccessResponse(const std::string& data) const;
  std::string createErrorResponse(const std::string& error) const;
};

}  // namespace modules
}  // namespace mini_rn

#endif  // DEVICEINFOMODULE_H
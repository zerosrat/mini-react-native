#include "NativeModule.h"
#include "ModuleRegistry.h"
#include <iostream>

namespace mini_rn {
namespace modules {

void NativeModule::setModuleRegistry(ModuleRegistry* registry) {
  m_moduleRegistry = registry;
}

void NativeModule::sendSuccessCallback(int callId, const std::string& result) {
  if (m_moduleRegistry) {
    // 通过 ModuleRegistry 的公有方法发送成功回调
    // 我们需要在 ModuleRegistry 中添加公有的回调方法
    m_moduleRegistry->sendSuccessCallback(callId, result);
  } else {
    std::cout << "[NativeModule] Warning: No ModuleRegistry set, cannot send success callback for callId "
              << callId << ", result: " << result << std::endl;
  }
}

void NativeModule::sendErrorCallback(int callId, const std::string& error) {
  if (m_moduleRegistry) {
    // 通过 ModuleRegistry 的公有方法发送错误回调
    // 我们需要在 ModuleRegistry 中添加公有的回调方法
    m_moduleRegistry->sendErrorCallback(callId, error);
  } else {
    std::cout << "[NativeModule] Warning: No ModuleRegistry set, cannot send error callback for callId "
              << callId << ", error: " << error << std::endl;
  }
}

} // namespace modules
} // namespace mini_rn
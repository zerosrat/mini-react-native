#include "ModuleRegistry.h"

#include <iostream>
#include <stdexcept>

#include "DeviceInfoModule.h"

namespace mini_rn {
namespace modules {

ModuleRegistry::ModuleRegistry(
    std::vector<std::unique_ptr<NativeModule>> modules)
    : modules_(std::move(modules)) {
  // 为所有初始模块设置 ModuleRegistry 引用
  for (auto& module : modules_) {
    if (module) {
      module->setModuleRegistry(this);
    }
  }

  // 初始化模块名称映射
  updateModuleNamesFromIndex(0);

  std::cout << "[ModuleRegistry] Initialized with " << modules_.size()
            << " modules" << std::endl;
}

void ModuleRegistry::registerModules(
    std::vector<std::unique_ptr<NativeModule>> modules) {
  if (modules.empty()) {
    return;
  }

  size_t startIndex = modules_.size();

  // 添加新模块到现有模块列表
  for (auto& module : modules) {
    if (module) {
      std::string moduleName = module->getName();

      // 检查模块名称是否已存在
      if (modulesByName_.find(moduleName) != modulesByName_.end()) {
        std::cout << "[ModuleRegistry] Warning: Module '" << moduleName
                  << "' already exists, skipping registration" << std::endl;
        continue;
      }

      // 设置模块的 ModuleRegistry 引用
      module->setModuleRegistry(this);

      modules_.push_back(std::move(module));
    }
  }

  // 更新模块名称映射
  updateModuleNamesFromIndex(startIndex);

  std::cout << "[ModuleRegistry] Registered " << (modules_.size() - startIndex)
            << " new modules, total: " << modules_.size() << std::endl;
}

std::vector<std::string> ModuleRegistry::moduleNames() {
  std::vector<std::string> names;
  names.reserve(modules_.size());

  for (const auto& module : modules_) {
    if (module) {
      names.push_back(module->getName());
    }
  }

  return names;
}

void ModuleRegistry::callNativeMethod(unsigned int moduleId,
                                      unsigned int methodId,
                                      const std::string& params, int callId) {
  std::cout << "[ModuleRegistry] Calling method - Module ID: " << moduleId
            << ", Method ID: " << methodId << ", Call ID: " << callId
            << std::endl;

  // 验证模块和方法 ID
  if (!validateIds(moduleId, methodId)) {
    std::string error = "Invalid module ID (" + std::to_string(moduleId) +
                        ") or method ID (" + std::to_string(methodId) + ")";
    std::cout << "[ModuleRegistry] Error: " << error << std::endl;
    sendErrorCallback(callId, error);
    return;
  }

  try {
    NativeModule* module = modules_[moduleId].get();
    std::vector<std::string> methods = module->getMethods();

    if (methodId >= methods.size()) {
      std::string error = "Method ID " + std::to_string(methodId) +
                          " out of range for module '" + module->getName() +
                          "'";
      std::cout << "[ModuleRegistry] Error: " << error << std::endl;
      sendErrorCallback(callId, error);
      return;
    }

    std::string methodName = methods[methodId];
    std::cout << "[ModuleRegistry] Invoking method '" << methodName
              << "' on module '" << module->getName() << "'" << std::endl;

    // 调用模块方法
    module->invoke(methodName, params, callId);

  } catch (const std::exception& e) {
    std::string error = "Exception in module method: " + std::string(e.what());
    std::cout << "[ModuleRegistry] Error: " << error << std::endl;
    sendErrorCallback(callId, error);
  } catch (...) {
    std::string error = "Unknown exception in module method";
    std::cout << "[ModuleRegistry] Error: " << error << std::endl;
    sendErrorCallback(callId, error);
  }
}

bool ModuleRegistry::setCallbackHandler(CallbackHandler handler) {
  if (callbackHandlerSet_) {
    std::cout << "[ModuleRegistry] Warning: Callback handler already set, ignoring duplicate call"
              << std::endl;
    return false;
  }

  callbackHandler_ = std::move(handler);
  callbackHandlerSet_ = true;
  std::cout << "[ModuleRegistry] Callback handler set successfully" << std::endl;
  return true;
}

bool ModuleRegistry::hasModule(unsigned int moduleId) const {
  return moduleId < modules_.size() && modules_[moduleId] != nullptr;
}

std::string ModuleRegistry::getModuleName(unsigned int moduleId) const {
  if (!hasModule(moduleId)) {
    return "";
  }
  return modules_[moduleId]->getName();
}

size_t ModuleRegistry::getModuleMethodCount(unsigned int moduleId) const {
  if (!hasModule(moduleId)) {
    return 0;
  }
  return modules_[moduleId]->getMethods().size();
}

std::vector<std::string> ModuleRegistry::getMethodNames(
    unsigned int moduleId) const {
  if (!hasModule(moduleId)) {
    return {};
  }
  return modules_[moduleId]->getMethods();
}

std::string ModuleRegistry::callSerializableNativeHook(
    unsigned int moduleId, unsigned int methodId, const std::string& params) {
  // 避免未使用参数的警告
  (void)params;
  std::cout << "[ModuleRegistry] Calling serializable native hook - Module ID: "
            << moduleId << ", Method ID: " << methodId << std::endl;

  // 验证模块和方法 ID
  if (!validateIds(moduleId, methodId)) {
    std::string error = "Invalid module ID (" + std::to_string(moduleId) +
                        ") or method ID (" + std::to_string(methodId) + ")";
    std::cout << "[ModuleRegistry] Error: " << error << std::endl;
    return "";
  }

  try {
    NativeModule* module = modules_[moduleId].get();
    std::vector<std::string> methods = module->getMethods();
    std::string methodName = methods[methodId];
    std::string moduleName = module->getName();

    std::cout << "[ModuleRegistry] Sync calling method '" << methodName
              << "' on module '" << moduleName << "'" << std::endl;

    // 特殊处理已知的同步方法
    if (moduleName == "DeviceInfo") {
      // 将 DeviceInfoModule 转换为具体类型以访问实现方法
      // 注意：这里使用动态转换，在实际项目中可能需要更安全的类型检查
      auto* deviceInfoModule =
          dynamic_cast<mini_rn::modules::DeviceInfoModule*>(module);
      if (!deviceInfoModule) {
        std::cout
            << "[ModuleRegistry] Error: Failed to cast to DeviceInfoModule"
            << std::endl;
        return "";
      }

      if (methodName == "getSystemVersion") {
        return deviceInfoModule->getSystemVersionImpl();
      } else if (methodName == "getDeviceId") {
        return deviceInfoModule->getDeviceIdImpl();
      }
    }

    // 对于其他模块或方法，返回错误
    std::cout << "[ModuleRegistry] Warning: Sync call not supported for "
              << moduleName << "." << methodName << std::endl;
    return "";

  } catch (const std::exception& e) {
    std::string error =
        "Exception in sync module method: " + std::string(e.what());
    std::cout << "[ModuleRegistry] Error: " << error << std::endl;
    return "";
  } catch (...) {
    std::string error = "Unknown exception in sync module method";
    std::cout << "[ModuleRegistry] Error: " << error << std::endl;
    return "";
  }
}

void ModuleRegistry::updateModuleNamesFromIndex(size_t startIndex) {
  for (size_t i = startIndex; i < modules_.size(); ++i) {
    if (modules_[i]) {
      std::string moduleName = modules_[i]->getName();
      modulesByName_[moduleName] = i;

      std::cout << "[ModuleRegistry] Mapped module '" << moduleName
                << "' to ID " << i << std::endl;
    }
  }
}

bool ModuleRegistry::validateIds(unsigned int moduleId,
                                 unsigned int methodId) const {
  // 检查模块 ID 是否有效
  if (!hasModule(moduleId)) {
    return false;
  }

  // 检查方法 ID 是否有效
  std::vector<std::string> methods = modules_[moduleId]->getMethods();
  return methodId < methods.size();
}

void ModuleRegistry::sendErrorCallback(int callId, const std::string& error) {
  if (callbackHandler_) {
    callbackHandler_(callId, error, true);
  } else {
    std::cout << "[ModuleRegistry] Warning: No callback handler set, cannot "
                 "send error: "
              << error << std::endl;
  }
}

void ModuleRegistry::sendSuccessCallback(int callId,
                                         const std::string& result) {
  if (callbackHandler_) {
    callbackHandler_(callId, result, false);
  } else {
    std::cout << "[ModuleRegistry] Warning: No callback handler set, cannot "
                 "send result: "
              << result << std::endl;
  }
}

}  // namespace modules
}  // namespace mini_rn
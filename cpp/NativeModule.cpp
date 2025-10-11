/**
 * NativeModule.cpp
 * Native 模块基类的实现
 */

#include "NativeModule.h"
#include "JSCExecutor.h"
#include <json/json.h>
#include <iostream>
#include <sstream>

namespace bridge {

// 静态成员初始化
std::map<std::string, NativeModuleFactory::CreateFunction> NativeModuleFactory::s_moduleFactories;

NativeModule::NativeModule(const std::string& name) 
    : m_name(name), m_executor(nullptr) {
    std::cout << "[NativeModule] 创建模块: " << name << std::endl;
}

std::string NativeModule::getMethodsJSON() {
    Json::Value methods(Json::arrayValue);
    
    // 重新构建方法描述符映射
    m_methodDescriptors = getMethods();
    m_methodNameToId.clear();
    
    for (size_t i = 0; i < m_methodDescriptors.size(); ++i) {
        methods.append(m_methodDescriptors[i].name);
        m_methodNameToId[m_methodDescriptors[i].name] = static_cast<int>(i);
    }
    
    Json::StreamWriterBuilder builder;
    return Json::writeString(builder, methods);
}

std::string NativeModule::getConstantsJSON() {
    auto constants = getConstants();
    Json::Value jsonConstants;
    
    for (const auto& pair : constants) {
        // 尝试解析为 JSON，如果失败则作为字符串处理
        Json::Value value;
        Json::Reader reader;
        if (reader.parse(pair.second, value)) {
            jsonConstants[pair.first] = value;
        } else {
            jsonConstants[pair.first] = pair.second;
        }
    }
    
    Json::StreamWriterBuilder builder;
    return Json::writeString(builder, jsonConstants);
}

int NativeModule::getMethodId(const std::string& methodName) {
    auto it = m_methodNameToId.find(methodName);
    return (it != m_methodNameToId.end()) ? it->second : -1;
}

const NativeMethodDescriptor* NativeModule::getMethodDescriptor(int methodId) {
    if (methodId >= 0 && methodId < static_cast<int>(m_methodDescriptors.size())) {
        return &m_methodDescriptors[methodId];
    }
    return nullptr;
}

void NativeModule::registerMethod(const NativeMethodDescriptor& descriptor) {
    m_methodDescriptors.push_back(descriptor);
    m_methodNameToId[descriptor.name] = static_cast<int>(m_methodDescriptors.size() - 1);
    
    log("debug", "注册方法: " + descriptor.name);
}

void NativeModule::sendEventToJS(const std::string& eventName, const std::string& eventData) {
    if (!m_executor) {
        log("warn", "无法发送事件，Executor 未设置: " + eventName);
        return;
    }
    
    log("debug", "发送事件到 JS: " + eventName);
    
    // 构造事件调用脚本
    std::ostringstream script;
    script << "if (global.__bridge && global.__bridge._emit) { "
           << "global.__bridge._emit('" << eventName << "', " << eventData << "); "
           << "}";
    
    m_executor->executeScript(script.str(), "event-emit");
}

std::string NativeModule::parseJsonString(const std::string& jsonStr) {
    Json::Value value;
    Json::Reader reader;
    
    if (reader.parse(jsonStr, value)) {
        if (value.isString()) {
            return value.asString();
        } else if (value.isConvertibleTo(Json::stringValue)) {
            return value.asString();
        }
    }
    
    return jsonStr; // 如果解析失败，返回原字符串
}

std::string NativeModule::createJsonString(const std::map<std::string, std::string>& data) {
    Json::Value jsonObj;
    
    for (const auto& pair : data) {
        jsonObj[pair.first] = pair.second;
    }
    
    Json::StreamWriterBuilder builder;
    return Json::writeString(builder, jsonObj);
}

void NativeModule::log(const std::string& level, const std::string& message) {
    std::cout << "[" << m_name << "-" << level << "] " << message << std::endl;
}

// NativeModuleFactory 实现

void NativeModuleFactory::registerModule(const std::string& name, CreateFunction createFunc) {
    s_moduleFactories[name] = createFunc;
    std::cout << "[NativeModuleFactory] 注册模块工厂: " << name << std::endl;
}

std::shared_ptr<NativeModule> NativeModuleFactory::createModule(const std::string& name) {
    auto it = s_moduleFactories.find(name);
    if (it != s_moduleFactories.end()) {
        std::cout << "[NativeModuleFactory] 创建模块实例: " << name << std::endl;
        return it->second();
    }
    
    std::cerr << "[NativeModuleFactory] 未找到模块工厂: " << name << std::endl;
    return nullptr;
}

std::vector<std::string> NativeModuleFactory::getRegisteredModules() {
    std::vector<std::string> names;
    names.reserve(s_moduleFactories.size());
    
    for (const auto& pair : s_moduleFactories) {
        names.push_back(pair.first);
    }
    
    return names;
}

} // namespace bridge

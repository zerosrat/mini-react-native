#include "UIManager.h"
#include "common/utils/JSONParser.h"
#include <sstream>
#include <iostream>

namespace mini_rn {
namespace modules {

UIManager::UIManager(std::shared_ptr<view::ShadowTree> shadowTree)
    : shadowTree_(shadowTree) {
  std::cout << "[UIManager] Initialized with ShadowTree" << std::endl;
}

std::string UIManager::getName() const {
  return "UIManager";
}

std::vector<std::string> UIManager::getMethods() const {
  return {
    "createView",        // methodId = 0
    "updateView",        // methodId = 1
    "manageChildren",    // methodId = 2
    "setChildren",       // methodId = 3
    "removeRootView"     // methodId = 4
  };
}

void UIManager::invoke(const std::string& methodName, const std::string& args,
                      int callId) {
  try {
    std::cout << "[UIManager] Invoking: " << methodName << " with args: "
              << args << std::endl;

    if (methodName == "createView") {
      createView(args);
      sendSuccessCallback(callId, "");
    } else if (methodName == "updateView") {
      updateView(args);
      sendSuccessCallback(callId, "");
    } else if (methodName == "manageChildren") {
      manageChildren(args);
      sendSuccessCallback(callId, "");
    } else if (methodName == "setChildren") {
      setChildren(args);
      sendSuccessCallback(callId, "");
    } else if (methodName == "removeRootView") {
      removeRootView(args);
      sendSuccessCallback(callId, "");
    } else {
      sendErrorCallback(callId, "Unknown method: " + methodName);
    }
  } catch (const std::exception& e) {
    sendErrorCallback(callId, "Method invocation failed: " + std::string(e.what()));
  }
}

std::vector<int> UIManager::parseJSONArray(const std::string& json) const {
  std::vector<int> result;

  // 简单的 JSON 数组解析（假设格式为 [1,2,3]）
  if (json.empty() || json == "null" || json == "[]") {
    return result;
  }

  // 移除方括号
  std::string content = json;
  if (content.front() == '[' && content.back() == ']') {
    content = content.substr(1, content.length() - 2);
  }

  // 分割逗号
  std::stringstream ss(content);
  std::string item;

  while (std::getline(ss, item, ',')) {
    // 去除空格
    item.erase(0, item.find_first_not_of(" \t\n\r\f\v"));
    item.erase(item.find_last_not_of(" \t\n\r\f\v") + 1);

    if (!item.empty()) {
      try {
        result.push_back(std::stoi(item));
      } catch (const std::exception& e) {
        std::cerr << "[UIManager] Failed to parse integer: " << item << std::endl;
      }
    }
  }

  return result;
}

std::string UIManager::parseProps(const std::string& args) const {
  // 简单解析：假设 args 是 JSON 数组，props 在第二个位置
  // 格式: [tag, className, props, rootViewTag]
  if (args.empty() || args == "null") {
    return "{}";
  }

  // 查找第二个元素（props）
  size_t start = 0;
  int count = 0;
  bool inString = false;
  bool escapeNext = false;

  for (size_t i = 0; i < args.length(); ++i) {
    if (escapeNext) {
      escapeNext = false;
      continue;
    }

    char c = args[i];

    if (c == '\\') {
      escapeNext = true;
    } else if (c == '"') {
      inString = !inString;
    } else if (!inString && (c == ',' || c == '[')) {
      if (c == ',') {
        count++;
        if (count == 2) {
          start = i + 1;
          break;
        }
      }
    }
  }

  // 找到 props 的开始，现在找到它的结束
  if (start == 0) {
    return "{}";
  }

  size_t end = start;
  int braceCount = 0;
  inString = false;
  escapeNext = false;

  for (size_t i = start; i < args.length(); ++i) {
    if (escapeNext) {
      escapeNext = false;
      continue;
    }

    char c = args[i];

    if (c == '\\') {
      escapeNext = true;
    } else if (c == '"') {
      inString = !inString;
    } else if (!inString) {
      if (c == '{') {
        braceCount++;
      } else if (c == '}') {
        braceCount--;
        if (braceCount == 0) {
          end = i + 1;
          break;
        }
      }
    }
  }

  if (end > start) {
    return args.substr(start, end - start);
  }

  return "{}";
}

void UIManager::createView(const std::string& args) {
  // 参数格式: [tag, className, props, rootViewTag]
  // 简单解析
  std::vector<int> tags = parseJSONArray(args);
  if (tags.size() < 2) {
    std::cerr << "[UIManager] createView requires at least tag and className" << std::endl;
    return;
  }

  int tag = tags[0];

  // 提取 className（第二个参数，可能是字符串）
  size_t start = args.find(',') + 1;
  if (start != std::string::npos) {
    // 跳过空格
    while (start < args.length() && args[start] == ' ') start++;

    // 提取引号内的类名
    if (start < args.length() && args[start] == '"') {
      start++;
      size_t end = args.find('"', start);
      if (end != std::string::npos) {
        std::string className = args.substr(start, end - start);

        // 提取 props
        std::string props = parseProps(args);

        // 创建节点
        shadowTree_->createNode(tag, className, props);

        std::cout << "[UIManager] Created view: tag=" << tag
                  << ", className=" << className << std::endl;
        return;
      }
    }
  }

  std::cerr << "[UIManager] Failed to parse createView args: " << args << std::endl;
}

void UIManager::updateView(const std::string& args) {
  // 参数格式: [tag, className, props]
  std::vector<int> tags = parseJSONArray(args);
  if (tags.empty()) {
    std::cerr << "[UIManager] updateView requires tag" << std::endl;
    return;
  }

  int tag = tags[0];
  std::string props = parseProps(args);

  // 更新节点属性
  shadowTree_->updateProps(tag, props);

  std::cout << "[UIManager] Updated view: tag=" << tag << std::endl;
}

void UIManager::manageChildren(const std::string& /*args*/) {
  // 参数格式: [containerViewTag, moveFrom, moveTo, addChildTags, addAtIndices, removeFrom, removeAtIndices]
  // 这是一个复杂的方法，暂时简化实现
  std::cout << "[UIManager] manageChildren called (simplified implementation)" << std::endl;

  // TODO: 实现完整的 manageChildren 逻辑
  // 需要解析多个数组参数并执行相应的移动、添加、删除操作
}

void UIManager::setChildren(const std::string& args) {
  // 参数格式: [containerViewTag, reactTags]
  // 简单解析：提取第一个数字作为 containerViewTag，然后解析子数组

  // 找到第一个逗号，提取 containerViewTag
  size_t firstComma = args.find(',');
  if (firstComma == std::string::npos) {
    std::cerr << "[UIManager] setChildren requires containerViewTag and reactTags" << std::endl;
    return;
  }

  // 提取 containerViewTag
  std::string containerTagStr = args.substr(1, firstComma - 1); // 跳过开头的 '['
  int containerViewTag = std::stoi(containerTagStr);

  // 提取子标签数组部分
  size_t childrenStart = args.find('[', firstComma);
  if (childrenStart == std::string::npos) {
    std::cerr << "[UIManager] No children array found" << std::endl;
    return;
  }

  // 找到匹配的 ']'
  size_t childrenEnd = childrenStart;
  int bracketCount = 0;
  bool inString = false;
  bool escapeNext = false;

  for (size_t i = childrenStart; i < args.length(); ++i) {
    if (escapeNext) {
      escapeNext = false;
      continue;
    }

    char c = args[i];
    if (c == '\\') {
      escapeNext = true;
    } else if (c == '"') {
      inString = !inString;
    } else if (!inString) {
      if (c == '[') {
        bracketCount++;
      } else if (c == ']') {
        bracketCount--;
        if (bracketCount == 0) {
          childrenEnd = i + 1;
          break;
        }
      }
    }
  }

  std::string childrenArray = args.substr(childrenStart, childrenEnd - childrenStart);

  // 解析子标签
  std::vector<int> childTags;
  if (childrenArray.length() > 2) { // 不是 "[]"
    std::string content = childrenArray.substr(1, childrenArray.length() - 2); // 去掉方括号
    std::stringstream ss(content);
    std::string item;

    while (std::getline(ss, item, ',')) {
      // 去除空格
      item.erase(0, item.find_first_not_of(" \t\n\r\f\v"));
      item.erase(item.find_last_not_of(" \t\n\r\f\v") + 1);

      if (!item.empty()) {
        try {
          childTags.push_back(std::stoi(item));
        } catch (const std::exception& e) {
          std::cerr << "[UIManager] Failed to parse child tag: " << item << std::endl;
        }
      }
    }
  }

  // 清除现有子节点并添加新的
  auto container = shadowTree_->getNodeByTag(containerViewTag);
  if (container) {
    // 先清除所有子节点
    container->removeAllChildren();

    // 添加新的子节点
    for (int childTag : childTags) {
      shadowTree_->appendChild(containerViewTag, childTag);
    }

    std::cout << "[UIManager] Set children: container=" << containerViewTag
              << ", children=" << childTags.size() << std::endl;
  } else {
    std::cerr << "[UIManager] Container not found: " << containerViewTag << std::endl;
  }
}

void UIManager::removeRootView(const std::string& args) {
  // 参数格式: [rootViewTag]
  std::vector<int> tags = parseJSONArray(args);
  if (tags.empty()) {
    std::cerr << "[UIManager] removeRootView requires rootViewTag" << std::endl;
    return;
  }

  int rootViewTag = tags[0];

  // 清除整个树
  shadowTree_->setRootNode(nullptr);

  std::cout << "[UIManager] Removed root view: tag=" << rootViewTag << std::endl;
}

}  // namespace modules
}  // namespace mini_rn
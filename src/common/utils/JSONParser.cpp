#include "JSONParser.h"
#include "../bridge/JSCExecutor.h"  // 引入BridgeMessage定义

#include <chrono>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <algorithm>

namespace mini_rn {
namespace utils {

// === 核心解析方法 ===

mini_rn::bridge::BridgeMessage SimpleBridgeJSONParser::parseBridgeQueue(const std::string& jsonStr) {
    std::cout << "[JSONParser] Parsing Bridge queue JSON: " << jsonStr.substr(0, 100)
              << (jsonStr.length() > 100 ? "..." : "") << std::endl;

    mini_rn::bridge::BridgeMessage message;

    try {
        // 去除空白字符
        std::string trimmedStr = trim(jsonStr);

        // 验证外层数组格式
        if (trimmedStr.empty() || trimmedStr[0] != '[' || trimmedStr.back() != ']') {
            throw std::runtime_error("Invalid JSON format: not an array");
        }

        // 移除外层括号
        std::string innerContent = trimmedStr.substr(1, trimmedStr.length() - 2);

        // 分割顶层数组元素
        std::vector<std::string> topLevelArrays = splitTopLevelArrays(innerContent);

        // 验证数组数量（应该是4个：moduleIds, methodIds, params, callbackIds）
        if (topLevelArrays.size() != 4) {
            throw std::runtime_error("Invalid Bridge queue format: expected 4 arrays, got " +
                                    std::to_string(topLevelArrays.size()));
        }

        std::cout << "[JSONParser] Found 4 top-level arrays as expected" << std::endl;

        // 解析每个数组
        // 数组0：moduleIds（整数数组）
        message.moduleIds = parseIntArray(topLevelArrays[0]);
        std::cout << "[JSONParser] Parsed moduleIds: " << message.moduleIds.size() << " elements" << std::endl;

        // 数组1：methodIds（整数数组）
        message.methodIds = parseIntArray(topLevelArrays[1]);
        std::cout << "[JSONParser] Parsed methodIds: " << message.methodIds.size() << " elements" << std::endl;

        // 数组2：params（字符串数组，可能包含嵌套结构）
        message.params = parseStringArray(topLevelArrays[2]);
        std::cout << "[JSONParser] Parsed params: " << message.params.size() << " elements" << std::endl;

        // 数组3：callbackIds（整数数组，可能包含null/-1）
        message.callbackIds = parseIntArray(topLevelArrays[3]);
        std::cout << "[JSONParser] Parsed callbackIds: " << message.callbackIds.size() << " elements" << std::endl;

        // 验证消息格式
        if (!message.isValid()) {
            throw std::runtime_error("Invalid Bridge message: array lengths don't match");
        }

        std::cout << "[JSONParser] Successfully parsed Bridge message with "
                  << message.getCallCount() << " calls" << std::endl;

        return message;

    } catch (const std::exception& e) {
        std::cout << "[JSONParser] Error parsing JSON: " << e.what() << std::endl;
        throw;
    }
}

// === 数组解析辅助方法 ===

std::vector<int> SimpleBridgeJSONParser::parseIntArray(const std::string& arrayStr) {
    std::vector<int> result;
    std::string trimmedStr = trim(arrayStr);

    // 处理空数组
    if (trimmedStr == "[]") {
        return result;
    }

    // 验证数组格式
    if (trimmedStr.empty() || trimmedStr[0] != '[' || trimmedStr.back() != ']') {
        throw std::runtime_error("Invalid array format: " + arrayStr);
    }

    // 移除括号
    std::string content = trimmedStr.substr(1, trimmedStr.length() - 2);
    content = trim(content);

    // 处理空内容
    if (content.empty()) {
        return result;
    }

    // 分割元素
    std::stringstream ss(content);
    std::string element;

    while (std::getline(ss, element, ',')) {
        element = trim(element);

        if (element.empty()) {
            continue;
        }

        // 处理null值（React Native中callbackId可能为null）
        if (element == "null" || element == "undefined") {
            result.push_back(-1);  // 使用-1表示无效的回调ID
        } else if (isInteger(element)) {
            result.push_back(std::stoi(element));
        } else {
            std::cout << "[JSONParser] Warning: Non-integer element '" << element
                      << "' in int array, treating as -1" << std::endl;
            result.push_back(-1);
        }
    }

    return result;
}

std::vector<std::string> SimpleBridgeJSONParser::parseStringArray(const std::string& arrayStr) {
    std::vector<std::string> result;
    std::string trimmedStr = trim(arrayStr);

    // 处理空数组
    if (trimmedStr == "[]") {
        return result;
    }

    // 验证数组格式
    if (trimmedStr.empty() || trimmedStr[0] != '[' || trimmedStr.back() != ']') {
        throw std::runtime_error("Invalid array format: " + arrayStr);
    }

    // 移除外层括号
    std::string content = trimmedStr.substr(1, trimmedStr.length() - 2);

    // 对于params数组，每个元素可能是：
    // 1. 字符串字面量："hello"
    // 2. 嵌套数组：["param1", "param2"]
    // 3. 空数组：[]
    // 4. null值：null

    size_t pos = 0;
    while (pos < content.length()) {
        // 跳过空白和逗号
        while (pos < content.length() && (content[pos] == ' ' || content[pos] == '\t' ||
               content[pos] == '\n' || content[pos] == '\r' || content[pos] == ',')) {
            pos++;
        }

        if (pos >= content.length()) {
            break;
        }

        std::string element;

        if (content[pos] == '[') {
            // 嵌套数组：找到匹配的']'
            size_t endPos = findMatchingBracket(content, pos);
            element = content.substr(pos, endPos - pos + 1);
            pos = endPos + 1;
        } else if (content[pos] == '"') {
            // 字符串字面量：找到匹配的'"'
            pos++; // 跳过开始的引号
            size_t startPos = pos;
            while (pos < content.length() && content[pos] != '"') {
                if (content[pos] == '\\') {
                    pos += 2; // 跳过转义字符
                } else {
                    pos++;
                }
            }
            if (pos >= content.length()) {
                throw std::runtime_error("Unterminated string literal");
            }
            element = content.substr(startPos, pos - startPos);
            pos++; // 跳过结束的引号
        } else {
            // 其他值（null, 数字等）
            size_t startPos = pos;
            while (pos < content.length() && content[pos] != ',' && content[pos] != ']') {
                pos++;
            }
            element = trim(content.substr(startPos, pos - startPos));
        }

        if (!element.empty()) {
            result.push_back(element);
        }
    }

    return result;
}

// === 辅助工具方法 ===

size_t SimpleBridgeJSONParser::findMatchingBracket(const std::string& str, size_t startPos) {
    if (startPos >= str.length() || str[startPos] != '[') {
        throw std::runtime_error("Invalid start position for bracket matching");
    }

    int bracketCount = 1;
    size_t pos = startPos + 1;

    while (pos < str.length() && bracketCount > 0) {
        if (str[pos] == '[') {
            bracketCount++;
        } else if (str[pos] == ']') {
            bracketCount--;
        } else if (str[pos] == '"') {
            // 跳过字符串字面量
            pos++;
            while (pos < str.length() && str[pos] != '"') {
                if (str[pos] == '\\') {
                    pos += 2; // 跳过转义字符
                } else {
                    pos++;
                }
            }
        }
        pos++;
    }

    if (bracketCount != 0) {
        throw std::runtime_error("Unmatched brackets in JSON");
    }

    return pos - 1; // 返回匹配的']'位置
}

std::vector<std::string> SimpleBridgeJSONParser::splitTopLevelArrays(const std::string& arrayStr) {
    std::vector<std::string> result;
    size_t pos = 0;

    while (pos < arrayStr.length()) {
        // 跳过空白和逗号
        while (pos < arrayStr.length() && (arrayStr[pos] == ' ' || arrayStr[pos] == '\t' ||
               arrayStr[pos] == '\n' || arrayStr[pos] == '\r' || arrayStr[pos] == ',')) {
            pos++;
        }

        if (pos >= arrayStr.length()) {
            break;
        }

        if (arrayStr[pos] == '[') {
            // 找到数组的结束位置
            size_t endPos = findMatchingBracket(arrayStr, pos);
            std::string arrayElement = arrayStr.substr(pos, endPos - pos + 1);
            result.push_back(arrayElement);
            pos = endPos + 1;
        } else {
            // 非数组元素，找到下一个逗号或结束
            size_t startPos = pos;
            int bracketCount = 0;
            bool inString = false;

            while (pos < arrayStr.length()) {
                if (!inString) {
                    if (arrayStr[pos] == '[') {
                        bracketCount++;
                    } else if (arrayStr[pos] == ']') {
                        bracketCount--;
                    } else if (arrayStr[pos] == '"') {
                        inString = true;
                    } else if (arrayStr[pos] == ',' && bracketCount == 0) {
                        break;
                    }
                } else {
                    if (arrayStr[pos] == '"' && (pos == 0 || arrayStr[pos-1] != '\\')) {
                        inString = false;
                    }
                }
                pos++;
            }

            std::string element = trim(arrayStr.substr(startPos, pos - startPos));
            if (!element.empty()) {
                result.push_back(element);
            }
        }
    }

    return result;
}

std::string SimpleBridgeJSONParser::trim(const std::string& str) {
    size_t start = 0;
    size_t end = str.length();

    // 找到第一个非空白字符
    while (start < end && (str[start] == ' ' || str[start] == '\t' ||
           str[start] == '\n' || str[start] == '\r')) {
        start++;
    }

    // 找到最后一个非空白字符
    while (end > start && (str[end-1] == ' ' || str[end-1] == '\t' ||
           str[end-1] == '\n' || str[end-1] == '\r')) {
        end--;
    }

    return str.substr(start, end - start);
}

bool SimpleBridgeJSONParser::isInteger(const std::string& str) {
    if (str.empty()) {
        return false;
    }

    size_t start = 0;
    if (str[0] == '-') {
        start = 1;
        if (str.length() == 1) {
            return false;
        }
    }

    for (size_t i = start; i < str.length(); i++) {
        if (str[i] < '0' || str[i] > '9') {
            return false;
        }
    }

    return true;
}

// === 性能测量方法 ===

long SimpleBridgeJSONParser::measureParsingTime(const std::string& jsonStr) {
    Timer timer;

    try {
        parseBridgeQueue(jsonStr);
        return timer.getElapsedMicroseconds();
    } catch (const std::exception& e) {
        std::cout << "[JSONParser] Performance test failed: " << e.what() << std::endl;
        return -1;
    }
}

std::string SimpleBridgeJSONParser::generateTestBridgeJSON(int callCount, int paramSize) {
    std::ostringstream oss;

    oss << "[";

    // moduleIds
    oss << "[";
    for (int i = 0; i < callCount; i++) {
        if (i > 0) oss << ",";
        oss << (i % 10 + 1); // 模块ID 1-10
    }
    oss << "],";

    // methodIds
    oss << "[";
    for (int i = 0; i < callCount; i++) {
        if (i > 0) oss << ",";
        oss << (i % 5 + 1); // 方法ID 1-5
    }
    oss << "],";

    // params
    oss << "[";
    for (int i = 0; i < callCount; i++) {
        if (i > 0) oss << ",";
        oss << "[";
        for (int j = 0; j < paramSize; j++) {
            if (j > 0) oss << ",";
            oss << "\"param" << i << "_" << j << "\"";
        }
        oss << "]";
    }
    oss << "],";

    // callbackIds
    oss << "[";
    for (int i = 0; i < callCount; i++) {
        if (i > 0) oss << ",";
        oss << (i < callCount / 2 ? i + 100 : -1); // 一半有回调，一半没有
    }
    oss << "]";

    oss << "]";

    return oss.str();
}

// === Timer实现 ===

SimpleBridgeJSONParser::Timer::Timer()
    : m_start(std::chrono::high_resolution_clock::now()) {
}

long SimpleBridgeJSONParser::Timer::getElapsedMicroseconds() const {
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - m_start);
    return duration.count();
}

}  // namespace utils
}  // namespace mini_rn
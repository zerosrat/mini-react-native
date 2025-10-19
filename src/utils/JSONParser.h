#ifndef JSONPARSER_H
#define JSONPARSER_H

#include <string>
#include <vector>

// 前向声明Bridge消息结构（避免循环依赖）
namespace mini_rn {
namespace bridge {
struct BridgeMessage;
}
}

namespace mini_rn {
namespace utils {

/**
 * SimpleBridgeJSONParser - 专门用于解析React Native Bridge消息的简化JSON解析器
 *
 * 设计目标：
 * 1. 专门针对Bridge消息格式：[[moduleIds], [methodIds], [params], [callbackIds]]
 * 2. 手撸实现，深度理解JSON序列化/反序列化的性能开销
 * 3. 学习React Native Bridge通信的性能瓶颈
 * 4. 为后续性能优化积累实战经验
 *
 * 支持的数据格式：
 * - 外层数组：[array1, array2, array3, array4]
 * - 整数数组：[1, 2, 3]
 * - 字符串数组：["hello", "world"]
 * - 混合数组：[["param1", "param2"], []]
 *
 * 不支持的复杂格式（暂时）：
 * - 嵌套对象：{key: value}
 * - 深层嵌套数组
 * - 特殊字符转义
 */
class SimpleBridgeJSONParser {
public:
    /**
     * 解析Bridge队列JSON字符串为BridgeMessage结构
     *
     * @param jsonStr JSON字符串，格式：[[1,2],[3,4],[["hello"],[]]],[100,-1]]
     * @return BridgeMessage 解析后的Bridge消息结构
     * @throws std::runtime_error 解析失败时抛出异常
     *
     * 示例输入：
     * "[[1,2],[3,4],[["hello","world"],[]]],[100,-1]]"
     *
     * 解析结果：
     * - moduleIds: [1, 2]
     * - methodIds: [3, 4]
     * - params: [["hello","world"], []]
     * - callbackIds: [100, -1]
     */
    static mini_rn::bridge::BridgeMessage parseBridgeQueue(const std::string& jsonStr);

    /**
     * 性能测量相关方法（学习用）
     */

    /**
     * 测量解析性能
     * @param jsonStr 要解析的JSON字符串
     * @return 解析耗时（微秒）
     */
    static long measureParsingTime(const std::string& jsonStr);

    /**
     * 生成测试用的Bridge JSON字符串
     * @param callCount 调用数量
     * @param paramSize 每个参数的大小
     * @return 测试用的JSON字符串
     */
    static std::string generateTestBridgeJSON(int callCount, int paramSize);

private:
    /**
     * 私有辅助方法
     */

    /**
     * 解析整数数组
     * @param arrayStr 数组字符串，如 "[1,2,3]"
     * @return 整数向量
     */
    static std::vector<int> parseIntArray(const std::string& arrayStr);

    /**
     * 解析字符串数组（可能包含嵌套数组）
     * @param arrayStr 数组字符串，如 "[\"hello\",\"world\"]" 或 "[[\"a\",\"b\"],[]]"
     * @return 字符串向量（嵌套数组会被序列化为字符串）
     */
    static std::vector<std::string> parseStringArray(const std::string& arrayStr);

    /**
     * 查找匹配的括号位置
     * @param str 输入字符串
     * @param startPos 开始位置（应该是'['字符）
     * @return 匹配的']'字符位置
     */
    static size_t findMatchingBracket(const std::string& str, size_t startPos);

    /**
     * 分割顶层数组元素
     * @param arrayStr 数组字符串，如 "[[1,2],[3,4]]"
     * @return 分割后的子数组字符串向量
     */
    static std::vector<std::string> splitTopLevelArrays(const std::string& arrayStr);

    /**
     * 去除字符串两端的空白字符
     * @param str 输入字符串
     * @return 去除空白后的字符串
     */
    static std::string trim(const std::string& str);

    /**
     * 解析字符串字面量（处理转义字符）
     * @param str 带引号的字符串，如 "\"hello\""
     * @return 不带引号的字符串，如 "hello"
     */
    static std::string parseStringLiteral(const std::string& str);

    /**
     * 检查字符串是否为整数
     * @param str 输入字符串
     * @return 是否为整数
     */
    static bool isInteger(const std::string& str);

    /**
     * 性能计时器辅助类
     */
    class Timer {
    public:
        Timer();
        long getElapsedMicroseconds() const;
    private:
        std::chrono::high_resolution_clock::time_point m_start;
    };
};

}  // namespace utils
}  // namespace mini_rn

#endif  // JSONPARSER_H
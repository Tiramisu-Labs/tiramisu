#pragma once

#include <unordered_map>
#include <string>
#include <variant>
#include <memory>

enum class NodeType : int8_t {
    Null,
    String,
    Number,
    Node
};

class YAML;

class YAMLNode {
    private:
    std::variant<std::monostate, std::string, double, std::unique_ptr<YAML>> value;
    NodeType type = NodeType::Null;

    public:
    YAMLNode() = default;
    YAMLNode(YAMLNode&&) noexcept = default;
    YAMLNode& operator=(YAMLNode&&) noexcept = default;
    YAMLNode(const YAMLNode&) = delete;
    YAMLNode& operator=(const YAMLNode&) = delete;

    // YAMLNode clone() const;

    YAMLNode& operator=(double num) {
        type = NodeType::Number;
        value = num;
        return *this;
    }

    YAMLNode& operator=(const std::string& str) {
        type = NodeType::String;
        value = str;
        return *this;
    }

    NodeType getType() const { return type; }
    
    bool isNull()   const { return type == NodeType::Null; }
    bool isNumber() const { return type == NodeType::Number; }
    bool isString() const { return type == NodeType::String; }
    bool isNode()   const { return type == NodeType::Node; }

    template<typename T>
    T as() const {
        return std::get<T>(value);
    }

    YAMLNode& operator[](const std::string& key);
};

class YAML
{
    private:
    std::unordered_map<std::string, YAMLNode> map;

    public:

    YAML() = default;
    
    YAML(YAML&&) noexcept = default;
    YAML& operator=(YAML&&) noexcept = default;
    YAML(const YAML&) = delete;
    YAML& operator=(const YAML&) = delete;

    YAMLNode& operator[](const std::string& key) { return map[key]; }
    
};

inline YAMLNode& YAMLNode::operator[](const std::string& key) {
    if (type == NodeType::Null) {
        type = NodeType::Node;
        value = std::make_unique<YAML>();
    }

    if (type == NodeType::Node) {
        return (*std::get<std::unique_ptr<YAML>>(value))[key];
    }

    throw std::runtime_error("Error: Cannot use [] on a scalar value (String/Number)");
}
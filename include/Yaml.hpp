#pragma once

#include <unordered_map>
#include <string>
#include <variant>
#include <memory>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stack>
#include <algorithm>
#include <stdexcept>
#include <type_traits>

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

    auto begin();
    auto end();
    auto begin() const;
    auto end() const;

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
        if constexpr (std::is_integral_v<T> && !std::is_same_v<T, bool>) {
            return static_cast<T>(std::get<double>(value));
        } else {
            return std::get<T>(value);
        }
    }

    YAML* getNestedNode() const {
        if (type == NodeType::Node) {
            return std::get<std::unique_ptr<YAML>>(value).get();
        }
        return nullptr;
    }

    void makeNode() {
        if (type == NodeType::Null) {
            type = NodeType::Node;
            value = std::make_unique<YAML>();
        }
    }

    YAMLNode& operator[](const std::string& key);
    const YAMLNode& operator[](const std::string& key) const;
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
    
    const YAMLNode& operator[](const std::string& key) const {
        auto it = map.find(key);
        if (it != map.end()) return it->second;
        static const YAMLNode null_node;
        return null_node;
    }

    auto begin() noexcept { return map.begin(); }
    auto end() noexcept   { return map.end(); }

    auto begin() const noexcept { return map.begin(); }
    auto end() const noexcept   { return map.end(); }
    
    auto cbegin() const noexcept { return map.cbegin(); }
    auto cend() const noexcept   { return map.cend(); }

    static YAML parseYaml(const std::filesystem::path& path);
};

inline auto YAMLNode::begin() {
    if (type != NodeType::Node) throw std::runtime_error("Cannot iterate over a scalar YAMLNode");
    return std::get<std::unique_ptr<YAML>>(value)->begin();
}

inline auto YAMLNode::end() {
    if (type != NodeType::Node) throw std::runtime_error("Cannot iterate over a scalar YAMLNode");
    return std::get<std::unique_ptr<YAML>>(value)->end();
}

inline auto YAMLNode::begin() const {
    if (type != NodeType::Node) throw std::runtime_error("Cannot iterate over a scalar YAMLNode");
    const YAML* yamlPtr = std::get<std::unique_ptr<YAML>>(value).get();
    return yamlPtr->begin();
}

inline auto YAMLNode::end() const {
    if (type != NodeType::Node) throw std::runtime_error("Cannot iterate over a scalar YAMLNode");
    const YAML* yamlPtr = std::get<std::unique_ptr<YAML>>(value).get();
    return yamlPtr->end();
}

inline YAMLNode& YAMLNode::operator[](const std::string& key) {
    makeNode();
    if (type == NodeType::Node) {
        return (*std::get<std::unique_ptr<YAML>>(value))[key];
    }
    throw std::runtime_error("Error: Cannot use [] on a scalar value (String/Number)");
}

inline const YAMLNode& YAMLNode::operator[](const std::string& key) const {
    if (type == NodeType::Node) {
        const YAML* yamlPtr = std::get<std::unique_ptr<YAML>>(value).get();
        return (*yamlPtr)[key];
    }
    static const YAMLNode null_node;
    return null_node;
}

inline std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

inline YAML YAML::parseYaml(const std::filesystem::path& path)
{
    YAML root;
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open YAML file: " + path.string());
    }

    std::string line;
    std::stack<std::pair<size_t, YAML*>> nodeStack;
    
    while (std::getline(file, line)) {
        std::string trimmed = trim(line);
        if (trimmed.empty() || trimmed[0] == '#') {
            continue;
        }

        size_t indent = line.find_first_not_of(" ");
        if (indent == std::string::npos) indent = 0;

        size_t colonPos = trimmed.find(':');
        if (colonPos == std::string::npos) {
            continue;
        }

        std::string key = trim(trimmed.substr(0, colonPos));
        std::string valStr = trim(trimmed.substr(colonPos + 1));

        while (!nodeStack.empty() && indent <= nodeStack.top().first) {
            nodeStack.pop();
        }

        YAMLNode* currentNode = nullptr;

        if (nodeStack.empty()) {
            root.map[key] = YAMLNode();
            currentNode = &root.map[key];
        } else {
            YAML* currentParentYaml = nodeStack.top().second;
            (*currentParentYaml)[key] = YAMLNode();
            currentNode = &((*currentParentYaml)[key]);
        }

        if (!valStr.empty()) {
            try {
                size_t idx;
                double num = std::stod(valStr, &idx);
                if (idx == valStr.size()) {
                    *currentNode = num;
                } else {
                    *currentNode = valStr;
                }
            } catch (...) {
                if ((valStr.front() == '"' && valStr.back() == '"') || 
                    (valStr.front() == '\'' && valStr.back() == '\'')) {
                    valStr = valStr.substr(1, valStr.size() - 2);
                }
                *currentNode = valStr;
            }
        } else {
            currentNode->makeNode(); 
            YAML* childYaml = currentNode->getNestedNode();
            nodeStack.push({indent, childYaml});
        }
    }

    return root;
}
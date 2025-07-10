#pragma once

class Token
{
    private:
    ETypes m_type;
    std::string m_value;
    std::string m_metadata;

    public:
    Token(ETypes t, std::string&& s) : m_type(t), m_value(std::move(s)) {}
    Token(ETypes t, std::string&& s, std::string&& metadata)
        : m_type(t), m_value(std::move(s)), m_metadata(std::move(metadata)) {}
    
    /* Default constructor for bool flag*/
    Token(ETypes t, std::string&& s, bool b)
        : m_type(t), m_value(std::move(s)), m_metadata(b ? "bool": "") {}

    std::string getValue() { return m_value; }
    std::string getValue() const { return m_value; }
    std::string getMetadata() { return m_metadata; }
    std::string getMetadata() const { return m_metadata; }
    ETypes getType() { return m_type; }
    ETypes getType() const { return m_type; }
};



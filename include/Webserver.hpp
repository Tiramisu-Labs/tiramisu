#pragma once

#include <string>
#include <memory>
#include <vector>

class SshHandler;
class Token;

enum class EWebCommands
{
    UNRECOGNIZED,
    LOGIN,
    START,
    RUN,
    RELOAD,
    CONFIG
};

class Webserver
{
    private:
    std::unique_ptr<SshHandler> handler;
    
    public:
    Webserver();
    ~Webserver();

    void exec(std::string address, std::string cmd, std::vector<Token> flags, int port);
    void upload(const std::string address, const std::string password, int port, const std::string path);
};
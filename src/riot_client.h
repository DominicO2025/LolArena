#pragma once
#include <string>

class RiotClient {
public:
    explicit RiotClient(const std::string& apiKey);

    std::string getAccountByRiotId(
        const std::string& region,
        const std::string& gameName,
        const std::string& tagLine
    );

private:
    std::string apiKey_;
};
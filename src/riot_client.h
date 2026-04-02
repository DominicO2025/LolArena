#pragma once
#include <string>

struct RiotAccount {
    std::string gameName;
    std::string tagLine;
    std::string puuid;
};

class RiotClient {
public:
    explicit RiotClient(const std::string& apiKey);

    RiotAccount getAccountByRiotId(
        const std::string& region,
        const std::string& gameName,
        const std::string& tagLine
    );

private:
    std::string apiKey_;
};
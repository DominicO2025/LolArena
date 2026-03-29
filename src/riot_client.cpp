#include "riot_client.h"

RiotClient::RiotClient(const std::string& apiKey)
    : apiKey_(apiKey) {}

std::string RiotClient::getAccountByRiotId(
    const std::string& region,
    const std::string& gameName,
    const std::string& tagLine
) {
    return R"({
        "gameName": ")" + gameName + R"(",
        "tagLine": ")" + tagLine + R"(",
        "puuid": "fake-puuid"
    })";
}
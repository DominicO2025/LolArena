#include "riot_client.h"

#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include <sstream>
#include <stdexcept>
#include <algorithm>

using json = nlohmann::json;

// ── CURL helpers ─────────────────────────────────────────────────────────────

static size_t writeCallback(char* ptr, size_t size, size_t nmemb, std::string* out) {
    out->append(ptr, size * nmemb);
    return size * nmemb;
}

std::string RiotClient::httpGet(const std::string& url) {
    CURL* curl = curl_easy_init();
    if (!curl) throw std::runtime_error("curl_easy_init failed");

    std::string response;
    struct curl_slist* headers = nullptr;
    std::string authHeader = "X-Riot-Token: " + apiKey_;
    headers = curl_slist_append(headers, authHeader.c_str());

    curl_easy_setopt(curl, CURLOPT_URL,            url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER,     headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,  writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA,      &response);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT,        15L);

    CURLcode res = curl_easy_perform(curl);
    long httpCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK)
        throw std::runtime_error(std::string("curl error: ") + curl_easy_strerror(res));
    if (httpCode == 401)
        throw std::runtime_error("Riot API: Unauthorized – check RIOT_API_KEY");
    if (httpCode == 403)
        throw std::runtime_error("Riot API: Forbidden – key may lack permissions");
    if (httpCode == 404)
        throw std::runtime_error("Riot API: 404 Not Found – " + url);
    if (httpCode == 429)
        throw std::runtime_error("Riot API: Rate-limited (429)");
    if (httpCode < 200 || httpCode >= 300)
        throw std::runtime_error("Riot API: HTTP " + std::to_string(httpCode));

    return response;
}

// ── Region helpers ────────────────────────────────────────────────────────────

// Maps platform region (na1, euw1, kr, …) → routing region (americas, europe, asia)
std::string RiotClient::toRoutingRegion(const std::string& r) {
    std::string lo = r;
    std::transform(lo.begin(), lo.end(), lo.begin(), ::tolower);

    if (lo == "na1"  || lo == "na"  || lo == "br1"  || lo == "br"  ||
        lo == "la1"  || lo == "la2" || lo == "americas")
        return "americas";

    if (lo == "euw1" || lo == "eune1" || lo == "eun1" || lo == "tr1" ||
        lo == "ru"   || lo == "eu"    || lo == "europe")
        return "europe";

    if (lo == "kr"   || lo == "jp1"  || lo == "jp"   || lo == "asia")
        return "asia";

    if (lo == "oc1"  || lo == "sg2"  || lo == "tw2"  || lo == "vn2" ||
        lo == "sea"  || lo == "sea2" || lo == "sea4"  || lo == "sea5")
        return "sea";

    // fallback: assume caller already passed a routing region
    return lo;
}

// ── Constructor ───────────────────────────────────────────────────────────────

RiotClient::RiotClient(const std::string& apiKey)
    : apiKey_(apiKey)
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

// ── Account API ───────────────────────────────────────────────────────────────

RiotAccount RiotClient::getAccountByRiotId(
    const std::string& region,
    const std::string& gameName,
    const std::string& tagLine
) {
    std::string routing = toRoutingRegion(region);

    // URL-encode the name/tag (simple: spaces → %20; Crow handles most already)
    // For a production build use curl_easy_escape; minimal version here.
    std::string url = "https://" + routing +
                      ".api.riotgames.com/riot/account/v1/accounts/by-riot-id/" +
                      gameName + "/" + tagLine;

    std::string body = httpGet(url);
    json j           = json::parse(body);

    RiotAccount acc;
    acc.gameName = j.value("gameName", "");
    acc.tagLine  = j.value("tagLine",  "");
    acc.puuid    = j.value("puuid",    "");
    return acc;
}

// ── Match IDs ─────────────────────────────────────────────────────────────────

// Arena queue ID is 1700
std::vector<std::string> RiotClient::getArenaMatchIds(
    const std::string& region,
    const std::string& puuid,
    int count
) {
    std::string routing = toRoutingRegion(region);
    // Fetch in batches of 100 (API max)
    count = std::min(count, 100);

    std::string url = "https://" + routing +
                      ".api.riotgames.com/lol/match/v5/matches/by-puuid/" +
                      puuid + "/ids?queue=1700&type=ranked&start=0&count=" +
                      std::to_string(count);

    std::string body = httpGet(url);
    json j           = json::parse(body);

    std::vector<std::string> ids;
    for (auto& el : j) ids.push_back(el.get<std::string>());
    return ids;
}

// ── Single Match Entry ────────────────────────────────────────────────────────

ArenaGameEntry RiotClient::getArenaMatchEntry(
    const std::string& region,
    const std::string& matchId,
    const std::string& puuid
) {
    std::string routing = toRoutingRegion(region);
    std::string url = "https://" + routing +
                      ".api.riotgames.com/lol/match/v5/matches/" + matchId;

    std::string body = httpGet(url);
    json j           = json::parse(body);

    const auto& info         = j.at("info");
    int gameDuration         = info.value("gameDuration", 0);
    long long gameCreation   = info.value("gameCreation", 0LL);
    const auto& participants = info.at("participants");

    for (const auto& p : participants) {
        if (p.value("puuid", "") != puuid) continue;

        ArenaGameEntry entry;
        entry.championName    = p.value("championName",  "Unknown");
        entry.placement       = p.value("placement",     8);
        entry.gameDurationSecs= gameDuration;
        entry.gameCreation    = gameCreation;
        entry.win             = (entry.placement <= 2); // top-2 = win in Arenas

        return entry;
    }

    throw std::runtime_error("PUUID not found in match " + matchId);
}
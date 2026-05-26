#include "player_handler.h"

#include <nlohmann/json.hpp>
#include <algorithm>
#include <map>
#include <string>
#include <vector>

using json = nlohmann::json;

// ── GET /api/player?name=X&tag=Y&region=Z ────────────────────────────────────
// Returns the Riot account (gameName, tagLine, puuid) for a given Riot ID.

crow::response handlePlayerRoute(const crow::request& req, RiotClient& riotClient) {
    auto name   = req.url_params.get("name");
    auto tag    = req.url_params.get("tag");
    auto region = req.url_params.get("region");

    if (!name)   return crow::response(400, "Missing 'name' parameter");
    if (!tag)    return crow::response(400, "Missing 'tag' parameter");
    if (!region) return crow::response(400, "Missing 'region' parameter");

    try {
        RiotAccount acc = riotClient.getAccountByRiotId(region, name, tag);

        crow::json::wvalue result;
        result["gameName"] = acc.gameName;
        result["tagLine"]  = acc.tagLine;
        result["puuid"]    = acc.puuid;
        return crow::response(result);

    } catch (const std::exception& e) {
        crow::json::wvalue err;
        err["error"] = std::string(e.what());
        return crow::response(500, err);
    }
}

// ── GET /api/player/arena-stats?puuid=X&region=Y&count=Z ─────────────────────
// Returns champions sorted by total hours played in Arena, with wins/games.
//
// Optional: pass count=N (1-100) to control how many recent matches to scan.

crow::response handleArenaStatsRoute(const crow::request& req, RiotClient& riotClient) {
    auto puuid  = req.url_params.get("puuid");
    auto region = req.url_params.get("region");
    auto countP = req.url_params.get("count");

    if (!puuid)  return crow::response(400, "Missing 'puuid' parameter");
    if (!region) return crow::response(400, "Missing 'region' parameter");

    int count = 100;
    if (countP) {
        try { count = std::stoi(countP); }
        catch (...) { return crow::response(400, "'count' must be an integer"); }
        if (count < 1 || count > 100)
            return crow::response(400, "'count' must be between 1 and 100");
    }

    try {
        // 1. Fetch Arena match IDs
        std::vector<std::string> matchIds =
            riotClient.getArenaMatchIds(region, puuid, count);

        // 2. Aggregate per-champion stats
        std::map<std::string, ChampionArenaStats> statsMap;

        for (const auto& matchId : matchIds) {
            try {
                ArenaGameEntry entry =
                    riotClient.getArenaMatchEntry(region, matchId, puuid);

                auto& s = statsMap[entry.championName];
                s.championName  = entry.championName;
                s.gamesPlayed  += 1;
                s.wins         += entry.win ? 1 : 0;
                s.totalSeconds += entry.gameDurationSecs;
                s.totalHours    = s.totalSeconds / 3600.0;

            } catch (const std::exception& matchErr) {
                // Skip individual bad matches rather than failing the whole request
                // (e.g. a match that was remade or has missing data)
                continue;
            }
        }

        // 3. Sort by totalHours descending
        std::vector<ChampionArenaStats> sorted;
        sorted.reserve(statsMap.size());
        for (auto& [name, stat] : statsMap) sorted.push_back(stat);

        std::sort(sorted.begin(), sorted.end(),
            [](const ChampionArenaStats& a, const ChampionArenaStats& b) {
                return a.totalSeconds > b.totalSeconds;
            });

        // 4. Build JSON response
        crow::json::wvalue result;
        result["puuid"]        = std::string(puuid);
        result["matchesScanned"] = static_cast<int>(matchIds.size());
        result["totalChampions"] = static_cast<int>(sorted.size());

        std::vector<crow::json::wvalue> champArr;
        champArr.reserve(sorted.size());

        for (const auto& s : sorted) {
            crow::json::wvalue c;
            c["champion"]   = s.championName;
            c["gamesPlayed"]= s.gamesPlayed;
            c["wins"]       = s.wins;
            c["winRate"]    = s.gamesPlayed > 0
                                  ? std::round((s.wins * 100.0 / s.gamesPlayed) * 10) / 10
                                  : 0.0;
            c["totalHours"] = std::round(s.totalHours * 100) / 100;  // 2 d.p.
            c["totalMinutes"]= static_cast<int>(s.totalSeconds / 60);
            champArr.push_back(std::move(c));
        }

        result["champions"] = std::move(champArr);
        return crow::response(result);

    } catch (const std::exception& e) {
        crow::json::wvalue err;
        err["error"] = std::string(e.what());
        return crow::response(500, err);
    }
}
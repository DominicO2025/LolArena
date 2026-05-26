#pragma once
#include <string>
#include <vector>
#include <stdexcept>
 
// ── Data Structures ──────────────────────────────────────────────────────────
 
struct RiotAccount {
    std::string gameName;
    std::string tagLine;
    std::string puuid;
};
 
// One Arena game entry for a specific participant
struct ArenaGameEntry {
    std::string championName;
    int    placement;       // 1–8 (1 or 2 = win/top-2 in duo)
    int    gameDurationSecs;
    bool   win;             // placement <= 2
    long long gameCreation; // epoch ms
};
 
// Per-champion aggregated stats
struct ChampionArenaStats {
    std::string championName;
    int    gamesPlayed  = 0;
    int    wins         = 0;       // top-2 placements
    int    totalSeconds = 0;       // total time in game with this champ
    double totalHours   = 0.0;
};
 
// ── RiotClient ───────────────────────────────────────────────────────────────
 
class RiotClient {
public:
    explicit RiotClient(const std::string& apiKey);
 
    // Riot Account API  (americas / europe / asia routing)
    RiotAccount getAccountByRiotId(
        const std::string& routingRegion,   // e.g. "americas"
        const std::string& gameName,
        const std::string& tagLine
    );
 
    // Match history: returns list of matchIds filtered to Arena (queue 1700)
    std::vector<std::string> getArenaMatchIds(
        const std::string& routingRegion,   // e.g. "americas"
        const std::string& puuid,
        int count = 100                     // max matches to fetch
    );
 
    // Parse a single match and return the participant entry for puuid
    ArenaGameEntry getArenaMatchEntry(
        const std::string& routingRegion,
        const std::string& matchId,
        const std::string& puuid
    );
 
private:
    std::string apiKey_;
 
    // Low-level HTTPS GET; throws std::runtime_error on failure
    std::string httpGet(const std::string& url);
 
    // Map user-facing region (na1, euw1 …) to routing region (americas, europe …)
    static std::string toRoutingRegion(const std::string& platformRegion);
};
 
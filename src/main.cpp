#define ASIO_STANDALONE
#include "crow.h"
#include <cstdlib>
#include <iostream>
#include <string>
#include <fstream>

#include "player_handler.h"
#include "riot_client.h"

int main() {
    const char* riotApiKey = std::getenv("RIOT_API_KEY");

    if (!riotApiKey) {
        std::cerr << "RIOT_API_KEY not set\n";
        return 1;
    }

    RiotClient riotClient(riotApiKey);
    crow::SimpleApp app;

    // ── Health check ────────────────────────────────────────────────────────
    CROW_ROUTE(app, "/")([]() {
        std::ifstream file("index.html");
        if (!file) return crow::response(404, "index.html not found");
        std::string body((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
        crow::response res(body);
        res.set_header("Content-Type", "text/html");
        return res;
    });

    // ── Step 1: resolve Riot ID → PUUID ─────────────────────────────────────
    // GET /api/player?name=<gameName>&tag=<tagLine>&region=<na1|euw1|kr|…>
    //
    // Example: /api/player?name=Faker&tag=KR1&region=kr
    //
    // Returns: { gameName, tagLine, puuid }
    CROW_ROUTE(app, "/api/player")
    ([&riotClient](const crow::request& req) {
        return handlePlayerRoute(req, riotClient);
    });

    // ── Step 2: fetch Arena champion stats for a PUUID ───────────────────────
    // GET /api/player/arena-stats?puuid=<puuid>&region=<region>&count=<1-100>
    //
    // Example: /api/player/arena-stats?puuid=abc123&region=na1&count=50
    //
    // Returns: {
    //   puuid, matchesScanned, totalChampions,
    //   champions: [
    //     { champion, gamesPlayed, wins, winRate, totalHours, totalMinutes },
    //     ...  (sorted by totalHours descending)
    //   ]
    // }
    CROW_ROUTE(app, "/api/player/arena-stats")
    ([&riotClient](const crow::request& req) {
        return handleArenaStatsRoute(req, riotClient);
    });

    std::cout << "Server starting on port 18080...\n";
    app.port(18080).multithreaded().run();
}
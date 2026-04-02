#define ASIO_STANDALONE
#include "crow.h"
#include <cstdlib>
#include <iostream>
#include <string>

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

    CROW_ROUTE(app, "/")([]() {
        return "Hello from my C++ backend!";
    });

    CROW_ROUTE(app, "/hello")([]() {
        return "Hello from hello route!";
    });

    CROW_ROUTE(app, "/api/player")
    ([&riotClient](const crow::request& req) {
        return handlePlayerRoute(req, riotClient);
    });

    app.port(18080).multithreaded().run();
}
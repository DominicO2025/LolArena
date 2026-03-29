#define ASIO_STANDALONE
#include "crow.h"
#include <string>
#include <cstdlib>
#include <iostream>
#include "player_handler.h"

int main() {
    const char* riotApiKey = std::getenv("RIOT_API_KEY");

    if (riotApiKey) {
        std::cout << "RIOT_API_KEY loaded successfully\n";
    } else {
        std::cout << "RIOT_API_KEY not set yet\n";
    }

    crow::SimpleApp app;

    CROW_ROUTE(app, "/")([]() {
        return "Hello from my C++ backend!";
    });

    CROW_ROUTE(app, "/hello")([]() {
        return "Hello from hello route!";
    });

    CROW_ROUTE(app, "/api/test")
    ([](const crow::request& req) {
        auto name = req.url_params.get("name");
        auto tag = req.url_params.get("tag");

        if (!name) {
            return crow::response(400, "Missing 'name' parameter");
        }

        if (!tag) {
            return crow::response(400, "Missing 'tag' parameter");
        }

        crow::json::wvalue result;
        result["message"] = "Test route works";
        result["name"] = std::string(name);
        result["tag"] = std::string(tag);
        result["fullRiotId"] = std::string(name) + "#" + std::string(tag);

        return crow::response(result);
    });

    CROW_ROUTE(app, "/api/player")
    ([](const crow::request& req) {
        return handlePlayerRoute(req);
    });

    app.port(18080).multithreaded().run();
}
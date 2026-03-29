#include "player_handler.h"
#include <string>

crow::response handlePlayerRoute(const crow::request& req) {
    auto name = req.url_params.get("name");
    auto tag = req.url_params.get("tag");
    auto region = req.url_params.get("region");

    if (!name) {
        return crow::response(400, "Missing 'name' parameter");
    }

    if (!tag) {
        return crow::response(400, "Missing 'tag' parameter");
    }

    if (!region) {
        return crow::response(400, "Missing 'region' parameter");
    }

    crow::json::wvalue result;
    result["ok"] = true;
    result["account"]["gameName"] = std::string(name);
    result["account"]["tagLine"] = std::string(tag);
    result["account"]["region"] = std::string(region);
    result["account"]["puuid"] = "fake-puuid-for-now";

    return crow::response(result);
}
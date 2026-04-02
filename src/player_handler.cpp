#include "player_handler.h"
#include <string>

crow::response handlePlayerRoute(const crow::request& req, RiotClient& riotClient) {
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

    std::string riotResponse = riotClient.getAccountByRiotId(region, name, tag);

    crow::json::wvalue result;
    result["ok"] = true;
    result["raw"] = riotResponse;

    return crow::response(result);
}
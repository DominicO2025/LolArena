#pragma once
#include "crow.h"
#include "riot_client.h"

crow::response handlePlayerRoute(const crow::request& req, RiotClient& riotClient);
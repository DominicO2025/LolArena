#define ASIO_STANDALONE
#include "crow.h"

int main() {
    crow::SimpleApp app; 

    CROW_ROUTE(app, "/")([]() {
        return "Hello from my c++ backend!"; 
    });

    CROW_ROUTE(app, "/hello")([](){
        return "Hello from hello route!";
    });

    app.port(18080).multithreaded().run(); 
}
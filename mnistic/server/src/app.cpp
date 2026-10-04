#include <app.h>

#include <core/config.h>
#include <core/logger.h>
#include <handlers/routes.h>

#include <csignal>
#include <string>

namespace server {

static server_app* g_app = nullptr;

server_app::server_app() {
    g_app = this;
}

server_app::~server_app() {
    if (g_app == this) g_app = nullptr;
}

void server_app::signal_handler(int /*sig*/) {
    if (g_app) g_app->stop();
}

bool server_app::init() {
    auto& cfg = config::instance();
    cfg.load();

    port_ = cfg.port();
    url_ = "http:

        auto& lgr = logger::instance();
    lgr.info("Server starting... on " + url_);

    svr_.set_mount_point("/", "./web");
    register_routes(svr_);
    return true;
}

void server_app::stop() {
    svr_.stop();
}

int server_app::run() {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    std::string cmd = "start " + url_;
    system(cmd.c_str());

    if (!svr_.listen("127.0.0.1", port_)) {
        logger::instance().error(
            "Failed to start server on port " + std::to_string(port_));
        return 1;
    }

    logger::instance().info("Server stopped.");
    return 0;
}

}  // namespace server

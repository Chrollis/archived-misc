#pragma once

#include <httplib.h>

#include <string>

namespace server {

class server_app {
public:
    server_app();
    ~server_app();

    bool init();

    int run();

    void stop();

private:
    static void signal_handler(int sig);

    httplib::Server svr_;
    std::string url_;
    int port_ = 3705;
};

}  // namespace server

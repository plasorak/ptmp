#include "internals.h"
#include "json.hpp"

#include <string> 
#include <iostream>             // debug
#include <algorithm>
#include <unordered_map>

using json = nlohmann::json;

using namespace ptmp::internals;

int ptmp::internals::socket_type(std::string name)
{
    std::transform(name.begin(), name.end(), name.begin(), ::tolower);
    std::unordered_map<std::string, int> zmqtypes {
        {"pair", 0},
        {"pub", 1},
        {"sub", 2},
        {"req", 3},
        {"rep", 4},
        {"dealer", 5},
        {"router", 6},
        {"pull", 7},
        {"push", 8},
        {"xpub", 9},
        {"xsub", 10},
        {"stream", 11}
    };
    return zmqtypes[name];
}


zsock_t* ptmp::internals::endpoint(const std::string& config)
{
    auto jcfg = json::parse(config);
    auto jsock = jcfg["socket"];
    std::string stype = jsock["type"];
    int socktype = socket_type(stype);
    zsock_t* sock = zsock_new(socktype);
    if (socktype == 2) {        // fixme: support topics?
        zsock_set_subscribe(sock, "");
        int old = zsock_rcvhwm(sock);
        const int hwm = old * 1000;
        zsys_info("increase sub hwm from %d to %d", old, hwm);
        zsock_set_rcvhwm(sock, hwm);
    }
    if (socktype == 1) {
        const int old = zsock_sndhwm(sock);
        const int hwm = old * 1000;
        zsys_info("increase pub hwm from %d to %d", old, hwm);
        zsock_set_sndhwm(sock, hwm);
    }
    for (auto jaddr : jsock["bind"]) {
        std::string addr = jaddr;
        zsock_bind(sock, addr.c_str(), NULL);
    }
    for (auto jaddr : jsock["connect"]) {
        std::string addr = jaddr;
        zsock_connect(sock, addr.c_str(), NULL);
    }
    return sock;
}

ptmp::internals::Socket::Socket(const std::string& config)
    : m_sock(endpoint(config))
    , m_poller(zpoller_new(m_sock, NULL))
{
}

ptmp::internals::Socket::~Socket()
{
    std::cerr << "Socket destroy\n";
    zpoller_destroy(&m_poller);
    zsock_destroy(&m_sock);

}
zmsg_t* ptmp::internals::Socket::msg(int timeout_msec)
{
    void* which = zpoller_wait(m_poller, timeout_msec);
    if (!which) return NULL;
    return zmsg_recv((zsock_t*)which);
}


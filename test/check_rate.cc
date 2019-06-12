// Yet another ZeroMQ netcat'ish program.
#include "json.hpp"
#include "CLI11.hpp"
#include <czmq.h>

#include <vector>
#include <string>
#include <unordered_map>
#include <cstdio>
#include <chrono>


#include "ptmp/api.h"
#include "ptmp/internals.h"
using json = nlohmann::json;


int main(int argc, char* argv[])
{
    CLI::App app{"Receive TPSets and check its rate"};

    int average=500;
    app.add_option("-A,--average", average,
                   "number of milliseconds to do the averaging (default 500)");    

    int hwm = 1000;
    app.add_option("-m,--socket-hwm", hwm,
                   "The ZeroMQ socket highwater mark");
    
    std::string socktype="PUB";
    app.add_option("-p,--socket-pattern", socktype,
                   "The ZeroMQ socket pattern for endpoint [PUB, PAIR, PUSH]");
    
    std::string attach="bind";
    app.add_option("-a,--socket-attachment", attach,
                   "The socket attachement method [bind|connect] for this endpoint");
    
    std::vector<std::string> endpoints;
    app.add_option("-e,--socket-endpoints", endpoints,
                   "The socket endpoint addresses in ZeroMQ format (tcp:// or ipc://)")->expected(-1);

    int timeout=average/100;
    app.add_option("-T,--timeout-ms", timeout,
                   "Number of ms to wait for a message, default is indefinite (-1)");
    CLI11_PARSE(app, argc, argv);

    json jcfg;
    jcfg["socket"]["hwm"] = hwm;
    jcfg["socket"]["type"] = socktype;
    for (const auto& endpoint : endpoints) {
        jcfg["socket"][attach].push_back(endpoint);
    }
    
    ptmp::TPReceiver recv(jcfg.dump());

    ptmp::data::TPSet tps;
    
    int count=0;
    
    uint64_t now1, now2;
    now1 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    now2 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    
    int ntpsetreceived=0;
    int ntpreceived=0;
    
    while (true) {
        
        bool ok = recv(tps, timeout);
        ntpsetreceived += ok;
        if (ok) ntpreceived += tps.tps_size();
        now1 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        if (now1 - now2 >= average) {
          std::string thestring = "TPSet rate: " + std::to_string(1000.*(double)ntpsetreceived/(now1 - now2))+" Hz \tTP rate: "
                                   + std::to_string(1000.*(double)ntpreceived/(now1 - now2));
          std::cout << thestring.c_str() <<"\r";
          std::cout.flush();
          ntpsetreceived=0;
          ntpreceived=0;
          now2 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        }
    }
    
    
    return 0;
}

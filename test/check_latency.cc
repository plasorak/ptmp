// Yet another ZeroMQ netcat'ish program.
#include "json.hpp"
#include "CLI11.hpp"
#include <czmq.h>

#include <vector>
#include <string>
#include <unordered_map>
#include <cstdio>
#include <chrono>
#include <memory>
#include <tuple>
#include <signal.h>

#include "ptmp/api.h"
#include "ptmp/internals.h"
using json = nlohmann::json;

std::ofstream myfile;
typedef std::tuple<uint64_t, uint64_t, uint64_t> data;
std::vector<data> ntp;
std::unique_ptr<ptmp::TPReceiver> receiver;
    int number=0;

void DumpFileAndClose(int exitstat=0) {
  if (myfile.is_open()) {
    for (int i=0; i<number; ++i) {
      myfile << std::to_string(std::get<0>(ntp[i])) << " "
             << std::to_string(std::get<1>(ntp[i])) << " "
             << std::to_string(std::get<2>(ntp[i])*50) << "\n";
    }
    ntp.clear();
    myfile.close();
  }
  
  if (!receiver) {
    std::cerr << "check_latency: The receiver wasn't created, probably something has gone wrong\n";
    exitstat = 1;
  } else {
    receiver.reset();
  }
  
  exit(exitstat);
}

void intHandler(int) {
  std::cout <<"interrupted\n";
  DumpFileAndClose(1);
}

int main(int argc, char* argv[])
{
    setenv("ZSYS_SIGHANDLER", "false", true);

    CLI::App app{"Receive TPSets and check its rate, format of the output file is <now in 50MHz> <tp.tstart()> <tps.create()>, no conversion needs to be made"};

    std::string filename;
    app.add_option("-f,--file", filename,
                   "name of the output file, this argument is required")->required();
    
    int count = 1000;
    app.add_option("-c,--count", count,
                   "the number of time the dumps should be done, in units of 1k (default 1000, which will save 1,000,000 latencies)");
    
    int hwm = 1000;
    app.add_option("-m,--socket-hwm", hwm,
                   "The ZeroMQ socket highwater mark (default 1000)");
    
    std::string socktype="SUB";
    app.add_option("-p,--socket-pattern", socktype,
                   "The ZeroMQ socket pattern for endpoint [SUB PAIR] (default SUB)");
    
    std::string attach="connect";
    app.add_option("-a,--socket-attachment", attach,
                   "The socket attachement method [bind|connect] for this endpoint (default connect)");
    
    bool print = false;
    app.add_option("-v,--verbose", print,
                   "Whether to print (1) or not (0) anything to the stdout (default no, 0)");
    
    std::vector<std::string> endpoints;
    app.add_option("-e,--socket-endpoints", endpoints,
                   "The socket endpoint addresses in ZeroMQ format (tcp:// or ipc://, default empty)")->expected(-1);

    int timeout=-1;
    app.add_option("-T,--timeout-ms", timeout,
                   "Number of ms to wait for a message, (default is indefinite: -1)");
    CLI11_PARSE(app, argc, argv);

    json jcfg;
    jcfg["socket"]["hwm"] = hwm;
    jcfg["socket"]["type"] = socktype;
    for (const auto& endpoint : endpoints) {
        jcfg["socket"][attach].push_back(endpoint);
    }
    
    receiver.reset(new ptmp::TPReceiver(jcfg.dump()));

    ptmp::data::TPSet tps;

    ntp.reserve(count*1000);
      
    myfile.open(filename);
    
    signal(SIGINT, intHandler);
    
    while (true) {
      
      bool ok = (*receiver)(tps, timeout);
      auto ticks = std::chrono::duration_cast<std::chrono::duration<uint64_t, std::ratio<1,50000000>>>(std::chrono::system_clock::now().time_since_epoch());
      uint64_t now = ticks.count();
      if (ok) {
        
        if (tps.tps_size()>0) {
          ntp[number++] = std::make_tuple(now,tps.tps()[0].tstart(),tps.created());
          if (number >= count*1000) {
             DumpFileAndClose(0);
          }
          
          if (print && number %10000==0) {
            double latency_hit = now - tps.tps()[0].tstart();
            double latency_tpset = now - tps.created()*50;
            // std::string thestring = ("Now: " + std::to_string(now) +
            //                          " hit ts: " + std::to_string(tps.tps()[0].tstart()) +
            //                          " message ts: " + std::to_string(tps.created()*50));
            std::string thestring = ("Hit latency: " + std::to_string(latency_hit/5000) +
                                     " ms \tTPSet latency: " + std::to_string(latency_tpset/5000) + " ms");
            std::cout << thestring <<"\r";
            std::cout.flush();
          }
        }
      }
    }

    
    return 0;
}

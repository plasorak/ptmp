#include "ptmp/api.h"
#include "ptmp/internals.h"

#include "json.hpp"

#include <queue>

using json = nlohmann::json;


// lil helper for window operations.  A window is defined by
//
// - wind :: an index locating the window absolutely in time.  The start of wind=0 is at toff.
// 
// - tspan :: the duration of the window in HW clock tricks
//
// - toff :: an offset in HW clock ticks from if t=0 was a boundary.
//
struct time_window_t {

    ptmp::data::data_time_t wind;
    const ptmp::data::data_time_t toff, tspan;

    time_window_t(ptmp::data::data_time_t tspan, ptmp::data::data_time_t toff, ptmp::data::data_time_t wind = 0)
        : tspan(tspan), toff(toff%tspan), wind(wind) { }

    // return the begin time of the window
    ptmp::data::data_time_t tbegin() const {
        return wind*tspan + toff;
    }

    // Return true if t is in time window
    bool in(ptmp::data::data_time_t t) const {
        const ptmp::data::data_time_t tbeg = tbegin();
        return tbeg <= t and t < tbeg+tspan;
    }

    int cmp(ptmp::data::data_time_t t) const {
        const ptmp::data::data_time_t tbeg = tbegin();
        if (t < tbeg) return -1;
        if (t >= tbeg+tspan) return +1;
        return 0;
    }

    void set_bytime(ptmp::data::data_time_t t) {
        wind = (t-toff) / tspan;
    }
};

struct tp_greater_t {
    bool operator()(const ptmp::data::TrigPrim& a, const ptmp::data::TrigPrim& b) {
        // give priority to smaller times
        return a.tstart() > b.tstart();
    }
};

// a priority queue of TPs that maintains a span.
class priority_tp_span_t {
public:
    typedef ptmp::data::TrigPrim value_type;
    typedef typename std::vector<value_type> collection_type;

    ptmp::data::data_time_t span() const {
        return m_recent - m_pqueue.top().tstart();
    }

    // Add a TP to the queue.  
    void add(const ptmp::data::TrigPrim& tp) {
        ptmp::data::data_time_t tstart = tp.tstart();
        m_recent = std::max(m_recent, tstart);
        m_pqueue.push(tp);
    }

    // forward methods
    bool empty() const { return m_pqueue.empty(); }
    size_t size() const { return m_pqueue.size(); }
    const value_type& top() const { return m_pqueue.top(); }
    void pop() { m_pqueue.pop(); }

private:
    std::priority_queue<value_type, collection_type, tp_greater_t> m_pqueue;

    // most recent (largest) hw clock seen
    ptmp::data::data_time_t m_recent;
};

static void dump_window(const time_window_t& window, std::string msg="")
{
    zsys_debug("window %s #%ld [%ld]+%ld @ %ld",
               msg.c_str(), window.wind, window.toff, window.tspan, window.tbegin());
}
static void dump_tpset(const ptmp::data::TPSet& tps, std::string msg="")
{
    ptmp::data::data_time_t tstart = tps.tstart();
    zsys_debug("tpset: %s @ %ld + %d # %d detid: %d",
               msg.c_str(), tstart, tps.tspan(), tps.count(), tps.detid());
    for (const auto& tp : tps.tps()) {
        ptmp::data::data_time_t tp_ts = tp.tstart();
        ptmp::data::data_time_t tp_dt = tp_ts - tstart;
        zsys_debug("\ttp: @ %ld (%8ld) + %d qpeak: %d qtot: %d chan: %d",
                   tp_ts, tp_dt, tp.tspan(), tp.adcpeak(), tp.adcsum(), tp.channel());
    }
}

static void test_time_window()
{
    const ptmp::data::data_time_t tspan = 300, toff=53;

    time_window_t tw(tspan, toff);
    assert(tw.wind == 0);
    assert(toff == tw.tbegin());
    assert(tw.in(toff));
    zsys_debug("cmp: %d" , tw.cmp(tspan+toff-1));
    assert(tw.in(tspan+toff-1));
    assert(!tw.in(toff-1));
    assert(!tw.in(tspan+toff));
    dump_window(tw);

    tw.set_bytime(tspan + toff + 1);
    //zsys_debug("wind: %ld", tw.wind);
    assert(tw.wind == 1L);
    assert(tw.tbegin() == tspan + toff);
    tw.wind = 1000;
    assert(tw.tbegin() == 1000*tspan + toff);
    dump_window(tw);
}
    
struct TPWindower {
    zsock_t* osock;
    time_window_t window;
    ptmp::data::TPSet tps;
    priority_tp_span_t buffer;
    ptmp::data::data_time_t tbuf;
    
    TPWindower(zsock_t* osock, ptmp::data::data_time_t tspan, ptmp::data::data_time_t toff, ptmp::data::data_time_t tbuf, int detid)
        : osock(osock), window(tspan, toff), tbuf(std::max(tspan,tbuf)) {
        tps.set_count(0);
        tps.set_tstart(toff);
        tps.set_tspan(tspan);
        tps.set_detid(detid);
        tps.set_created(ptmp::data::now()); // maybe want to override this just before a send()
    }

    // Maybe add (a copy of) the given tp.  Return true if added.
    bool maybe_add(const ptmp::data::TrigPrim& tp) {
        const ptmp::data::data_time_t tstart = tp.tstart();
        const int cmp = window.cmp(tstart);
        if (cmp < 0) {
            zsys_debug("window: tardy TP @%ld dt=%ld",
                       tstart, window.tbegin()-tstart);
            // dump_window(window);
            return false;
        }
        buffer.add(tp);

        while (buffer.span() >= tbuf) {  // drain buffer
            reset(buffer.top().tstart());
            while (window.in(buffer.top().tstart())) {
                ptmp::data::TrigPrim* newtp = tps.add_tps();
                *newtp = buffer.top();
                buffer.pop();
                tps.set_totaladc(tps.totaladc() + newtp->adcsum());
                const auto chan = newtp->channel();
                if (tps.chanbeg() == -1 or tps.chanbeg() > chan) {
                    tps.set_chanbeg(chan);
                }
                if (tps.chanend() == -1 or tps.chanend() < chan) {
                    tps.set_chanend(chan);
                }
            }
            if (osock) {                       // allow null for testing
                ptmp::internals::send(osock, tps); // fixme: can throw
            }
            else {
                zsys_debug("testing mode, not actually sending a TPSet");
            }
            // note, this leaves tps full of stale/sent info....

            // dump_window(window, "send");
            // dump_tpset(tps, "send");
            // ptmp::data::dump(tps, "send");
        }
        return true;
    }

    // Reset state.  Better send() tps before calling.
    const time_window_t& reset(ptmp::data::data_time_t t) {
        window.set_bytime(t);
        tps.set_count(tps.count()+1);
        tps.set_created(ptmp::data::now());
        tps.set_tstart(window.tbegin());
        tps.set_chanbeg(-1);
        tps.set_chanend(-1);
        tps.set_totaladc(0);
        tps.clear_tps();
        return window;
    }
};

static void test_tpwindower()
{
    ptmp::data::data_time_t tspan=300, toff=53;
    TPWindower tpw(nullptr, tspan, toff, 0, 1234);
    dump_window(tpw.window);
    const auto& tw = tpw.reset(tspan+toff);
    assert(tw.wind == 1);
    assert(tw.in(tspan+toff));
    assert(tw.in(tspan+2*toff-1));

    ptmp::data::TrigPrim tp;

    tp.set_tstart(tspan+toff);  // next window
    tp.set_channel(42);
    bool ok = tpw.maybe_add(tp);
    assert(ok);
    zsys_debug("buffered %d tps over %ld", (int)tpw.buffer.size(), tpw.buffer.span());
    assert(tpw.buffer.size() == 1);
    dump_window(tpw.window);

    tp.set_tstart(tp.tstart() + 1);
    tp.set_channel(43);    
    ok = tpw.maybe_add(tp);
    assert(ok);
    zsys_debug("buffered %d tps over %ld", (int)tpw.buffer.size(), tpw.buffer.span());
    assert(tpw.buffer.size() == 2);
    dump_window(tpw.window);
    
    tp.set_tstart(toff);
    ok = tpw.maybe_add(tp);
    assert(!ok);
    zsys_debug("buffered %d tps over %ld", (int)tpw.buffer.size(), tpw.buffer.span());
    assert(tpw.buffer.size() == 2);
    dump_window(tpw.window);

    zsys_debug("advancing window");
    tp.set_tstart(toff+2*tspan); // in next window
    ok = tpw.maybe_add(tp);
    assert(ok);
    dump_window(tpw.window);
    zsys_debug("buffered %d tps over %ld", (int)tpw.buffer.size(), tpw.buffer.span());
    assert(tpw.buffer.size() == 1);

    tp.set_tstart(tp.tstart() + 1);
    ok = tpw.maybe_add(tp);
    assert(ok);
    zsys_debug("buffered %d tps over %ld", (int)tpw.buffer.size(), tpw.buffer.span());
    assert(tpw.buffer.size() == 2);
    dump_window(tpw.window);
}


// The actor function
void tpwindow_proxy(zsock_t* pipe, void* vargs)
{
    auto config = json::parse((const char*) vargs);
    int detid = -1;
    if (config["detid"].is_number()) {
        detid = config["detid"];
    }
    ptmp::data::data_time_t toff=0;
    if (config["toffset"].is_number()) {
        toff = config["toffset"];
    }
    ptmp::data::data_time_t tspan=0;
    if (config["tspan"].is_number()) {
        tspan = config["tspan"];
    }
    if (!tspan) {
        zsys_error("tpwindow requires finite tspan");
        throw std::runtime_error("tpwindow requires finite tspan");

    }
    ptmp::data::data_time_t tbuf=0;
    if (config["tbuf"].is_number()) {
        tspan = config["tbuf"];
    }

    zsock_t* isock = ptmp::internals::endpoint(config["input"].dump());
    zsock_t* osock = ptmp::internals::endpoint(config["output"].dump());
    if (!isock or !osock) {
        zsys_error("tpwindow requires socket configuration");
        return;
    }
    
    zsock_signal(pipe, 0);      // signal ready
    zpoller_t* pipe_poller = zpoller_new(pipe, isock, NULL);

    // initial window is most likely way before any data.
    TPWindower windower(osock, tspan, toff, tbuf, detid);

    while (!zsys_interrupted) {

        void* which = zpoller_wait(pipe_poller, -1);
        if (!which) {
            zsys_info("TPWindow proxy interrupted");
            break;
        }
        if (which == pipe) {
            zsys_info("TPWindow proxy got quit with %d TPs", windower.tps.tps().size());
            dump_window(windower.window, "quit");
            break;
        }

        // zsys_info("TPWindow got input");
        // dump_window(windower.window, "recv");

        zmsg_t* msg = zmsg_recv(isock);
        if (!msg) {
            zsys_info("TPWindow proxy interrupted");
            zmsg_destroy(&msg);
            break;
        }

        ptmp::data::TPSet tps;
        ptmp::internals::recv(msg, tps); // throws
        ptmp::data::real_time_t latency = ptmp::data::now() - tps.created();
        // dump_window(windower.window, "recv");
        // dump_tpset(tps, "recv");
        // ptmp::data::dump(tps, "recv");

        if (detid < 0) {        // forward if user doesn't provide
            detid = tps.detid();
            windower.tps.set_detid(detid);
        }            

        int ntps_failed = 0;
        // We do not know ordering of TrigPrim inside TPSet
        std::sort(tps.mutable_tps()->begin(), tps.mutable_tps()->end(),
                  [](const ptmp::data::TrigPrim& a, const ptmp::data::TrigPrim& b) {
                      return a.tstart() < b.tstart();
                  });
        for (const auto& tp : tps.tps()) {
            bool ok = windower.maybe_add(tp);
            if (!ok) {
                ++ntps_failed;
            //     zsys_debug("\tfail TP at %ld", tp.tstart());
            // }
            // else {
            //     zsys_debug("\tkeep TP at %ld", tp.tstart());
            }

        }
        if (ntps_failed) {
            //zsys_debug("tpwindow: failed to add %d TPs, latency:%ld", ntps_failed, latency);
            continue;
        }        


    } // message loop


    // zsys_debug("tpwindow actor cleaning up");
    zpoller_destroy(&pipe_poller);
    zsock_destroy(&isock);
    zsock_destroy(&osock);
    // zsys_debug("tpwindow done");
}

ptmp::TPWindow::TPWindow(const std::string& config)
    : m_actor(zactor_new(tpwindow_proxy, (void*)config.c_str()))
{
}

ptmp::TPWindow::~TPWindow()
{
    // zsys_debug("tpwindow: signaling done");
    zsock_signal(zactor_sock(m_actor), 0); // signal quit
    zactor_destroy(&m_actor);
    // zsys_debug("tpwindow: destroying");
}

void ptmp::TPWindow::test()
{
    test_time_window();
    test_tpwindower();
}

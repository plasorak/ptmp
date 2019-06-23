{
    // Create a port structure.  A port has a name which should be
    // unique in the node.  The sock_type is the usual ZMQ name (eg
    // "PUB").  An attachments (atts) is an array of (bc,addr) pairs
    // where 'bc' is either 'bind' or 'connect' and 'addr' is a zeromq
    // address string.  If the type is omitted or atts are incomplete
    // then the port is not fully formed.
    port(name, type=null, atts=[]):: {
        name:name, type:type, atts:atts
    },
    
    // Create a node.  A node represents some unit of action (eg, an
    // application or an actor within a larger application).  A fully
    // formed node has a number of fully formed ports constructed as
    // above.
    node(name, ports=[], data={}):: {
        name: name,
        ports: ports,
        data: data,
    },


    // Create a file source subgraph with a node.data suitable for
    // generating a command line.  A single socket is plugged and no
    // addresses attached.
    //
    // FIXME:, currently czmqat doesn't support mixed bind/connect
    // (although zmq does).
    //
    // FIXME: this structure only supports a single endpoint (although
    // czmqat supports multiple).
    fsource(name, filename, port, clidata={}) :: self.node(
        name,
        ports=[port],
        data = {
            app_type: "subprocess",
            cmdline: "czmqat ifile -f {filename} osock " +
                "-m {hwm} -p {ports[0][type]} " +
                "-a {ports[0][atts][0][0]} -e {ports[0][atts][0][1]}",
            hwm: 1000,
            filename: filename
        } + clidata),
    
}
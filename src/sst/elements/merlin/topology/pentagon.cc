// Copyright 2009-2015 Sandia Corporation. Under the terms
// of Contract DE-AC04-94AL85000 with Sandia Corporation, the U.S.
// Government retains certain rights in this software.
// 
// Copyright (c) 2009-2016, Sandia Corporation
// All rights reserved.
// 
// This file is part of the SST software package. For license
// information, see the LICENSE file in the top level directory of the
// distribution.
#include <sst_config.h>
#include "pentagon.h"

#include <stdlib.h>


using namespace SST::Merlin;

/* Assumed connectivity of each router:
 * ports [0, host_ports-1]:      Hosts
 * ports [host_ports, host_ports+local_ports-1]:    Intra-group
 * ports [host_ports+local_ports, total_ports]:  ports used to construct hs graph
 */ 

topo_pentagon::topo_pentagon(Component* comp, Params& params) :
    Topology(comp)
{
    // id is the global id 
    router_id = params.find<int>("id", -1);
    if (router_id == -1) {
        output.fatal(CALL_INFO, -1, "id must be set\n");
    }
    
    output.init("Pentagon", 
        params.find<uint32_t>("verboseLevel",0),
        params.find<uint32_t>("verboseMask",-1), 
        Output::STDOUT);

    host_ports = (uint32_t)params.find<int>("pentagon:hosts_per_router", 1);
    outgoing_ports = (uint32_t)params.find<int>("pentagon:outgoing_ports", 0);
    local_ports = 2;
    routers_per_subnet = 5;
    uint32_t num_ports = (uint32_t)params.find<int>("num_ports");
    uint32_t needed_ports = host_ports + local_ports + outgoing_ports;
    if (needed_ports > num_ports) {
        output.fatal(CALL_INFO, -1, "Need more ports to support the given topology!\n");
    }
    
    std::string route_algo = params.find<std::string>("pentagon:algorithm", "minimal");
    
    if (!route_algo.compare("minimal")) {
        // TODO implement other algorithms later
        algorithm = MINIMAL;
    }
    
    std::string interconnect = params.find<std::string>("pentagon:interconnect", "none");
    
    if (interconnect.compare("fishlite") == 0 ) {  // returns 0 when equal.. wtf
        subnet = (uint32_t)params.find<int>("pentagon:subnet", 0);
        net_type = FISH_LITE;
    } else if (interconnect.compare("fishnet") == 0) {
        subnet = (uint32_t)params.find<int>("pentagon:subnet", 0);
        net_type = FISHNET;
    } else {
        // just a pentagon
        subnet = 0; 
        net_type = NONFISH;
    }
    router = (uint32_t)params.find<int>("pentagon:router", 0);
    
}


topo_pentagon::~topo_pentagon()
{
}


void topo_pentagon::route(int port, int vc, internal_router_event* ev)
{
    topo_pentagon_event *tp_ev = static_cast<topo_pentagon_event*>(ev);
    if ( (uint32_t)port >= (host_ports + local_ports) ) {
        /* Came in from another subnet.  Increment VC */
        tp_ev->setVC(vc+1);
    }
    
    uint32_t next_port = 0;
    if (tp_ev->dest.subnet != subnet) {
        // target is not this subnet
        // either fishlite or fishnet
        if (net_type == NONFISH) {
            output.fatal(CALL_INFO, -1, "How could you get here? \n");
        } else {
            uint32_t mid_rtr = 0;  // the router responsible for forwarding packet
            if (tp_ev->dest.subnet < subnet) {
                mid_rtr = tp_ev->dest.subnet;
            } else {
                mid_rtr = tp_ev->dest.subnet - 1; // minus 1 because the way it connects
            }
            if (net_type == FISH_LITE) {
                if (mid_rtr == router) {  // going to other subnets 
                    next_port = host_ports + local_ports;
                } else {  // forward
                    next_port = port_for_router(mid_rtr);
                }
            } else {  // fishnet
                if (mid_rtr == router) {  
                    // different from fishlite, the mid_rtr neighbors have access to
                    // the target subnet, so forward to one of its neighbors
                    // could be from host_ports to (host_ports + local_ports -1)
                    next_port = host_ports;
                } else {
                    if (is_neighbor(router, mid_rtr)) { // if router in mid_rtr 's neighbor
                        bool find_rtr = false;
                        for (uint32_t i = 0; i < outgoing_ports; i++) {
                            uint32_t neighbor = neighbor_table[router][i];
                            if (neighbor >= subnet) {
                                neighbor += 1;
                            }
                            if (neighbor == tp_ev->dest.subnet) {  // going to other subnet
                                next_port = host_ports + local_ports + i;
                                find_rtr = true;
                                break;
                            }
                        }
                        if (!find_rtr) {
                            output.fatal(CALL_INFO, -1, "cannot find route!\n");
                        }
                    } else { // forward to mid_rtr
                        next_port = port_for_router(mid_rtr);
                    }
                }
            }
        }       
    } else if ( tp_ev->dest.router != router) {
        // not this router, forward to other routers in this subnet
        // trivial routing
        next_port = port_for_router(tp_ev->dest.router);
    } else {
        // this router
        next_port = tp_ev->dest.host;
    }
    output.verbose(CALL_INFO, 1, 1, "%u:%u, Recv: %d/%d  Setting Next Port/VC:  %u/%u\n", 
                    subnet, router, port, vc, next_port, tp_ev->getVC());
    tp_ev->setNextPort(next_port);
}


internal_router_event* topo_pentagon::process_input(RtrEvent* ev)
{
    topo_pentagon::fishnetAddr destAddr = {0, 0, 0};
    id_to_location(ev->request->dest, &destAddr);
    topo_pentagon_event *tp_ev = new topo_pentagon_event(destAddr);
    // if to implement other algorithm, need to add stuff..
    
    tp_ev->src_subnet = subnet;

    tp_ev->setEncapsulatedEvent(ev);
    // TODO not sure about this vn thing
    tp_ev->setVC(ev->request->vn *3);
    return tp_ev;
}

// this is to figure out what are the possible outports for a given input port
void topo_pentagon::routeInitData(int port, internal_router_event* ev, std::vector<int> &outPorts)
{
    topo_pentagon_event *tp_ev = static_cast<topo_pentagon_event*>(ev);
    if ( tp_ev->dest.host == (uint32_t)INIT_BROADCAST_ADDR ) {
        uint32_t total_ports = host_ports + local_ports + outgoing_ports;
        if ( (uint32_t)port >= (host_ports + local_ports ) ) {
            /* Came in from another subnet.
             * Send to locals, and other routers in subnet
             */
            tp_ev->is_forwarded = 0;
            for ( uint32_t p = 0; p < (host_ports + local_ports ); p++ ) {
                outPorts.push_back((int)p);
            }
        } else if ( (uint32_t)port >= host_ports ) {
            /* Came in from another router in subnet.
             * send to hosts
             * if this is the source subnet, 
             * forward to the other router, or send to other subnets
             */
            for ( uint32_t p = 0; p < host_ports; p++ ) {
                outPorts.push_back((int)p);
            }
            /* Note this should be different from dragonfly
             * because you can forward 
             */
            if (tp_ev->src_subnet == subnet) {
                for ( uint32_t p = (host_ports + local_ports); p < total_ports; p++) {
                    outPorts.push_back((int)p);
                }
            }
            if (tp_ev->is_forwarded == 0) {
                tp_ev->is_forwarded = 1; 
                for ( uint32_t p = host_ports; p < (host_ports + local_ports); p++) {
                    outPorts.push_back((int)p);
                }
            }
        } else {
            /* Came in from a host
             * Send to all other hosts and routers in group, and all groups
             */
            for (uint32_t p = 0; p < total_ports; p++) {
                if (p != (uint32_t)port) {
                    outPorts.push_back((int)p);
                }
            }
        }
    } else {
        route(port, 0, ev);
        outPorts.push_back(ev->getNextPort());
    }
}


internal_router_event* topo_pentagon::process_InitData_input(RtrEvent* ev)
{
    topo_pentagon::fishnetAddr destAddr;
    id_to_location(ev->request->dest, &destAddr);
    topo_pentagon_event *tp_ev = new topo_pentagon_event(destAddr);
    tp_ev->src_subnet = subnet;
    tp_ev->is_forwarded = 0;
    tp_ev->setEncapsulatedEvent(ev);
    return tp_ev;
}


Topology::PortState topo_pentagon::getPortState(int port) const
{
    if ((uint32_t)port < host_ports) return R2N;
    else return R2R;
}

std::string topo_pentagon::getPortLogicalGroup(int port) const
{
    if ((uint32_t)port < host_ports) return "host";
    if ((uint32_t)port >= host_ports && (uint32_t)port < (host_ports + local_ports)) return "group";
    else return "global";
}

int topo_pentagon::getEndpointID(int port)
{
    return (subnet* (routers_per_subnet * host_ports)) + router*host_ports + port;
}

void topo_pentagon::id_to_location(int id, fishnetAddr *location) const
{
    if (id == INIT_BROADCAST_ADDR) {
        location->subnet = (uint32_t)INIT_BROADCAST_ADDR;
        location->router = (uint32_t)INIT_BROADCAST_ADDR;
        location->host = (uint32_t)INIT_BROADCAST_ADDR;
    } else {
        uint32_t hosts_per_subnet = host_ports * routers_per_subnet;
        location->subnet = id / hosts_per_subnet;
        location->router = (id % hosts_per_subnet) / host_ports;
        location->host = id % host_ports;
    }
}


uint32_t topo_pentagon::port_for_router(uint32_t dest_router) const 
{
    if (dest_router == ((router+1)%5) || 
        dest_router == ((router+2)%5)) {
        // left side
        return host_ports;
    } else {
        // right side
        return host_ports + 1;
    }
}

// see if the target router is a neighbor of this router
bool topo_pentagon::is_neighbor(uint32_t tgt_rtr, uint32_t this_rtr) const
{
    for (uint32_t i = 0; i < local_ports; i++) {
        if (tgt_rtr == neighbor_table[this_rtr][i]) {
            return true;
        }
    }
    return false;
}



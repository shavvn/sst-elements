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

#include <algorithm>
#include <stdlib.h>


using namespace SST::Merlin;

topo_pentagon::topo_pentagon(Component* comp, Params& params) :
    Topology(comp)
{
    // id is the global id 
    router_id = params.find<int>("id", -1);
    if (router_id == -1) {
        output.fatal(CALL_INFO, -1, "id must be set\n");
    }
    
    hosts_per_router = (uint32_t)params.find<int>("pentagon:hosts_per_router", 1);
    routers_per_subnet = (uint32_t)params.find<int>("pentagon:routers_per_subnet", 5);
    outgoing_ports = (uint32_t)params.find<int>("pentagon:outgoing_ports", 0);
    // set this to 3 for petersen graphs
    num_neighbors = (uint32_t)params.find<int>("pentagon:num_neighbors", 2);
    start_router_id = (uint32_t)params.find<int>("pentagon:start_router_id", -1);
    std::string route_algo = params.find<std::string>("pentagon:algorithm", "minimal");

    
    if (!route_algo.compare("minimal")) {
        // TODO implement other algorithms later
        algorithm = MINIMAL;
    }
    
    addr.subnet = (uint32_t)params.find<int>("pentagon:subnet", 0);
    addr.router = (uint32_t)params.find<int>("pentagon:router", 0);
    
}

void topo_pentagon::route(int port, int vc, internal_router_event* ev)
{
    topo_pentagon_event *tp_ev = static_cast<topo_pentagon_event*>(ev);
    
    if ( (uint32_t)port > (hosts_per_router + num_neighbors) ) {
        /* Came in from another subnet.  Increment VC */
        tp_ev->setVC(vc+1);
    }
    
    uint32_t next_port = 0;
    
    if (tp_ev->dest.subnet != addr.subnet) {
        // target is not this subnet
        // TODO decode which port should it be sent to 
        // for fishnet lite, this is easy, just go to correspoding router
        // for fishnet, find the neighbors of correspoding router
        // and then figure out which neighbor is closer...
    } else if ( tp_ev->dest.router != addr.router) {
        // not this router, forward to other routers in this subnet
        // trivial routing
        if (tp_ev->dest.router == ((addr.router+1)%5) || 
            tp_ev->dest.router == ((addr.router+2)%5)) {
            // left side
            next_port = hosts_per_router;
        } else {
            // right side
            next_port = hosts_per_router + 1;
        }
    } else {
        // this router
        next_port = addr.host;
    }
    output.verbose(CALL_INFO, 1, 1, "%u:%u, Recv: %d/%d  Setting Next Port/VC:  %u/%u\n", addr.subnet, addr.router, port, vc, next_port, tp_ev->getVC());
    tp_ev->setNextPort(next_port);
}


internal_router_event* topo_pentagon::process_input(RtrEvent* ev)
{
    topo_pentagon::fishnetAddr destAddr = {0, 0, 0};
    id_to_location(ev->request->dest, &destAddr);
    topo_pentagon_event *tp_ev = new topo_pentagon_event(destAddr);
    // TODO if to implement other algorithm, need to add stuff..
    
    tp_ev->src_subnet = addr.subnet;
    tp_ev->setEncapsulatedEvent(ev);
    // TODO not sure about this vn thing
    tp_ev->setVC(ev->request->vn *3);
    return tp_ev;
}

internal_router_event* topo_pentagon::process_InitData_input(RtrEvent* ev)
{
    topo_pentagon::fishnetAddr destAddr;
    id_to_location(ev->request->dest, &destAddr);
    topo_pentagon_event *tp_ev = new topo_pentagon_event(destAddr);
    tp_ev->src_subnet = addr.subnet;
    tp_ev->setEncapsulatedEvent(ev);
    return tp_ev;
}


// this is to figure out what are the possible outports for a given input port
void topo_pentagon::routeInitData(int port, internal_router_event* ev, std::vector<int> &outPorts)
{
    topo_pentagon_event *tp_ev = static_cast<topo_pentagon_event*>(ev);
    if ( tp_ev->dest.host == (uint32_t)INIT_BROADCAST_ADDR ) {
        uint32_t total_ports = hosts_per_router + num_neighbors + outgoing_ports;
        if ( (uint32_t)port >= (hosts_per_router + num_neighbors ) ) {
            /* Came in from another subnet.
             * Send to locals, and other routers in subnet
             */
            for ( uint32_t p = 0; p < (hosts_per_router + num_neighbors ); p++ ) {
                outPorts.push_back((int)p);
            }
        } else if ( (uint32_t)port >= hosts_per_router ) {
            /* Came in from another router in subnet.
             * send to hosts
             * if this is the source subnet, send to other subnets
             */
            for ( uint32_t p = 0; p < hosts_per_router; p++ ) {
                outPorts.push_back((int)p);
            }
            if (tp_ev->src_subnet = addr.subnet) {
                
                for (uint32_t p = (hosts_per_router+num_neighbors); p < total_ports; p++) {
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


void topo_pentagon::id_to_location(int id, fishnetAddr *location) const
{
    if (id == INIT_BROADCAST_ADDR) {
        location->subnet = (uint32_t)INIT_BROADCAST_ADDR;
        location->router = (uint32_t)INIT_BROADCAST_ADDR;
        location->host = (uint32_t)INIT_BROADCAST_ADDR;
    } else {
        uint32_t hosts_per_subnet = hosts_per_router * routers_per_subnet;
        location->subnet = id / hosts_per_subnet;
        location->router = (id % hosts_per_subnet) / hosts_per_router;
        location->host = id % hosts_per_router;
    }
}

/*
uint32_t topo_pentagon::router_to_subnet(uint32_t subnet) const
{
    return 0;
}


uint32_t topo_pentagon::port_for_subnet(uint32_t subnet) const
{
    return 0; 
}


Topology::PortState topo_pentagon::getPortState(int port) const
{
    if ((uint32_t)port < hosts_per_router) return R2N;
    else return R2R;
}


uint32_t localToGlobalID(uint32_t local_num)
{
    return (router_id_start + (local_num%5);
}
*/



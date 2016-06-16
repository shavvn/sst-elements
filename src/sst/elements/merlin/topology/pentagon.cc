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
 * ports [host_ports, host_ports+num_neighbors-1]:    Intra-group
 * ports [host_ports+num_neighbors, total_ports]:  ports used to construct hs graph
 */ 

topo_pentagon::topo_pentagon(Component* comp, Params& params) :
    Topology(comp)
{
    // id is the global id 
    router_id = params.find<int>("id", -1);
    if (router_id == -1) {
        output.fatal(CALL_INFO, -1, "id must be set\n");
    }
    
    host_ports = (uint32_t)params.find<int>("pentagon:host_ports", 1);
    uint32_t num_ports = (uint32_t)params.find<int>("num_ports");
    outgoing_ports = (uint32_t)params.find<int>("pentagon:outgoing_ports", 0);
    local_ports = 2;
    routers_per_subnet = 5;
    // set this to 3 for petersen graphs
    // num_neighbors = (uint32_t)params.find<int>("pentagon:num_neighbors", 2);
    start_router_id = (uint32_t)params.find<int>("pentagon:start_router_id", -1);
    std::string route_algo = params.find<std::string>("pentagon:algorithm", "minimal");

    
    if (!route_algo.compare("minimal")) {
        // TODO implement other algorithms later
        algorithm = MINIMAL;
    }
    
    subnet = (uint32_t)params.find<int>("pentagon:subnet", 0);
    router = (uint32_t)params.find<int>("pentagon:router", 0);
    
}


topo_pentagon::~topo_pentagon()
{
}


void topo_pentagon::route(int port, int vc, internal_router_event* ev)
{
    topo_pentagon_event *tp_ev = static_cast<topo_pentagon_event*>(ev);
    uint32_t local_ports = 2;
    if ( (uint32_t)port > (host_ports + local_ports) ) {
        /* Came in from another subnet.  Increment VC */
        tp_ev->setVC(vc+1);
    }
    
    uint32_t next_port = 0;
    
    if (tp_ev->dest.subnet != subnet) {
        // target is not this subnet
        // only implement for angelfish-lite for now.. 
        if (tp_ev->dest.subnet == router) {
            next_port = host_ports + local_ports;
        } else {
            next_port = port_for_router(tp_ev->dest.subnet);
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
    output.verbose(CALL_INFO, 1, 1, "Processing input...\n");
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
    output.verbose(CALL_INFO, 1, 1, "route InitData...\n");
    topo_pentagon_event *tp_ev = static_cast<topo_pentagon_event*>(ev);
    if ( tp_ev->dest.host == (uint32_t)INIT_BROADCAST_ADDR ) {
        uint32_t local_ports = 2;
        uint32_t total_ports = host_ports + local_ports + outgoing_ports;
        if ( (uint32_t)port >= (host_ports + local_ports ) ) {
            /* Came in from another subnet.
             * Send to locals, and other routers in subnet
             */
            for ( uint32_t p = 0; p < (host_ports + local_ports ); p++ ) {
                outPorts.push_back((int)p);
            }
        } else if ( (uint32_t)port >= host_ports ) {
            /* Came in from another router in subnet.
             * send to hosts
             * if this is the source subnet, send to other subnets
             */
            for ( uint32_t p = 0; p < host_ports; p++ ) {
                outPorts.push_back((int)p);
            }
            if (tp_ev->src_subnet = subnet) {
                for (uint32_t p = (host_ports+local_ports); p < total_ports; p++) {
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
    output.verbose(CALL_INFO, 1, 1, "Processing InitData input...\n");
    topo_pentagon::fishnetAddr destAddr;
    id_to_location(ev->request->dest, &destAddr);
    topo_pentagon_event *tp_ev = new topo_pentagon_event(destAddr);
    tp_ev->src_subnet = subnet;
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
    if ((uint32_t)port >= host_ports && (uint32_t)port < (host_ports + 2)) return "group";
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

/*
uint32_t topo_pentagon::router_to_subnet(uint32_t subnet) const
{
    return 0;
}


uint32_t topo_pentagon::port_for_subnet(uint32_t subnet) const
{
    return 0; 
}





uint32_t localToGlobalID(uint32_t local_num)
{
    return (router_id_start + (local_num%5);
}
*/



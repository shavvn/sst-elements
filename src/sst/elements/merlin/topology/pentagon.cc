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
    Topology()
{
    // id is the global id 
    router_id = params.find<int>("id", -1);
    if (router_id == -1) {
        output.fatal(CALL_INFO, -1, "id must be set\n");
    }
    
    hosts_per_router = (uint32_t)params.find<int>("pentagon:hosts_per_router", 1);
    start_router_id = (uint32_t)params.find<int>("pentagon:start_router_id", -1);
    std::string route_algo = params.find<std::string>("pentagon:algorithm", "minimal");
    
    if (!route_algo.compare("minimal") {
        // TODO implement other algorithms later
        algorithm = MINIMAL;
    }
    
    addr.subnet = (uint32_t)params.find<int>("pentagon:subnet_num", 0);
    addr.router = (uint32_t)params.find<int>("pentagon:router", 0);
    
}

void topo_pentagon::route(int port, int vc, internal_router_event* ev)
{
    topo_pentagon_event *tp_ev = static_cast<topo_pentagon_event*>(ev);
    
    if ((uint32_t)port > (hosts_per_router + 5)) {
        /* Came in from another group.  Increment VC */
        tp_ev->setVC(vc+1);
    }
    
    uint32_t next_port = 0;
    
    if (tp_ev->dest.subnet != addr.subnet) {
        // target is not this group
        // TODO decode which port should it be sent to 
    } else if ( tp_ev->dest.router != router_id) {
        // not this router, forward to other routers in this group
        // TODO this is where routing matters...
        // for min routing, you can always go one direction
        // for other dynamic routing you have more choices
    } else {
        // this router
        // TODO figure out the port that host correspond to
    }
}

// this is to figure out what are the possible outports 
void topo_pentagon::routeInitData(int port, internal_router_event* ev, std::vector<int> &outPorts)
{
    
}

internal_router_event* topo_torus::process_input(RtrEvent* ev)
{
    topo_pentagon_event* tp_ev = new topo_pentagon_event();
    tp_ev->setEncapsulatedEvent(ev);
    tp_ev->setVC();  // TODO figure out what to do with VC
    
    return tp_ev;
}

uint32_t localToGlobalID(uint32_t local_num)
{
    return (router_id_start + (local_num%5);
}

int
topo_pentagon::get_dest_router(int dest_id) const
{
    return dest_id / num_local_ports;
}

int
topo_torus::get_dest_local_port(int dest_id) const
{
    return local_port_start + (dest_id % num_local_ports);
}


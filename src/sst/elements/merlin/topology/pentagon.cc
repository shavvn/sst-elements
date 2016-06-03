// Copyright 2009-2015 Sandia Corporation. Under the terms
// of Contract DE-AC04-94AL85000 with Sandia Corporation, the U.S.
// Government retains certain rights in this software.
// 
// Copyright (c) 2009-2015, Sandia Corporation
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
    uint32_t id = params.find<int>("id", -1);
    if (id == -1) {
        // do something?
    }
     
    local_id = id % 5;
    
    // port_start = int[5][2]
    
    std::route_algo = p.find<std::string>("pentagon:algorithm", "minimal")
    
    if (!route_algo.compare("minimal") {
        // hard reset to minimal for now
        algorithm = MINIMAL;
    }
    
}

void topo_pentagon::route(int port, int vc, internal_router_event* ev)
{
    topo_pentagon_event *td_ev = static_cast<topo_pentagon_event*>(ev);
    
    if ((uint32_t)port >= 2) {
        /* port 0 and 1 are used within group */
        
    }
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


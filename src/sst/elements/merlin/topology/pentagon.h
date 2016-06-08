// -*- mode: c++ -*-

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

#ifndef COMPONENTS_MERLIN_TOPOLOGY_PENTAGON_H
#define COMPONENTS_MERLIN_TOPOLOGY_PENTAGON_H

#include <sst/core/event.h>
#include <sst/core/link.h>
#include <sst/core/params.h>

#include <string.h>

#include "sst/elements/merlin/router.h"

namespace SST {
namespace Merlin {
    
class topo_pentagon: public Topology {
    
    // global router id
    uint32_t router_id;
    // hosts per router, all routers should have the same value of this
    uint32_t hosts_per_router;
    // the start router id of this group
    uint32_t start_router_id;
    // in cases this is used to build larger graph
    uint32_t routers_per_subnet;
    // num of adjacent routers
    uint32_t num_neighbors;
    // num of ports going out of this pentagon
    uint32_t outgoing_ports;
    // the start host id under this router
    // uint32_t start_host_id;
    // the routing of fishnet is based on subnet and local router within subnet
    
    
    // only implemented MINIMAL for now... 
    enum RouteAlgo {
        MINIMAL,
        VILIANT,
        ADAPTIVE
    };
    
    RouteAlgo algorithm;
    
    
public:
    struct fishnetAddr {
        uint32_t subnet;
        uint32_t router;
        uint32_t host;
    };
    topo_pentagon(Component* comp, Params& params);
    ~topo_pentagon();
    
    virtual void route(int port, int vc, internal_router_event* ev);
    virtual internal_router_event* process_input(RtrEvent* ev);
    
    virtual void routeInitData(int port, internal_router_event* ev, std::vector<int> &outPorts);
    virtual internal_router_event* process_InitData_input(RtrEvent* ev);

    virtual PortState getPortState(int port) const;
    virtual int computeNumVCs(int vns);
    virtual int getEndpointID(int port);
    uint32_t localToGlobalID(uint32_t local_num);

protected:
    virtual int choose_multipath(int start_port, int num_ports, int dest_dist);
    
private:
    fishnetAddr addr;
    void id_to_location(int id, fishnetAddr *location) const;
};


class topo_pentagon_event : public internal_router_event {
    /* Assumed connectivity of each router:
     * ports [0, hosts_per_router-1]:      Hosts
     * ports [hosts_per_router, hosts_per_router+num_neighbors-1]:    Intra-group
     * ports [hosts_per_router+num_neighbors, total_ports]:  Inter-group
     */
public:
    uint32_t src_subnet;
    topo_pentagon::fishnetAddr dest;
    topo_pentagon_event() {}
    topo_pentagon_event(const topo_pentagon::fishnetAddr &dest) : dest(dest) {}
    ~topo_pentagon_event() {}
    virtual internal_router_event* clone(void)
    {
        return new topo_pentagon_event(*this);
    }
    
    void serialize_order(SST::Core::Serialization::serializer &ser) 
    {
        internal_router_event::serialize_order(ser);
        ser & src_subnet;
        ser & dest.subnet;
        ser & dest.router;
        ser & dest.host;
    }
    
private:
    ImplementSerializable(SST::Merlin::topo_pentagon_event)
    
};

}
}

#endif // COMPONENTS_MERLIN_TOPOLOGY_PENTAGON_H
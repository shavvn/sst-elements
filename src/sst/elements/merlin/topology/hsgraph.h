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

#ifndef COMPONENTS_MERLIN_TOPOLOGY_HSGRAPH_H
#define COMPONENTS_MERLIN_TOPOLOGY_HSGRAPH_H

#include <sst/core/event.h>
#include <sst/core/link.h>
#include <sst/core/params.h>


#include "sst/elements/merlin/router.h"

namespace SST {
namespace Merlin {

class topo_hsgraph: public Topology {
    // Assumed connectivity of each router:
    // ports [0, host_ports-1]:      Hosts
    // ports [host_ports, host_ports+local_ports-1]:    Intra-group
    // ports [host_ports+local_ports, total_ports]:  ports used to construct hs graph
    // a description of hsgraph could be found here:
    // https://en.wikipedia.org/wiki/Hoffman%E2%80%93Singleton_graph
    //
    uint32_t router_id;
    uint32_t host_ports;
    uint32_t routers_per_subnet;
    uint32_t local_ports;
    uint32_t outgoing_ports;
    uint32_t subnet;
    uint32_t router;
    
    // only implemented MINIMAL for now... 
    enum RouteAlgo {
        MINIMAL,
        VILIANT,
        ADAPTIVE
    };
    
    RouteAlgo algorithm;
    enum FishnetType {
        NONFISH,
        FISH_LITE,
        FISHNET
    };
    
    FishnetType net_type;
    
public:
    struct fishnetAddr {
        uint32_t subnet;
        uint32_t router;
        uint32_t host;
    };
    
    
    topo_hsgraph(Component* comp, Params& params);
    ~topo_hsgraph();
    
    virtual void route(int port, int vc, internal_router_event* ev);
    virtual internal_router_event* process_input(RtrEvent* ev);
    
    virtual PortState getPortState(int port) const;
    virtual std::string getPortLogicalGroup(int port) const;
    
    virtual void routeInitData(int port, internal_router_event* ev, std::vector<int> &outPorts);
    virtual internal_router_event* process_InitData_input(RtrEvent* ev);

    
    virtual int computeNumVCs(int vns) { return vns * 3; }
    virtual int getEndpointID(int port);
    
private:
    // hsgraph graph here is built as a pentagon + pentagram
    // with port 0 and port 1 connects to adjacent routers within pentagon or pentagram
    // port 2 connects the 2 corresponding routers in pentagon and pentagram
    // routers 0 to 4 are in pentagon the rest in pentagram
    // this is the offset to host ports 
    const static uint32_t routing_table[50][50]; 
    const static uint32_t neighbor_table[50][7];
    
    void id_to_location(int id, fishnetAddr *location) const;
    uint32_t port_for_router(uint32_t dest_router) const;
    bool is_neighbor(uint32_t tgt_rtr, uint32_t this_rtr) const;
};


class topo_hsgraph_event : public internal_router_event {
    
public:
    uint32_t src_subnet;
    bool is_forwarded;
    
    topo_hsgraph::fishnetAddr dest;
    
    topo_hsgraph_event() {}
    topo_hsgraph_event(const topo_hsgraph::fishnetAddr &dest) : dest(dest) {}
    ~topo_hsgraph_event() {}
    
    virtual internal_router_event* clone(void)
    {
        return new topo_hsgraph_event(*this);
    }
    
    void serialize_order(SST::Core::Serialization::serializer &ser)
    {
        internal_router_event::serialize_order(ser);
        ser & src_subnet;
        ser & is_forwarded;
        ser & dest.subnet;
        ser & dest.router;
        ser & dest.host;
    }

private:
    ImplementSerializable(SST::Merlin::topo_hsgraph_event);
};


}
}


#endif // COMPONENTS_MERLIN_TOPOLOGY_HSGRAPH_H

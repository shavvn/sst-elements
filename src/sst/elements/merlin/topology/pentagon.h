// -*- mode: c++ -*-

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

#ifndef COMPONENTS_MERLIN_TOPOLOGY_PENTAGON_H
#define COMPONENTS_MERLIN_TOPOLOGY_PENTAGON_H

#include <sst/core/event.h>
#include <sst/core/link.h>
#include <sst/core/params.h>

#include <string.h>

#include "sst/elements/merlin/router.h"

namespace SST {
namespace Merlin {
    
class topo_pentagon_event : public internal_router_event {
public:
    int dest_loc;
    
    topo_pentagon_event() {}
    topo_pentagon_event(int dest) {dest_loc = dest;}
    ~topo_pentagon_event() {}
    virtual internal_router_event* clone(void)
    {
        topo_pentagon_event* tpe = new topo_pentagon_event(*this); // what's THIS?
        tte->dest_loc = dest_loc;
        return tpe;
    }
    
    void serialize_order(SST::Core::Serialization::serializer &ser) 
    {
        internal_router_event::serialize_order(ser);
        ser & dest_loc;
    }
    
private:
    ImplementSerializable(SST::Merlin::topo_pentagon_event)
    
};

class topo_pentagon: public Topology {
    
    int router_id;
    int* id_loc;
    
    int port_start[5][2];
    
    int num_local_ports;
    int local_port_start;
    
    enum RouteAlgo {
        MINIMAL,
        VILIANT,
        ADAPTIVE
    };
    
    RouteAlgo algorithm;
    // local_id is the id within a pentagon
    uint32_t local_id;
    // group_id is the pentagon group id 
    uint32_t group_id;
    
public:
    topo_pentagon(Component* comp, Params& params);
    ~topo_torus();
    
    virtual void route(int port, int vc, internal_router_event* ev);
    virtual internal_router_event* process_input(RtrEvent* ev);
    
    virtual void routeInitData(int port, internal_router_event* ev, std::vector<int> &outPorts);
    virtual internal_router_event* process_InitData_input(RtrEvent* ev);

    virtual PortState getPortState(int port) const;
    virtual int computeNumVCs(int vns);
    virtual int getEndpointID(int port);

protected:
    virtual int choose_multipath(int start_port, int num_ports, int dest_dist);
    
private:
    void idToLocation(int id, int *location) const;
    int get_dest_router(int dest_id) const;
    int get_dest_local_port(int dest_id) const; 
};

}
}

#endif // COMPONENTS_MERLIN_TOPOLOGY_PENTAGON_H
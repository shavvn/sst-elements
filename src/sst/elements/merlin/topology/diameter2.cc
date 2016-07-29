// Copyright 2009-2016 Sandia Corporation. Under the terms
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
#include "diameter2.h"

#include <stdlib.h>
#include <fstream>

using namespace SST::Merlin;

topo_diameter2:: topo_diameter2(Component* comp, Params& params):
    Topology(comp)
{
    router_id = params.find<int>("id", -1);
    if (router_id == -1) {
        output.fatal(CALL_INFO, -1, "id must be set\n");
    }
    std::string netlist_name = params.find<std::string>("diameter2:file", "MMS.5.adj.txt");
    parse_netlist_file(netlist_name);
    
    host_ports = params.find<uint32_t>("diameter2:hosts_per_router", 1);
    // outgoing_ports = params.find<uint32_t>("diameter2:outgoing_ports", 0);
    local_ports = sizeof(neighbor_table) / sizeof(*neighbor_table);
    router = (uint32_t)params.find<int>("diameter2:router", 0);
    std::string interconnect = params.find<std::string>("diameter2:interconnect", "none");
    if (interconnect.compare("fishlite") == 0 ) {  
        // fishlite interconnect, requiring 1 extra port
        subnet = (uint32_t)params.find<int>("diameter2:subnet", 0);
        net_type = FISH_LITE;
        outgoing_ports = 1;
    } else if (interconnect.compare("fishnet") == 0) {
        // fishnet interconnect, requiring same amount of outgoing_ports as local_ports
        subnet = (uint32_t)params.find<int>("diameter2:subnet", 0);
        net_type = FISHNET;
        outgoing_ports = local_ports;
    } else {
        // just a diameter2 graph
        subnet = 0; 
        net_type = NONFISH;
        outgoing_ports = 0;
    }
    uint32_t total_ports = host_ports + local_ports + outgoing_ports;
    uint32_t num_ports = (uint32_t)params.find<int>("num_ports");
    if (total_ports > num_ports) {
        output.fatal(CALL_INFO, -1, "Need more ports to support the given topology!\n");
    }
    std::string route_algo = params.find<std::string>("diameter2:algorithm", "minimal");
    if (!route_algo.compare("minimal")) {
        // TODO implement other algorithms later
        algorithm = MINIMAL;
    }
    
}

topo_diameter2:: ~topo_diameter2() {
    delete(routing_table);
    delete(neighbor_table);
}

void topo_diameter2::route(int port, int vc, internal_router_event* ev)
{
    topo_diameter2_event *tp_ev = static_cast<topo_diameter2_event*>(ev);

    if ( (uint32_t)port >= (host_ports + local_ports) ) {
        /* Came in from another subnet.  Increment VC */
        tp_ev->setVC(vc+1);
    }
    
    uint32_t next_port = 0;
    if (tp_ev->dest.subnet != subnet) {
        // target is not this subnet
        // only implement angelfish_lite for now..
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
                    next_port = host_ports + local_ports -1;
                } else {
                    if (is_neighbor(mid_rtr)) { // if router in mid_rtr 's neighbor
                        bool find_rtr = false;
                        for (uint32_t i = 0; i < outgoing_ports; i++) {
                            uint32_t neighbor = neighbor_table[i];
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


internal_router_event* topo_diameter2::process_input(RtrEvent* ev)
{
    topo_diameter2::fishnetAddr destAddr = {0, 0, 0};
    id_to_location(ev->request->dest, &destAddr);
    topo_diameter2_event *tp_ev = new topo_diameter2_event(destAddr);
    // if to implement other algorithm, need to add stuff..
    
    tp_ev->src_subnet = subnet;
    tp_ev->setEncapsulatedEvent(ev);
    // TODO not sure about this vn thing
    tp_ev->setVC(ev->request->vn *3);
    return tp_ev;
}


// this is to figure out what are the possible outports for a given input port
void topo_diameter2::routeInitData(int port, internal_router_event* ev, std::vector<int> &outPorts)
{
    topo_diameter2_event *tp_ev = static_cast<topo_diameter2_event*>(ev);
    if ( tp_ev->dest.host == (uint32_t)INIT_BROADCAST_ADDR ) {
        uint32_t total_ports = host_ports + local_ports + outgoing_ports;
        if ( (uint32_t)port >= (host_ports + local_ports ) ) {
            /* Came in from another subnet.
             * Send to locals, and other routers in subnet
             */
            tp_ev->is_forwarded = false;
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
            if (tp_ev->is_forwarded == false) {
                tp_ev->is_forwarded = true; 
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


internal_router_event* topo_diameter2::process_InitData_input(RtrEvent* ev)
{
    topo_diameter2::fishnetAddr destAddr;
    id_to_location(ev->request->dest, &destAddr);
    topo_diameter2_event *tp_ev = new topo_diameter2_event(destAddr);
    tp_ev->src_subnet = subnet;
    tp_ev->is_forwarded = false;
    tp_ev->setEncapsulatedEvent(ev);
    return tp_ev;
}

void topo_diameter2::parse_netlist_file(std::string file_name) {
    uint32_t num_nodes = 0;
    uint32_t num_links = 0;
    uint32_t num_edges = 0;
    std::ifstream input(file_name);
    std::string line;
    uint32_t tmp_num = 0;
    uint32_t line_num = 0;
    uint32_t **adj_table;
    while ( std::getline( input, line) ) {
        uint32_t line_index = 0;
        if (line_num == 0) {
            sscanf(line.c_str(), "%u %u", &num_nodes, &num_links);
            num_edges = num_links * 2 / num_nodes;
            adj_table = new uint32_t*[num_nodes];
            for (int i = 0; i < num_nodes; i++) {
                adj_table[i] = new uint32_t[num_edges];
            }
        } else {
            std::stringstream stream(line);
            while(1) {
                stream >> tmp_num;
                if (!stream) break;
                adj_table[line_num-1][line_index] = tmp_num;
                line_index += 1;    
            }
        }
        line_num += 1;
    }
    
    uint32_t this_node = 0;
    routing_table = new uint32_t[num_nodes];
    for (int i = 0; i < num_nodes; i++) {
        routing_table[i] = UINT32_MAX;
    }
    // first level routing
    for (int i = 0; i < num_edges; i++) {
        uint32_t neighbor = adj_table[this_node][i];
        routing_table[neighbor] = i;
    }
    // second level routing
    // for those non-complete moore graphs, if the routing table
    // entry is set then pass
    for (int i = 0; i < num_edges; i++) {
        uint32_t neighbor = adj_table[this_node][i];
        for (int j = 0; j < num_edges; j++) {
            uint32_t neighbors_neighbor = adj_table[neighbor][j];
            if (neighbors_neighbor == this_node) {
                continue;
            } 
            if (routing_table[neighbors_neighbor] == UINT32_MAX) {
                routing_table[neighbors_neighbor] = i;
            } 
        }
    }
    
    neighbor_table = new uint32_t[num_edges];
    for (int i = 0; i < num_edges; i++) {
        neighbor_table[i] = adj_table[this_node][i];
    }
    // clean up adjecent table since we have routing table
    // and neighbor table now
    for (int i = 0; i < num_nodes; i++) {
        delete(adj_table[i]);
    }
    delete(adj_table);
}


Topology::PortState topo_diameter2::getPortState(int port) const
{
    if ((uint32_t)port < host_ports) return R2N;
    else return R2R;
}


std::string topo_diameter2::getPortLogicalGroup(int port) const
{
    if ((uint32_t)port < host_ports) return "host";
    if ((uint32_t)port >= host_ports && (uint32_t)port < (host_ports + local_ports)) return "group";
    else return "global";
}


int topo_diameter2::getEndpointID(int port)
{
    return (subnet* (routers_per_subnet * host_ports)) + router*host_ports + port;
}


void topo_diameter2::id_to_location(int id, fishnetAddr *location) const
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


uint32_t topo_diameter2::port_for_router(uint32_t dest_router) const 
{
    // just looking up routing table
    return host_ports + routing_table[dest_router];
}


// see if the target router is a neighbor of this router
bool topo_diameter2::is_neighbor(uint32_t tgt_rtr) const
{
    for (uint32_t i = 0; i < local_ports; i++) {
        if (tgt_rtr == neighbor_table[i]) {
            return true;
        }
    }
    return false;
}


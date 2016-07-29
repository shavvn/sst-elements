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
    std::string route_algo = params.find<std::string>("hsgraph:algorithm", "minimal");
    if (!route_algo.compare("minimal")) {
        // TODO implement other algorithms later
        algorithm = MINIMAL;
    }
    
}

topo_diameter2:: ~topo_diameter2() {
    delete(routing_table);
    delete(neighbor_table);
}

topo_diameter2::parse_netlist_file(std::string file_name) {
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



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
    std::string netlistFile = params.find<std::string("file, "");
    netlistInput = fopen(netlistFile.c_str(), "rt");
    if(NULL == netlistInput) {
		fprintf(stderr, "Fatal: Unable to open file: %s in text reader.\n",
			netlistFile.c_str());
		exit(-1);
	}
    
}

topo_diameter2:: ~topo_diameter2() {
    if (NULL != netlistInput) {
        fclose(netlistInput);
    }
}

topo_diameter2::parse_netlist_file() {
    uint32_t num_nodes = 0;
    uint32_t num_links = 0;
    fscanf(netlistInput, "%u %u", &num_nodes, &num_links);
    std::string rawInput;
    std::vector<uint32_t> numbers;
    while (getline(netlistInput, rawInput, " ")) {
        std::stoi(rawInput)
    }
    routers_per_subnet = 
}



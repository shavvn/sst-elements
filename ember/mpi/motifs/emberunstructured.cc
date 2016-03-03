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

#include "emberunstructured.h"
#include "../../../scheduler/InputParser.h"
#include "../../embercustommap.h"


using namespace SST::Ember;

EmberUnstructuredGenerator::EmberUnstructuredGenerator(SST::Component* owner, Params& params) :
	EmberMessagePassingGenerator(owner, params, "Unstructured"), 
	m_loopIndex(0)
{
	graphFile  = params.find_string("arg.graphfile", "-1");
	if(graphFile.compare("-1") == 0)
		fatal(CALL_INFO, -1, "Communication graph file must be specified for unstructured motifs!\n");

	nx  = (uint32_t) params.find_integer("arg.nx", 100);
	ny  = (uint32_t) params.find_integer("arg.ny", 100);
	nz  = (uint32_t) params.find_integer("arg.nz", 100);

	items_per_cell = (uint32_t) params.find_integer("arg.fields_per_cell", 1);
	sizeof_cell = (uint32_t) params.find_integer("arg.datatype_width", 8);

	iterations = (uint32_t) params.find_integer("arg.iterations", 1);

	x_down = -1;
	x_up   = -1;
	y_down = -1;
	y_up   = -1;
	z_down = -1;
	z_up   = -1;

	configure();
}


void EmberUnstructuredGenerator::configure()
{
	//Get the name of the graph file from CustomRankMap object
	EmberCustomRankMap* cm = dynamic_cast<EmberCustomRankMap*>(m_rankMap);
	if(cm == NULL)
		fatal(CALL_INFO, -1, "CustomRankMap is NULL!\n");
	
	//std::cout << "graph file: " << graphFile.c_str() <<  std::endl;

	//Read raw communication from the graph file
	SST::Scheduler::CommParser commParser;
	rawCommMap = commParser.readCommFile(graphFile, size());

	for(unsigned int i = 0; i < rawCommMap->size(); i++){
        //std::cout << "rawCommMap(" << i << "):" << std::endl;

        for(std::map<int, int>::iterator it = rawCommMap->at(i).begin(); it != rawCommMap->at(i).end(); it++){
        	//std::cout << i << " communicates with " << it->first << std::endl;
		}
	}

	//Create the actual communication map based on the custom task mapping
	//Ex: TaskMapping on Node0 [3,5,7,120] -> CustomMap[0]=3, CustomMap[1]=5, CustomMap[2]=7, CustomMap[3]=120
	//	  If rawCommMap says Task0 communicates Task1 -> CommMap says Task3 communicates Task5
	CommMap = new std::vector<std::map<int,int> >;
	CommMap->resize(size());

	int srcTask, destTask;
	for(unsigned int i = 0; i < CommMap->size(); i++){
        srcTask = cm->CustomMap[i];
        //std::cout << "Rank(" << i << ") is in fact Rank(" << srcTask << ")"<< std::endl;

        for(std::map<int, int>::iterator it = rawCommMap->at(i).begin(); it != rawCommMap->at(i).end(); it++){
        	destTask = cm->CustomMap[it->first];
        	CommMap->at(srcTask)[destTask] = 1; //1 could be changed to the weight (it->second) in the future 
        	//std::cout << srcTask << " communicates with " << destTask << std::endl;
		}
	}

	//unsigned worldSize = size();

    if(0 == rank()) {
		output("Unstructured communication motif iterations: %" PRIu32 "\n", iterations);
		output("Unstructured communication motif iterms/cell: %" PRIu32 "\n", items_per_cell);
	}


	/*
	for(unsigned int i = 0; i < rawCommMap->size(); i++){
	    delete [] rawCommMap->at(i);
	}
	delete rawCommMap;
	*/
	//output("My rank is: %" PRIu32 "\n", rank()); // NetworkSim
}

bool EmberUnstructuredGenerator::generate( std::queue<EmberEvent*>& evQ ) 
{
    verbose(CALL_INFO, 1, 0, "loop=%d\n", m_loopIndex );

		std::vector<MessageRequest*> requests;

        for(std::map<int, int>::iterator it = CommMap->at(rank()).begin(); it != CommMap->at(rank()).end(); it++){
			MessageRequest*  req  = new MessageRequest();
			requests.push_back(req);
        	
        	//std::cout << rank() << " communicates with " << it->first << std::endl;
			enQ_irecv( evQ, (int32_t)it->first, items_per_cell * sizeof_cell * ny * nz, 0, GroupWorld, req);
			enQ_send( evQ , (int32_t)it->first, items_per_cell * sizeof_cell * ny * nz, 0, GroupWorld);
		}

		for(uint32_t i = 0; i < requests.size(); ++i) {
			enQ_wait( evQ, requests[i]);
		}

		requests.clear();

		/*
		if(x_down > -1) {
			MessageRequest*  req  = new MessageRequest();
			requests.push_back(req);

			enQ_irecv( evQ, x_down, items_per_cell * sizeof_cell * ny * nz, 0, GroupWorld, req);
		}

		if(x_up > -1) {
			MessageRequest*  req  = new MessageRequest();
			requests.push_back(req);

			enQ_irecv( evQ, x_up, items_per_cell * sizeof_cell * ny * nz, 0, GroupWorld, req);
		}

		if(x_down > -1) {
			enQ_send( evQ ,x_down, items_per_cell * sizeof_cell * ny * nz, 0, GroupWorld);
		}

		if(x_up > -1) {
			enQ_send( evQ ,x_up, items_per_cell * sizeof_cell * ny * nz, 0, GroupWorld);
		}

		for(uint32_t i = 0; i < requests.size(); ++i) {
			enQ_wait( evQ, requests[i]);
		}

		requests.clear();

		if(nsCopyTime > 0) {
			enQ_compute( evQ, nsCopyTime);
		}
		*/

    if ( ++m_loopIndex == iterations ) {
        return true;
    } else {
        return false;
    }

}

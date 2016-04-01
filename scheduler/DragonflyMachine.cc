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

#include "DragonflyMachine.h"

#include "Job.h"
#include "output.h"

#include "sst/core/rng/marsaglia.h"

#include <algorithm>
#include <sstream>

using namespace SST::Scheduler;
using namespace std;

DragonflyMachine::DragonflyMachine(int routersPerGroup, int portsPerRouter,
    int opticalsPerRouter, int nodesPerRouter, int coresPerNode, localTopo ltopo,
    globalTopo gtopo, double** D_matrix)
    : Machine(getNumNodes(opticalsPerRouter, routersPerGroup, nodesPerRouter), 
              coresPerNode,
              D_matrix,
              getNumLinks(portsPerRouter, nodesPerRouter, routersPerGroup, opticalsPerRouter)),
      ltopo(ltopo), gtopo(gtopo),
      routersPerGroup(routersPerGroup),
      nodesPerRouter(nodesPerRouter),
      portsPerRouter(portsPerRouter),
      opticalsPerRouter(opticalsPerRouter),
      numGroups(getNumGroups(routersPerGroup, opticalsPerRouter)),
      numNodes(getNumNodes(opticalsPerRouter, routersPerGroup, nodesPerRouter)),
      numRouters(getNumRouters(routersPerGroup, opticalsPerRouter)),
      numLinks(getNumLinks(portsPerRouter, nodesPerRouter, routersPerGroup, opticalsPerRouter))
{   
    //sanity check
    if (portsPerRouter < nodesPerRouter + opticalsPerRouter)
        schedout.fatal(CALL_INFO, 1, "DragonflyMachine: Too few ports!\n");
    if (opticalsPerRouter % 2 != 0)
        schedout.fatal(CALL_INFO, 1, "DragonflyMachine: opticalsPerRouter must be an even number!\n");
    
    int total;
    int dist_it;
    int linkCount = 0;
    int localLinksPerRouter = -1;

    switch (ltopo) {
        case ALLTOALL:
            localLinksPerRouter = routersPerGroup - 1;
            if (portsPerRouter != nodesPerRouter + opticalsPerRouter + localLinksPerRouter)
                schedout.fatal(CALL_INFO, 1, "DragonflyMachine: # of ports does not match"
                    "all-to-all local topology!\n");
            break;
        default:
            goto unknown_topo;
    }

    //init routers    
    routers = vector<map<int,int> >(numRouters);
    
    //build local groups
    switch (ltopo) {
    case ALLTOALL:
        for (int gID = 0; gID < numGroups; gID++) {
            for (int lID = 0; lID < routersPerGroup; lID++) {
                int rID = gID * routersPerGroup + lID;
                for (int otherID = rID + 1; otherID < (gID + 1) * routersPerGroup; otherID++) {
                    routers[rID][otherID] = linkCount;
                    routers[otherID][rID] = linkCount;
                    linkCount++;
                }
            }
        }
        break;
    default:
        goto unknown_topo;
    }
    
    //build global groups
    switch (gtopo) {
    case CIRCULANT:
        for (int gID = 0; gID < numGroups; gID++) {
            for (int lID = 0; lID < routersPerGroup; lID++) {
                int rID = gID * routersPerGroup + lID;
                for (int opt = 1; opt <= opticalsPerRouter / 2; opt++) {
                    int otherID = (rID + (lID * opticalsPerRouter / 2 + opt) * routersPerGroup)
                      % numRouters;
                    routers[rID][otherID] = linkCount;
                    routers[otherID][rID] = linkCount;
                    linkCount++;
                }
            }
        }
        break;
    case ABSOLUTE:
    {
        int maxLinks = opticalsPerRouter + localLinksPerRouter;
        for (int gID = 0; gID < numGroups; gID++) {
            int targetgID = gID + 1;
            for (int lID = 0; lID < routersPerGroup; lID++) {
                int rID = gID * routersPerGroup + lID;
                while(routers[rID].size() != maxLinks) {
                    int otherID = targetgID * routersPerGroup;
                    while (routers[otherID].size() == maxLinks) {
                        otherID = (otherID + 1) % numRouters;
                    }
                    routers[rID][otherID] = linkCount;
                    routers[otherID][rID] = linkCount;
                    linkCount++;
                    targetgID = (targetgID + 1) % numGroups;
                }
            }
        }
        break;
    }
    case RELATIVE:
    {
        for (int gID = 0; gID < numGroups; gID++) {
            int targetgID = gID + 1;
            for (int lID = 0; lID < routersPerGroup / 2; lID++) {
                int rID = gID * routersPerGroup + lID;
                int targetlID = routersPerGroup - lID - 1;
                for (int opt = 0; opt < opticalsPerRouter; opt++) {
                    int targetID = (targetgID * routersPerGroup + targetlID) % numRouters;
                    routers[rID][targetID] = linkCount;
                    routers[targetID][rID] = linkCount;
                    linkCount++;
                    targetgID++;
                }
            }
        }
        break;
    }
    default:
        goto unknown_topo;
    }
    
    //node-to-router link indices are in-order starting from (numLinks - nodesPerRouter * numRouters)
    //sanity check
    if (linkCount != numLinks - nodesPerRouter * numRouters) {
        schedout.fatal(CALL_INFO, 1, "DragonflyMachine: Network setup failed!\n");
    }
    
    //For nearestAllocMapper:
    //Fill nodes at distances for fast access
    //Calculate with breadth-first from node 0. Assume symmetrical network
    total = 1;
    dist_it = 1;
    nodesAtDistances.push_back(1);
    while (total < numNodes) {
        list<int> *temp_list = getFreeAtDistance(0, dist_it);
        nodesAtDistances.push_back(temp_list->size());
        total += temp_list->size();
        delete temp_list;
        dist_it++;
    }
    
    return;
unknown_topo:
    schedout.fatal(CALL_INFO, 1, "DragonflyMachine(): Unknown local or global topology\n");
}

AllocInfo* DragonflyMachine::getBaselineAllocation(Job* job) const
{
    int nodesNeeded = (int) ceil((float) job->getProcsNeeded() / coresPerNode);
    if (nodesNeeded > numNodes) {
        schedout.fatal(CALL_INFO, 1, "Baseline allocation requested for %d nodes for a %d-node machine.", nodesNeeded, numNodes);
    }

    AllocInfo* allocInfo = new AllocInfo(job, *this);
	for(int i = 0; i < nodesNeeded; i++){
		allocInfo->nodeIndices[i] = i;
	}
	
	return allocInfo;
}

std::string DragonflyMachine::getSetupInfo(bool comment)
{
    std::string com;
    if (comment) com="# ";
    else com="";
    std::stringstream ret;
    ret << com;
    ret << "local topology: " << ltopo;
    ret << ", global topology: " << gtopo;
    ret << ", num links: " << numLinks;
    ret << ", num routers: " << numRouters;
    ret << ", num groups: " << numGroups;
    ret << ", " << coresPerNode << " cores per node";
    return ret.str();
}

int DragonflyMachine::getNodeDistance(int node0, int node1) const
{
    list<int>* links = getRoute(node0, node1, 1);
    int size = links->size();
    delete links;
    return size;
}

list<int>* DragonflyMachine::getFreeAtDistance(int center, int distance) const
{
    list<int>* nodes = new list<int>();
    if (distance <= 1) {
        return nodes;
    }
  
    //apply breadth-first search to find routers at (distance - 2)
    //  to account for the node-to-router hop
    vector<bool> marked(numRouters, false);
    list<int>* rQ1 = new list<int>();
    int center_rID = routerOf(center);
    rQ1->push_back(center_rID);
    marked[center_rID] = true;
    
    while (distance > 2) {
        list<int>* rQ2 = new list<int>();
        while (!rQ1->empty()) {
            //add connected nodes to rQ2
            int rID = rQ1->front();
            rQ1->pop_front();

            for (map<int, int>::const_iterator it = routers[rID].begin();
                it != routers[rID].end(); it++) {
                if (!marked[it->first]){
                    marked[it->first] = true;
                    rQ2->push_back(it->first);
                }
            }
        }
        //we don't care about routers with smaller distance
        delete rQ1;
        rQ1 = rQ2;
        distance--;
    }
    for (int i = 0; i < numRouters; i++)
        rQ1->push_back(i); 
    
    //get all nodes connected to the routers in the queue
    while (!rQ1->empty()) {
        int rID = rQ1->front();
        rQ1->pop_front();
        for (int nID = rID * nodesPerRouter; nID < (rID + 1) * nodesPerRouter; nID++) {
            if (isFree(nID) && nID != center) {
                nodes->push_back(nID);
            }
        }
    }
    
    delete rQ1;

    return nodes;
}

int DragonflyMachine::nodesAtDistance(int dist) const
{    
    if (dist >= (int) nodesAtDistances.size())
        return 0;
    else
        return nodesAtDistances[dist];
}

list<int>* DragonflyMachine::getRoute(int node0, int node1, double commWeight) const
{
    list<int>* links = new list<int>();
    
    if(node0 == node1) {
        return links;
    }
    
    int rID0 = routerOf(node0);
    int rID1 = routerOf(node1);
    
    if (rID0 != rID1) {
        //apply breadth-first search to get all links from rID0 to rID1
  
        list<int> routerQ;
        vector<bool> marked(numRouters, false);
        vector<int> prevRouter(numRouters, -1);
        routerQ.push_back(rID0);
        
        while (!routerQ.empty()) {
            int curNode = routerQ.front();
            routerQ.pop_front();
            for (map<int,int>::const_iterator it = routers[curNode].begin();
                it != routers[curNode].end(); it++) {
                if (!marked[it->first]){
                    marked[it->first] = true;
                    routerQ.push_back(it->first);
                    prevRouter[it->first] = curNode;
                    if (it->first == rID1) {
                        routerQ.clear();
                        break;
                    }
                }
            }
        }
    
        //Found the shortest route. Now add links
        int cur = rID1;
        int previous = prevRouter[rID1];
        do {
            links->push_back(routers.at(cur).at(previous));
            cur = previous;
            previous = prevRouter[cur];
        } while (cur != rID0);
    }

    int nInterRouterLinks = numLinks - nodesPerRouter * numRouters;
    
    //node-to-router-hop
    links->push_front(nInterRouterLinks + node0);
    
    //router-to-node hop
    links->push_back(nInterRouterLinks + node1);

    return links;
}

// Copyright 2009-2017 Sandia Corporation. Under the terms
// of Contract DE-NA0003525 with Sandia Corporation, the U.S.
// Government retains certain rights in this software.
//
// Copyright (c) 2009-2017, Sandia Corporation
// All rights reserved.
//
// Portions are copyright of other developers:
// See the file CONTRIBUTORS.TXT in the top level directory
// the distribution for more information.
//
// This file is part of the SST software package. For license
// information, see the LICENSE file in the top level directory of the
// distribution.

#include <sst_config.h>
#include "sst/elements/memHierarchy/util.h"
#include "membackend/dummySimBackend.h"

using namespace SST;
using namespace SST::MemHierarchy;

DummySimMemory::DummySimMemory(Component *comp, Params &params) : 
    SimpleMemBackend(comp, params),
    callBackFunc(std::bind(&DummySimMemory::DummySimDone, this, std::placeholders::_1))
{
    std::string configIniFilename = params.find<std::string>("config_ini", NO_STRING_DEFINED);
    if(NO_STRING_DEFINED == configIniFilename)
        output->fatal(CALL_INFO, -1, "Model must define a 'device_ini' file parameter\n");
    memSystem = new dramcore::MemorySystem(configIniFilename, callBackFunc);
}


bool DummySimMemory::issueRequest(ReqId id, Addr addr, bool isWrite, unsigned ){
    bool ok = memSystem->InsertReq(addr, isWrite);
    if (!ok) return false; 
#ifdef __SST_DEBUG_OUTPUT__
    output->debug(_L10_, "Issued transaction for address %" PRIx64 "\n", (Addr)addr);
#endif
    dramReqs[addr].push_back(id);
    return ok;
}



void DummySimMemory::clock(){
    memSystem->ClockTick();
}



void DummySimMemory::finish(){
    memSystem->PrintStats();
}



void DummySimMemory::DummySimDone(uint64_t addr){ 
    std::deque<ReqId> &reqs = dramReqs[addr];
#ifdef __SST_DEBUG_OUTPUT__
    output->debug(_L10_, "Memory Request for %" PRIx64 " Finished [%zu reqs]\n", (Addr)addr, reqs.size());
#endif
    if (reqs.size() == 0) output->fatal(CALL_INFO, -1, "Error: reqs.size() is 0 at DRAMSimMemory done\n");
    ReqId reqId = reqs.front();
    reqs.pop_front();
    if(0 == reqs.size())
        dramReqs.erase(addr);

    handleMemResponse(reqId);
    return;
}

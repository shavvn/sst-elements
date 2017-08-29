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


#ifndef _H_SST_MEMH_DUMMYSIM_BACKEND
#define _H_SST_MEMH_DUMMYSIM_BACKEND

#include "membackend/memBackend.h"

#ifdef DEBUG
#define OLD_DEBUG DEBUG
#undef DEBUG
#endif

#include <memory_system.h>

#ifdef OLD_DEBUG
#define DEBUG OLD_DEBUG
#undef OLD_DEBUG
#endif

namespace SST {
namespace MemHierarchy {

class DummySimMemory : public SimpleMemBackend {
public:
    DummySimMemory(Component *comp, Params &params);
	virtual bool issueRequest(ReqId, Addr, bool, unsigned );
    virtual void clock();
    virtual void finish();

protected:
    void DummySimDone(uint64_t addr);
    dramcore::MemorySystem *memSystem;
    std::function<void(uint64_t)> callBackFunc;
    std::map<uint64_t, std::deque<ReqId> > dramReqs;
};

}
}

#endif

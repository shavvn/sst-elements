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
#include <sst/core/params.h>
#include <sst/core/rng/marsaglia.h>
#include <sst/elements/miranda/generators/gupsgen.h>

using namespace SST::Miranda;

#define DO_VERIFY 1
#if defined(DO_VERIFY)
static std::vector<uint64_t> verifyData;
#define VERIFY(addr, v) \
    if (verifyData[addr] != v) fprintf(stderr, "Expected %llu @ 0x%llx. Got %llu\n", verifyData[addr], addr, v)
#define UPDATE(addr, v) verifyData[addr] = v
#endif

GUPSGenerator::GUPSGenerator( Component* owner, Params& params ) :
	RequestGenerator(owner, params) {

	const uint32_t verbose = (uint32_t) params.find_integer("verbose", 0);

	out = new Output("GUPSGenerator[@p:@l]: ", verbose, 0, Output::STDOUT);

	iterations = (uint64_t) params.find_integer("iterations", 1);
	issueCount = ((uint64_t) params.find_integer("count", 1000)) * iterations;
	reqLength  = (uint64_t) params.find_integer("length", 8);
	maxAddr    = (uint64_t) params.find_integer("max_address", 524288);

	rng = new MarsagliaRNG(11, 31);

	out->verbose(CALL_INFO, 1, 0, "Will issue %" PRIu64 " operations\n", issueCount);
	out->verbose(CALL_INFO, 1, 0, "Request lengths: %" PRIu64 " bytes\n", reqLength);
	out->verbose(CALL_INFO, 1, 0, "Maximum address: %" PRIu64 "\n", maxAddr);

	issueOpFences = params.find_string("issue_op_fences", "yes") == "yes";
	atomic = params.find_string("use_atomics", "no") == "yes";

#if defined(DO_VERIFY)
	verifyData.resize(maxAddr);
#endif
}

GUPSGenerator::~GUPSGenerator() {
	delete out;
	delete rng;
}

void GUPSGenerator::generate(MirandaRequestQueue<GeneratorRequest*>* q) {

	const uint64_t rand_addr = rng->generateNextUInt64()>>8;
	// Ensure we have a reqLength aligned request
	const uint64_t addr_under_limit = (rand_addr % (maxAddr - reqLength));
	const uint64_t addr = addr_under_limit - (addr_under_limit % reqLength);

	out->verbose(CALL_INFO, 4, 0, "Generating next request number: %" PRIu64 " at address 0x%" PRIx64 "\n", issueCount, addr);

	MemoryOpRequest* readAddr = new MemoryOpRequest(addr, reqLength, READ);
	MemoryOpRequest* writeAddr = new MemoryOpRequest(addr, reqLength, WRITE);

	writeAddr->addDependency(readAddr->getRequestID());

	if ( atomic ) {
	    readAddr->setAtomic();
	    writeAddr->setAtomic();
	    readAddr->setCallback(std::bind(&GUPSGenerator::doIncrement, this, readAddr, writeAddr));
	}

	if ( issueOpFences ) {
	    FenceOpRequest *fence = new FenceOpRequest();
	    readAddr->addDependency(fence->getRequestID());
	    q->push_back(fence);
	}

	q->push_back(readAddr);
	q->push_back(writeAddr);

	issueCount--;
}

bool GUPSGenerator::isFinished() {
	return (issueCount == 0);
}

void GUPSGenerator::completed() {

}

void GUPSGenerator::doIncrement(MemoryOpRequest *read, MemoryOpRequest *write)
{
    static int cnt = 0;
    out->verbose(CALL_INFO, 4, 0,
	    "Cycle %d: Increment 0x%" PRIx64 " from %" PRIu64 " to %" PRIu64"\n",
	    cnt, read->getAddress(), read->getPayload<uint64_t>(), read->getPayload<uint64_t>()+1);
#if defined(DO_VERIFY)
	VERIFY(read->getAddress(), read->getPayload<uint64_t>());
	UPDATE(read->getAddress(), read->getPayload<uint64_t>()+1);
#endif
    switch ( reqLength ) {
    case 8: write->setPayload(read->getPayload<uint64_t>() + 1); break;
    case 4: write->setPayload(read->getPayload<uint32_t>() + 1); break;
    case 2: write->setPayload(read->getPayload<uint16_t>() + 1); break;
    case 1: write->setPayload(read->getPayload<uint8_t>() + 1);  break;
    default:
	out->fatal(CALL_INFO, -1, "Unable to do increments on size %" PRIu64 "\n", reqLength);
    }
    cnt++;
}


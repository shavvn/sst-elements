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


GUPSGenerator::GUPSGenerator( Component* owner, Params& params ) :
	RequestGenerator(owner, params) {

	const uint32_t verbose = (uint32_t) params.find_integer("verbose", 0);

    char prefix[128];
    snprintf(prefix, 127, "GUPSGenerator(%s)[@p:@l]: ", owner->getName().c_str());
	out = new Output(prefix, verbose, 0, Output::STDOUT);

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
	doVerify = params.find_string("do_verify", "no") == "yes";

    if ( doVerify )
        verifyData.resize(maxAddr / reqLength, 0);

    if ( doVerify || atomic ) {
        switch ( reqLength ) {
        case 8:
            callback = std::bind(&GUPSGenerator::doIncrement<uint64_t>, this,
                    std::placeholders::_1, std::placeholders::_2);
            break;
        case 4:
            callback = std::bind(&GUPSGenerator::doIncrement<uint32_t>, this,
                    std::placeholders::_1, std::placeholders::_2);
            break;
        case 2:
            callback = std::bind(&GUPSGenerator::doIncrement<uint16_t>, this,
                    std::placeholders::_1, std::placeholders::_2);
            break;
        case 1:
            callback = std::bind(&GUPSGenerator::doIncrement<uint8_t>, this,
                    std::placeholders::_1, std::placeholders::_2);
            break;
        default:
            out->fatal(CALL_INFO, -1, "Cannot verify or do atomics on size %" PRIu64 " variables (length)\n", reqLength);

        }
    }
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

	if ( issueOpFences ) {
	    FenceOpRequest *fence = new FenceOpRequest();
	    readAddr->addDependency(fence->getRequestID());
	    q->push_back(fence);
	}


	MemoryOpRequest* readAddr = new MemoryOpRequest(addr, reqLength, READ);
	MemoryOpRequest* writeAddr = new MemoryOpRequest(addr, reqLength, WRITE);

	writeAddr->addDependency(readAddr->getRequestID());

    if ( atomic || doVerify ) {
        readAddr->setCallback(std::bind(callback, readAddr, writeAddr));
    }

	if ( atomic ) {
	    readAddr->setAtomic();
	    writeAddr->setAtomic();
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



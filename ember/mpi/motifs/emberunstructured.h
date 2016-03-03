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


#ifndef _H_EMBER_UNSTRUCTURED
#define _H_EMBER_UNSTRUCTURED

#include <sst/core/component.h>
#include "mpi/embermpigen.h"

namespace SST {
namespace Ember {

class EmberUnstructuredGenerator : public EmberMessagePassingGenerator {

public:
	EmberUnstructuredGenerator(SST::Component* owner, Params& params);
	~EmberUnstructuredGenerator() {}
	void configure();
	bool generate( std::queue<EmberEvent*>& evQ );

private:
	std::string graphFile;
	uint32_t m_loopIndex;

	uint32_t iterations;

	// Share these over all instances of the motif

	uint32_t nx;
	uint32_t ny;
	uint32_t nz;
	uint32_t items_per_cell;
	uint32_t sizeof_cell;

	int32_t  x_down;
	int32_t  x_up;
	int32_t  y_down;
	int32_t  y_up;
	int32_t  z_down;
	int32_t  z_up;

	std::vector<std::map<int,int> >* rawCommMap; //raw communication map taken from input graph file
	std::vector<std::map<int,int> >* CommMap; //updated communication map based on the task mapping

};

}
}

#endif

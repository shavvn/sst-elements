
#include <sst_config.h>
#include "vinstload.h"

VanadisInstructionLoader::VanadisInstructionLoader( Component* owner, Params& params ) :
	nextAddr(0) {

	const uint32_t verbose = (uint32_t) params.find_integer("verbose");
	output = new Output("InstLoad[@p:@l]: ", verbose, 0, Output::STDOUT);	
}

VanadisInstructionLoader::~VanadisInstructionLoader() {
	delete output;
}

void VanadisInstructionLoader::clear() {

}



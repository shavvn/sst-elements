
#ifndef _H_SST_VANADIS_INSTRUCTION_LOAD
#define _H_SST_VANADIS_INSTRUCTION_LOAD

#include <sst/core/subcomponent.h>
#include <sst/core/params.h>

namespace SST {
namespace Vanadis {

class VanadisInstructionLoader : public SST::SubComponent {

  public:
    	VanadisInstructionLoader( Component* owner, Params& params );
    	~VanadisInstructionLoader();
	void setNextAddress(const uint64_t nAddr) { nextAddr = nAddr; }
	uint64_t getNextAddress() const { return nextAddr; }
    	void clear();

  protected:
	Output*	output;
	uint64_t nextAddr;	

};

}
}

#endif

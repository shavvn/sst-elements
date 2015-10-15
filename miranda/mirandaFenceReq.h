
#ifndef _H_MIRANDA_FENCE_REQ
#define _H_MIRANDA_FENCE_REQ

#include "mirandaReq.h"

namespace SST {
namespace Miranda {

class FenceOpRequest : public GeneratorRequest {
public:
	FenceOpRequest() : GeneratorRequest() {}
	~FenceOpRequest() {}
	ReqOperation getOperation() const { return REQ_FENCE; }
};

}
}

#endif

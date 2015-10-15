
#ifndef _H_MIRANDA_MEMORY_REQUEST
#define _H_MIRANDA_MEMORY_REQUEST

#include "mirandaReq.h"

namespace SST {
namespace Miranda {

class MemoryOpRequest : public GeneratorRequest {
public:
        MemoryOpRequest(const uint64_t cAddr,
                const uint64_t cLength,
                const ReqOperation cOpType) :
                GeneratorRequest(),
                addr(cAddr), length(cLength), op(cOpType) {}
        ~MemoryOpRequest() {}
        ReqOperation getOperation() const { return op; }
        bool isRead() const { return op == READ; }
        bool isWrite() const { return op == WRITE; }
        uint64_t getAddress() const { return addr; }
        uint64_t getLength() const { return length; }

protected:
        uint64_t addr;
        uint64_t length;
        ReqOperation op;
};

}
}

#endif

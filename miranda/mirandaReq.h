
#ifndef _H_MIRANDA_GENERATOR_REQUEST
#define _H_MIRANDA_GENERATOR_REQUEST

namespace SST {
namespace Miranda {

#include <vector>

static uint64_t nextGeneratorRequestID = 0;

class GeneratorRequest {
public:
	GeneratorRequest() {
		reqID = nextGeneratorRequestID++;
	}

	virtual ~GeneratorRequest() {}
	virtual ReqOperation getOperation() const = 0;
	uint64_t getRequestID() const { return reqID; }

	void addDependency(uint64_t depReq) {
		dependsOn.push_back(depReq);
	}

	void satisfyDependency(const GeneratorRequest* req) {
		satisfyDependency(req->getRequestID());
	}

	void satisfyDependency(const uint64_t req) {
		std::vector<uint64_t>::iterator searchDeps;

		for(searchDeps = dependsOn.begin(); searchDeps != dependsOn.end(); searchDeps++) {
			if( req == (*searchDeps) ) {
				dependsOn.erase(searchDeps);
				break;
			}
		}
	}

	bool canIssue() {
		return dependsOn.empty();
	}

	uint64_t getIssueTime() const {
		return issueTime;
	}

	void setIssueTime(const uint64_t now) {
		issueTime = now;
	}
protected:
	uint64_t reqID;
	uint64_t issueTime;
	std::vector<uint64_t> dependsOn;
};

}
}

#endif

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


// This type of event is used to transmit an arbitrary chunk of
// data between a CPU and an attached NIC. I guess it could be
// used anywhere a chunk of data and length is sufficient.
//
// The idea is to pack netsim API info into a data structure and
// send it off to the NIC which will know what to do with the
// data.

#ifndef SST_CORE_CPUNICEVENT_H
#define SST_CORE_CPUNICEVENT_H

#include <sst/core/serialization.h>
#include <sst/core/simulation.h>

#include <cstring>

#include <sst/core/event.h>

// We hardcode this here so we don't have to include netsim_internal.h
#define CPUNICEVENT_MAX_PARAMS		(64)

using namespace SST;

class CPUNicEvent : public Event {
    public:
        CPUNicEvent() : Event(), out(Simulation::getSimulation()->getSimulationOutput())   {
	    params_present= false;
	    params_len= 0;
	    routine= -1;
	    payload_len= 0;
	    payload_present= false;
	    hops= 0;
	    congestion_cnt= 0;
	    congestion_delay= 0;
	    entry_port= -1;
	    dest= -1;
	    local_traffic= false;
	    msg_id= 0;
	}

	// How to route this event through the network
	std::vector<int>route;
	std::vector<int>reverse_route; // Needed for bit_bucket
	SimTime_t router_delay;
	int hops;
	long long congestion_cnt;
	SimTime_t congestion_delay;

	// If this is data between cores sharing a cache, do not count
	// this as a router access.
	bool local_traffic;

	// The router model uses this to carry over input port info
	int entry_port;

	// Bit bucket uses this event on return sends.
	int return_event;

	// The destination rank for routing verification purposes
	int dest;
	uint64_t msg_id;// Each message event should have a unique ID for debugging

	// Some envelope info that can be used by the endpoints
	uint64_t msg_match_bits;
	uint64_t buf;
	uint32_t msg_len;
	uint32_t tag;

	// Functions to attach and detach parameters
	inline void AttachParams(const void *input, int len)   {
	    if (len > CPUNICEVENT_MAX_PARAMS)   {
		out.fatal(CALL_INFO, -1, "Only have room for %d bytes!!\n", CPUNICEVENT_MAX_PARAMS);
	    }
	    params_present= true;
	    params_len= len;
	    std::memcpy(event_params, input, len);
	}

	inline void DetachParams(void *output, int *len)   {
	    if (!params_present)   {
		out.fatal(CALL_INFO, -1, "No params present!\n");
	    }
	    if (*len > CPUNICEVENT_MAX_PARAMS)   {
		out.fatal(CALL_INFO, -1, "Can't detach %d bytes. Only have %d bytes (%d max) of params!!\n",
		    *len, params_len, CPUNICEVENT_MAX_PARAMS);
	    }
	    if ((int) params_len > *len)   {
		out.fatal(CALL_INFO, -1, "Have %d bytes of params, but user only wants %d!\n",
		    params_len, *len);
	    }

	    std::memcpy(output, event_params, params_len);
	    *len= params_len;
	}

	inline void SetRoutine(int r)   {
	    routine= r;
	}

	inline int GetRoutine(void)   {
	    return routine;
	}



	// Functions to attach and detach a message payload
	inline void AttachPayload(const char *payload, int len)   {
	    if (payload_present)   {
		out.fatal(CALL_INFO, -1, "Payload data already present!\n");
	    }
	    payload_present= true;
	    msg_payload.reserve(len);
	    payload_len= len;
	    msg_payload.insert(msg_payload.end(), payload, payload + len);
	}

	inline void DetachPayload(void *output, int *len)   {
	    if (!payload_present)   {
		out.fatal(CALL_INFO, -1, "No payload present!\n");
	    }
	    if (*len > payload_len)   {
		out.fatal(CALL_INFO, -1, "Have %d bytes of payload, but user wants %d!\n",
		    payload_len, *len);
	    }

	    int actual_len;
	    if (payload_len < *len)   {
		actual_len= payload_len;
	    } else   {
		actual_len= *len;
	    }
	    std::memcpy(output, &msg_payload[0], actual_len);
	    *len= actual_len;
	}

	inline int GetPayloadLen(void)   {
	    return payload_len;
	}


    private:
	bool params_present;
	int routine;
	unsigned int params_len;
	uint8_t event_params[CPUNICEVENT_MAX_PARAMS];
	std::vector<uint8_t>msg_payload;
	bool payload_present;
	int payload_len;
    Output &out;

        friend class boost::serialization::access;
        template<class Archive>
	void serialize(Archive &ar, const unsigned int version);

};


#endif // SST_CORE_CPUNICEVENT_H

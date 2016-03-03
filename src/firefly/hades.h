// Copyright 2013-2015 Sandia Corporation. Under the terms
// of Contract DE-AC04-94AL85000 with Sandia Corporation, the U.S.
// Government retains certain rights in this software.
//
// Copyright (c) 2013-2015, Sandia Corporation
// All rights reserved.
//
// This file is part of the SST software package. For license
// information, see the LICENSE file in the top level directory of the
// distribution.


#ifndef COMPONENTS_FIREFLY_HADES_H
#define COMPONENTS_FIREFLY_HADES_H

#include <sst/core/output.h>
#include <sst/core/params.h>

#include <sst/core/sharedRegion.h>

#include "sst/elements/hermes/hermes.h"
#include "group.h"
#include "info.h"
#include "protocolAPI.h"

namespace SST {
namespace Firefly {

class FunctionSM;
class VirtNic;

class Hades : public OS 
{
  public:
    Hades(Component*, Params&);
    ~Hades();
    virtual void printStatus( Output& );
    virtual void _componentInit(unsigned int phase );
    virtual void _componentSetup();
    void finish();

    int getNid();
    int getNumNids();

    int sizeofDataType( MP::PayloadDataType type ) { 
        return m_info.sizeofDataType(type); 
    }

    NodePerf* getNodePerf() {
        return m_nodePerf;
    }

  private:

    SST::Link*          m_enterLink;  
    VirtNic*            m_virtNic;
    Info                m_info;

  public:
    FunctionSM*         m_functionSM;
    Output              m_dbg;

  private:
    NodePerf*                            m_nodePerf;
    std::map<std::string,ProtocolAPI*>   m_protocolMapByName;
    std::map<int,ProtocolAPI*>           m_protocolM;
    SharedRegion*                        m_sreg;
    int                                  m_netMapSize;
    std::string                          m_netMapName;
};

} // namesapce Firefly 
} // namespace SST

#endif

// Copyright 2009-2016 Sandia Corporation. Under the terms
// of Contract DE-AC04-94AL85000 with Sandia Corporation, the U.S.
// Government retains certain rights in this software.
// 
// Copyright (c) 2009-2016, Sandia Corporation
// All rights reserved.
// 
// This file is part of the SST software package. For license
// information, see the LICENSE file in the top level directory of the
// distribution.

#include <sst_config.h>
#include "hsgraph.h"

#include <stdlib.h>

using namespace SST::Merlin;

const uint32_t topo_hsgraph::routing_table[50][50] = {
    {0, 6, 6, 5, 5, 0, 6, 0, 0, 5, 0, 4, 3, 2, 1, 1, 6, 1, 1, 5, 0, 2, 4, 1, 3, 2, 6, 2, 2, 5, 0, 3, 1, 4, 2, 3, 6, 3, 3, 5, 0, 1, 2, 3, 4, 4, 6, 4, 4, 5},
    {5, 0, 6, 6, 5, 5, 0, 6, 0, 0, 1, 0, 4, 3, 2, 5, 1, 6, 1, 1, 3, 0, 2, 4, 1, 5, 2, 6, 2, 2, 2, 0, 3, 1, 4, 5, 3, 6, 3, 3, 4, 0, 1, 2, 3, 5, 4, 6, 4, 4},
    {5, 5, 0, 6, 6, 0, 5, 0, 6, 0, 2, 1, 0, 4, 3, 1, 5, 1, 6, 1, 1, 3, 0, 2, 4, 2, 5, 2, 6, 2, 4, 2, 0, 3, 1, 3, 5, 3, 6, 3, 3, 4, 0, 1, 2, 4, 5, 4, 6, 4},
    {6, 5, 5, 0, 6, 0, 0, 5, 0, 6, 3, 2, 1, 0, 4, 1, 1, 5, 1, 6, 4, 1, 3, 0, 2, 2, 2, 5, 2, 6, 1, 4, 2, 0, 3, 3, 3, 5, 3, 6, 2, 3, 4, 0, 1, 4, 4, 5, 4, 6},
    {6, 6, 5, 5, 0, 6, 0, 0, 5, 0, 4, 3, 2, 1, 0, 6, 1, 1, 5, 1, 2, 4, 1, 3, 0, 6, 2, 2, 5, 2, 3, 1, 4, 2, 0, 6, 3, 3, 5, 3, 1, 2, 3, 4, 0, 6, 4, 4, 5, 4},
    {0, 0, 6, 5, 0, 0, 5, 6, 5, 6, 1, 1, 6, 5, 1, 0, 1, 2, 3, 4, 2, 2, 6, 5, 2, 0, 3, 1, 4, 2, 3, 3, 6, 5, 3, 0, 2, 4, 1, 3, 4, 4, 6, 5, 4, 0, 4, 3, 2, 1},
    {0, 0, 0, 6, 5, 6, 0, 5, 6, 5, 1, 1, 1, 6, 5, 4, 0, 1, 2, 3, 2, 2, 2, 6, 5, 2, 0, 3, 1, 4, 3, 3, 3, 6, 5, 3, 0, 2, 4, 1, 4, 4, 4, 6, 5, 1, 0, 4, 3, 2},
    {5, 0, 0, 0, 6, 5, 6, 0, 5, 6, 5, 1, 1, 1, 6, 3, 4, 0, 1, 2, 5, 2, 2, 2, 6, 4, 2, 0, 3, 1, 5, 3, 3, 3, 6, 1, 3, 0, 2, 4, 5, 4, 4, 4, 6, 2, 1, 0, 4, 3},
    {6, 5, 0, 0, 0, 6, 5, 6, 0, 5, 6, 5, 1, 1, 1, 2, 3, 4, 0, 1, 6, 5, 2, 2, 2, 1, 4, 2, 0, 3, 6, 5, 3, 3, 3, 4, 1, 3, 0, 2, 6, 5, 4, 4, 4, 3, 2, 1, 0, 4},
    {0, 6, 5, 0, 0, 5, 6, 5, 6, 0, 1, 6, 5, 1, 1, 1, 2, 3, 4, 0, 2, 6, 5, 2, 2, 3, 1, 4, 2, 0, 3, 6, 5, 3, 3, 2, 4, 1, 3, 0, 4, 6, 5, 4, 4, 4, 3, 2, 1, 0},
    {0, 1, 2, 3, 4, 0, 6, 0, 0, 5, 0, 6, 6, 5, 5, 5, 1, 6, 1, 1, 0, 4, 3, 2, 1, 2, 5, 2, 6, 2, 0, 2, 4, 1, 3, 3, 3, 5, 3, 6, 0, 3, 1, 4, 2, 6, 4, 4, 5, 4},
    {4, 0, 1, 2, 3, 5, 0, 6, 0, 0, 5, 0, 6, 6, 5, 1, 5, 1, 6, 1, 1, 0, 4, 3, 2, 2, 2, 5, 2, 6, 3, 0, 2, 4, 1, 6, 3, 3, 5, 3, 2, 0, 3, 1, 4, 4, 6, 4, 4, 5},
    {3, 4, 0, 1, 2, 0, 5, 0, 6, 0, 5, 5, 0, 6, 6, 1, 1, 5, 1, 6, 2, 1, 0, 4, 3, 6, 2, 2, 5, 2, 1, 3, 0, 2, 4, 3, 6, 3, 3, 5, 4, 2, 0, 3, 1, 5, 4, 6, 4, 4},
    {2, 3, 4, 0, 1, 0, 0, 5, 0, 6, 6, 5, 5, 0, 6, 6, 1, 1, 5, 1, 3, 2, 1, 0, 4, 2, 6, 2, 2, 5, 4, 1, 3, 0, 2, 5, 3, 6, 3, 3, 1, 4, 2, 0, 3, 4, 5, 4, 6, 4},
    {1, 2, 3, 4, 0, 6, 0, 0, 5, 0, 6, 6, 5, 5, 0, 1, 6, 1, 1, 5, 4, 3, 2, 1, 0, 5, 2, 6, 2, 2, 2, 4, 1, 3, 0, 3, 5, 3, 6, 3, 3, 1, 4, 2, 0, 4, 4, 5, 4, 6},
    {0, 0, 6, 5, 0, 0, 4, 3, 2, 1, 1, 6, 5, 1, 1, 0, 5, 6, 5, 6, 6, 5, 2, 2, 2, 0, 1, 2, 3, 4, 5, 3, 3, 3, 6, 0, 3, 1, 4, 2, 4, 4, 4, 6, 5, 0, 2, 4, 1, 3},
    {0, 0, 0, 6, 5, 1, 0, 4, 3, 2, 1, 1, 6, 5, 1, 6, 0, 5, 6, 5, 2, 6, 5, 2, 2, 4, 0, 1, 2, 3, 6, 5, 3, 3, 3, 2, 0, 3, 1, 4, 5, 4, 4, 4, 6, 3, 0, 2, 4, 1},
    {5, 0, 0, 0, 6, 2, 1, 0, 4, 3, 1, 1, 1, 6, 5, 5, 6, 0, 5, 6, 2, 2, 6, 5, 2, 3, 4, 0, 1, 2, 3, 6, 5, 3, 3, 4, 2, 0, 3, 1, 6, 5, 4, 4, 4, 1, 3, 0, 2, 4},
    {6, 5, 0, 0, 0, 3, 2, 1, 0, 4, 5, 1, 1, 1, 6, 6, 5, 6, 0, 5, 2, 2, 2, 6, 5, 2, 3, 4, 0, 1, 3, 3, 6, 5, 3, 1, 4, 2, 0, 3, 4, 6, 5, 4, 4, 4, 1, 3, 0, 2},
    {0, 6, 5, 0, 0, 4, 3, 2, 1, 0, 6, 5, 1, 1, 1, 5, 6, 5, 6, 0, 5, 2, 2, 2, 6, 1, 2, 3, 4, 0, 3, 3, 3, 6, 5, 3, 1, 4, 2, 0, 4, 4, 6, 5, 4, 2, 4, 1, 3, 0},
    {0, 3, 1, 4, 2, 0, 6, 0, 0, 5, 0, 1, 2, 3, 4, 1, 5, 1, 6, 1, 0, 6, 6, 5, 5, 6, 2, 2, 5, 2, 0, 4, 3, 2, 1, 5, 3, 6, 3, 3, 0, 2, 4, 1, 3, 4, 4, 5, 4, 6},
    {2, 0, 3, 1, 4, 5, 0, 6, 0, 0, 4, 0, 1, 2, 3, 1, 1, 5, 1, 6, 5, 0, 6, 6, 5, 2, 6, 2, 2, 5, 1, 0, 4, 3, 2, 3, 5, 3, 6, 3, 3, 0, 2, 4, 1, 6, 4, 4, 5, 4},
    {4, 2, 0, 3, 1, 0, 5, 0, 6, 0, 3, 4, 0, 1, 2, 6, 1, 1, 5, 1, 5, 5, 0, 6, 6, 5, 2, 6, 2, 2, 2, 1, 0, 4, 3, 3, 3, 5, 3, 6, 1, 3, 0, 2, 4, 4, 6, 4, 4, 5},
    {1, 4, 2, 0, 3, 0, 0, 5, 0, 6, 2, 3, 4, 0, 1, 1, 6, 1, 1, 5, 6, 5, 5, 0, 6, 2, 5, 2, 6, 2, 3, 2, 1, 0, 4, 6, 3, 3, 5, 3, 4, 1, 3, 0, 2, 5, 4, 6, 4, 4},
    {3, 1, 4, 2, 0, 6, 0, 0, 5, 0, 1, 2, 3, 4, 0, 5, 1, 6, 1, 1, 6, 6, 5, 5, 0, 2, 2, 5, 2, 6, 4, 3, 2, 1, 0, 3, 6, 3, 3, 5, 2, 4, 1, 3, 0, 4, 5, 4, 6, 4},
    {0, 0, 6, 5, 0, 0, 2, 4, 1, 3, 6, 5, 1, 1, 1, 0, 4, 3, 2, 1, 2, 2, 2, 6, 5, 0, 5, 6, 5, 6, 3, 6, 5, 3, 3, 0, 1, 2, 3, 4, 5, 4, 4, 4, 6, 0, 3, 1, 4, 2},
    {0, 0, 0, 6, 5, 3, 0, 2, 4, 1, 1, 6, 5, 1, 1, 1, 0, 4, 3, 2, 5, 2, 2, 2, 6, 6, 0, 5, 6, 5, 3, 3, 6, 5, 3, 4, 0, 1, 2, 3, 6, 5, 4, 4, 4, 2, 0, 3, 1, 4},
    {5, 0, 0, 0, 6, 1, 3, 0, 2, 4, 1, 1, 6, 5, 1, 2, 1, 0, 4, 3, 6, 5, 2, 2, 2, 5, 6, 0, 5, 6, 3, 3, 3, 6, 5, 3, 4, 0, 1, 2, 4, 6, 5, 4, 4, 4, 2, 0, 3, 1},
    {6, 5, 0, 0, 0, 4, 1, 3, 0, 2, 1, 1, 1, 6, 5, 3, 2, 1, 0, 4, 2, 6, 5, 2, 2, 6, 5, 6, 0, 5, 5, 3, 3, 3, 6, 2, 3, 4, 0, 1, 4, 4, 6, 5, 4, 1, 4, 2, 0, 3},
    {0, 6, 5, 0, 0, 2, 4, 1, 3, 0, 5, 1, 1, 1, 6, 4, 3, 2, 1, 0, 2, 2, 6, 5, 2, 5, 6, 5, 6, 0, 6, 5, 3, 3, 3, 1, 2, 3, 4, 0, 4, 4, 4, 6, 5, 3, 1, 4, 2, 0},
    {0, 2, 4, 1, 3, 0, 6, 0, 0, 5, 0, 3, 1, 4, 2, 1, 1, 5, 1, 6, 0, 1, 2, 3, 4, 5, 2, 6, 2, 2, 0, 6, 6, 5, 5, 6, 3, 3, 5, 3, 0, 4, 3, 2, 1, 4, 5, 4, 6, 4},
    {3, 0, 2, 4, 1, 5, 0, 6, 0, 0, 2, 0, 3, 1, 4, 6, 1, 1, 5, 1, 4, 0, 1, 2, 3, 2, 5, 2, 6, 2, 5, 0, 6, 6, 5, 3, 6, 3, 3, 5, 1, 0, 4, 3, 2, 4, 4, 5, 4, 6},
    {1, 3, 0, 2, 4, 0, 5, 0, 6, 0, 4, 2, 0, 3, 1, 1, 6, 1, 1, 5, 3, 4, 0, 1, 2, 2, 2, 5, 2, 6, 5, 5, 0, 6, 6, 5, 3, 6, 3, 3, 2, 1, 0, 4, 3, 6, 4, 4, 5, 4},
    {4, 1, 3, 0, 2, 0, 0, 5, 0, 6, 1, 4, 2, 0, 3, 5, 1, 6, 1, 1, 2, 3, 4, 0, 1, 6, 2, 2, 5, 2, 6, 5, 5, 0, 6, 3, 5, 3, 6, 3, 3, 2, 1, 0, 4, 4, 6, 4, 4, 5},
    {2, 4, 1, 3, 0, 6, 0, 0, 5, 0, 3, 1, 4, 2, 0, 1, 5, 1, 6, 1, 1, 2, 3, 4, 0, 2, 6, 2, 2, 5, 6, 6, 5, 5, 0, 3, 3, 5, 3, 6, 4, 3, 2, 1, 0, 5, 4, 6, 4, 4},
    {0, 0, 6, 5, 0, 0, 3, 1, 4, 2, 5, 1, 1, 1, 6, 0, 2, 4, 1, 3, 2, 6, 5, 2, 2, 0, 4, 3, 2, 1, 3, 3, 3, 6, 5, 0, 5, 6, 5, 6, 6, 5, 4, 4, 4, 0, 1, 2, 3, 4},
    {0, 0, 0, 6, 5, 2, 0, 3, 1, 4, 6, 5, 1, 1, 1, 3, 0, 2, 4, 1, 2, 2, 6, 5, 2, 1, 0, 4, 3, 2, 5, 3, 3, 3, 6, 6, 0, 5, 6, 5, 4, 6, 5, 4, 4, 4, 0, 1, 2, 3},
    {5, 0, 0, 0, 6, 4, 2, 0, 3, 1, 1, 6, 5, 1, 1, 1, 3, 0, 2, 4, 2, 2, 2, 6, 5, 2, 1, 0, 4, 3, 6, 5, 3, 3, 3, 5, 6, 0, 5, 6, 4, 4, 6, 5, 4, 3, 4, 0, 1, 2},
    {6, 5, 0, 0, 0, 1, 4, 2, 0, 3, 1, 1, 6, 5, 1, 4, 1, 3, 0, 2, 5, 2, 2, 2, 6, 3, 2, 1, 0, 4, 3, 6, 5, 3, 3, 6, 5, 6, 0, 5, 4, 4, 4, 6, 5, 2, 3, 4, 0, 1},
    {0, 6, 5, 0, 0, 3, 1, 4, 2, 0, 1, 1, 1, 6, 5, 2, 4, 1, 3, 0, 6, 5, 2, 2, 2, 4, 3, 2, 1, 0, 3, 3, 6, 5, 3, 5, 6, 5, 6, 0, 5, 4, 4, 4, 6, 1, 2, 3, 4, 0},
    {0, 4, 3, 2, 1, 0, 6, 0, 0, 5, 0, 2, 4, 1, 3, 6, 1, 1, 5, 1, 0, 3, 1, 4, 2, 2, 2, 5, 2, 6, 0, 1, 2, 3, 4, 3, 5, 3, 6, 3, 0, 6, 6, 5, 5, 5, 4, 6, 4, 4},
    {1, 0, 4, 3, 2, 5, 0, 6, 0, 0, 3, 0, 2, 4, 1, 1, 6, 1, 1, 5, 2, 0, 3, 1, 4, 6, 2, 2, 5, 2, 4, 0, 1, 2, 3, 3, 3, 5, 3, 6, 5, 0, 6, 6, 5, 4, 5, 4, 6, 4},
    {2, 1, 0, 4, 3, 0, 5, 0, 6, 0, 1, 3, 0, 2, 4, 5, 1, 6, 1, 1, 4, 2, 0, 3, 1, 2, 6, 2, 2, 5, 3, 4, 0, 1, 2, 6, 3, 3, 5, 3, 5, 5, 0, 6, 6, 4, 4, 5, 4, 6},
    {3, 2, 1, 0, 4, 0, 0, 5, 0, 6, 4, 1, 3, 0, 2, 1, 5, 1, 6, 1, 1, 4, 2, 0, 3, 5, 2, 6, 2, 2, 2, 3, 4, 0, 1, 3, 6, 3, 3, 5, 6, 5, 5, 0, 6, 6, 4, 4, 5, 4},
    {4, 3, 2, 1, 0, 6, 0, 0, 5, 0, 2, 4, 1, 3, 0, 1, 1, 5, 1, 6, 3, 1, 4, 2, 0, 2, 5, 2, 6, 2, 1, 2, 3, 4, 0, 5, 3, 6, 3, 3, 6, 6, 5, 5, 0, 4, 6, 4, 4, 5},
    {0, 0, 6, 5, 0, 0, 1, 2, 3, 4, 1, 1, 1, 6, 5, 0, 3, 1, 4, 2, 5, 2, 2, 2, 6, 0, 2, 4, 1, 3, 6, 5, 3, 3, 3, 0, 4, 3, 2, 1, 4, 6, 5, 4, 4, 0, 5, 6, 5, 6},
    {0, 0, 0, 6, 5, 4, 0, 1, 2, 3, 5, 1, 1, 1, 6, 2, 0, 3, 1, 4, 6, 5, 2, 2, 2, 3, 0, 2, 4, 1, 3, 6, 5, 3, 3, 1, 0, 4, 3, 2, 4, 4, 6, 5, 4, 6, 0, 5, 6, 5},
    {5, 0, 0, 0, 6, 3, 4, 0, 1, 2, 6, 5, 1, 1, 1, 4, 2, 0, 3, 1, 2, 6, 5, 2, 2, 1, 3, 0, 2, 4, 3, 3, 6, 5, 3, 2, 1, 0, 4, 3, 4, 4, 4, 6, 5, 5, 6, 0, 5, 6},
    {6, 5, 0, 0, 0, 2, 3, 4, 0, 1, 1, 6, 5, 1, 1, 1, 4, 2, 0, 3, 2, 2, 6, 5, 2, 4, 1, 3, 0, 2, 3, 3, 3, 6, 5, 3, 2, 1, 0, 4, 5, 4, 4, 4, 6, 6, 5, 6, 0, 5},
    {0, 6, 5, 0, 0, 1, 2, 3, 4, 0, 1, 1, 6, 5, 1, 3, 1, 4, 2, 0, 2, 2, 2, 6, 5, 2, 4, 1, 3, 0, 5, 3, 3, 3, 6, 4, 3, 2, 1, 0, 6, 5, 4, 4, 4, 5, 6, 5, 6, 0},
};

const uint32_t topo_hsgraph::neighbor_table[50][7] = {
    {5, 15, 25, 35, 45, 4, 1},
    {6, 16, 26, 36, 46, 0, 2},
    {7, 17, 27, 37, 47, 1, 3},
    {8, 18, 28, 38, 48, 2, 4},
    {9, 19, 29, 39, 49, 3, 0},
    {0, 10, 20, 30, 40, 8, 7},
    {1, 11, 21, 31, 41, 9, 8},
    {2, 12, 22, 32, 42, 5, 9},
    {3, 13, 23, 33, 43, 6, 5},
    {4, 14, 24, 34, 44, 7, 6},
    {5, 16, 27, 38, 49, 14, 11},
    {6, 17, 28, 39, 45, 10, 12},
    {7, 18, 29, 35, 46, 11, 13},
    {8, 19, 25, 36, 47, 12, 14},
    {9, 15, 26, 37, 48, 13, 10},
    {0, 14, 23, 32, 41, 18, 17},
    {1, 10, 24, 33, 42, 19, 18},
    {2, 11, 20, 34, 43, 15, 19},
    {3, 12, 21, 30, 44, 16, 15},
    {4, 13, 22, 31, 40, 17, 16},
    {5, 17, 29, 36, 48, 24, 21},
    {6, 18, 25, 37, 49, 20, 22},
    {7, 19, 26, 38, 45, 21, 23},
    {8, 15, 27, 39, 46, 22, 24},
    {9, 16, 28, 35, 47, 23, 20},
    {0, 13, 21, 34, 42, 28, 27},
    {1, 14, 22, 30, 43, 29, 28},
    {2, 10, 23, 31, 44, 25, 29},
    {3, 11, 24, 32, 40, 26, 25},
    {4, 12, 20, 33, 41, 27, 26},
    {5, 18, 26, 39, 47, 34, 31},
    {6, 19, 27, 35, 48, 30, 32},
    {7, 15, 28, 36, 49, 31, 33},
    {8, 16, 29, 37, 45, 32, 34},
    {9, 17, 25, 38, 46, 33, 30},
    {0, 12, 24, 31, 43, 38, 37},
    {1, 13, 20, 32, 44, 39, 38},
    {2, 14, 21, 33, 40, 35, 39},
    {3, 10, 22, 34, 41, 36, 35},
    {4, 11, 23, 30, 42, 37, 36},
    {5, 19, 28, 37, 46, 44, 41},
    {6, 15, 29, 38, 47, 40, 42},
    {7, 16, 25, 39, 48, 41, 43},
    {8, 17, 26, 35, 49, 42, 44},
    {9, 18, 27, 36, 45, 43, 40},
    {0, 11, 22, 33, 44, 48, 47},
    {1, 12, 23, 34, 40, 49, 48},
    {2, 13, 24, 30, 41, 45, 49},
    {3, 14, 20, 31, 42, 46, 45},
    {4, 10, 21, 32, 43, 47, 46},
};

topo_hsgraph::topo_hsgraph(Component* comp, Params& params) :
    Topology(comp)
{
    router_id = params.find<int>("id", -1);
    if (router_id == -1) {
        output.fatal(CALL_INFO, -1, "id must be set\n");
    }
    
    host_ports = (uint32_t)params.find<int>("hsgraph:hosts_per_router", 1);
    outgoing_ports = (uint32_t)params.find<int>("hsgraph:outgoing_ports", 0);
    local_ports = 7;
    routers_per_subnet = 50;
    uint32_t num_ports = (uint32_t)params.find<int>("num_ports");
    uint32_t needed_ports = host_ports + local_ports + outgoing_ports;
    if (needed_ports > num_ports) {
        output.fatal(CALL_INFO, -1, "Need more ports to support the given topology!\n");
    }
    
    std::string route_algo = params.find<std::string>("hsgraph:algorithm", "minimal");
    if (!route_algo.compare("minimal")) {
        // TODO implement other algorithms later
        algorithm = MINIMAL;
    }
    
    std::string interconnect = params.find<std::string>("hsgraph:interconnect", "none");
    
    if (interconnect.compare("fishlite") == 0 ) {  // returns 0 when equal.. wtf
        subnet = (uint32_t)params.find<int>("hsgraph:subnet", 0);
        net_type = FISH_LITE;
    } else if (interconnect.compare("fishnet") == 0) {
        subnet = (uint32_t)params.find<int>("hsgraph:subnet", 0);
        net_type = FISHNET;
    } else {
        // just a hsgraph
        subnet = 0; 
        net_type = NONFISH;
    }
    router = (uint32_t)params.find<int>("hsgraph:router", 0);
}


topo_hsgraph::~topo_hsgraph()
{
}


void topo_hsgraph::route(int port, int vc, internal_router_event* ev)
{
    topo_hsgraph_event *tp_ev = static_cast<topo_hsgraph_event*>(ev);

    if ( (uint32_t)port >= (host_ports + local_ports) ) {
        /* Came in from another subnet.  Increment VC */
        tp_ev->setVC(vc+1);
    }
    
    uint32_t next_port = 0;
    if (tp_ev->dest.subnet != subnet) {
        // target is not this subnet
        // only implement angelfish_lite for now..
        if (net_type == NONFISH) {
            output.fatal(CALL_INFO, -1, "How could you get here? \n");
        } else {
            uint32_t mid_rtr = 0;  // the router responsible for forwarding packet
            if (tp_ev->dest.subnet < subnet) {
                mid_rtr = tp_ev->dest.subnet;
            } else {
                mid_rtr = tp_ev->dest.subnet - 1; // minus 1 because the way it connects
            }
            if (net_type == FISH_LITE) {
                if (mid_rtr == router) {  // going to other subnets 
                    next_port = host_ports + local_ports;
                } else {  // forward
                    next_port = port_for_router(mid_rtr);
                }
            } else {  // fishnet
                if (mid_rtr == router) {  
                    // different from fishlite, the mid_rtr neighbors have access to
                    // the target subnet, so forward to one of its neighbors
                    // could be from host_ports to (host_ports + local_ports -1)
                    next_port = host_ports;
                } else {
                    if (is_neighbor(router, mid_rtr)) { // if router in mid_rtr 's neighbor
                        bool find_rtr = false;
                        for (uint32_t i = 0; i < outgoing_ports; i++) {
                            uint32_t neighbor = neighbor_table[router][i];
                            if (neighbor >= subnet) {
                                neighbor += 1;
                            }
                            if (neighbor == tp_ev->dest.subnet) {  // going to other subnet
                                next_port = host_ports + local_ports + i;
                                find_rtr = true;
                                break;
                            }
                        }
                        if (!find_rtr) {
                            output.fatal(CALL_INFO, -1, "cannot find route!\n");
                        }
                    } else { // forward to mid_rtr
                        next_port = port_for_router(mid_rtr);
                    }
                }
            }
        }       
    } else if ( tp_ev->dest.router != router) {
        // not this router, forward to other routers in this subnet
        // trivial routing
        next_port = port_for_router(tp_ev->dest.router);
    } else {
        // this router
        next_port = tp_ev->dest.host;
    }
    output.verbose(CALL_INFO, 1, 1, "%u:%u, Recv: %d/%d  Setting Next Port/VC:  %u/%u\n", 
                    subnet, router, port, vc, next_port, tp_ev->getVC());
    tp_ev->setNextPort(next_port);
}


internal_router_event* topo_hsgraph::process_input(RtrEvent* ev)
{
    topo_hsgraph::fishnetAddr destAddr = {0, 0, 0};
    id_to_location(ev->request->dest, &destAddr);
    topo_hsgraph_event *tp_ev = new topo_hsgraph_event(destAddr);
    // if to implement other algorithm, need to add stuff..
    
    tp_ev->src_subnet = subnet;
    tp_ev->setEncapsulatedEvent(ev);
    // TODO not sure about this vn thing
    tp_ev->setVC(ev->request->vn *3);
    return tp_ev;
}

// this is to figure out what are the possible outports for a given input port
void topo_hsgraph::routeInitData(int port, internal_router_event* ev, std::vector<int> &outPorts)
{
    topo_hsgraph_event *tp_ev = static_cast<topo_hsgraph_event*>(ev);
    if ( tp_ev->dest.host == (uint32_t)INIT_BROADCAST_ADDR ) {
        uint32_t total_ports = host_ports + local_ports + outgoing_ports;
        if ( (uint32_t)port >= (host_ports + local_ports ) ) {
            /* Came in from another subnet.
             * Send to locals, and other routers in subnet
             */
            tp_ev->is_forwarded = false;
            for ( uint32_t p = 0; p < (host_ports + local_ports ); p++ ) {
                outPorts.push_back((int)p);
            }
        } else if ( (uint32_t)port >= host_ports ) {
            /* Came in from another router in subnet.
             * send to hosts
             * if this is the source subnet, 
             * forward to the other router, or send to other subnets
             */
            for ( uint32_t p = 0; p < host_ports; p++ ) {
                outPorts.push_back((int)p);
            }
            /* Note this should be different from dragonfly
             * because you can forward 
             */
            if (tp_ev->src_subnet == subnet) {
                for ( uint32_t p = (host_ports + local_ports); p < total_ports; p++) {
                    outPorts.push_back((int)p);
                }
            }
            if (tp_ev->is_forwarded == false) {
                tp_ev->is_forwarded = true; 
                for ( uint32_t p = host_ports; p < (host_ports + local_ports); p++) {
                    outPorts.push_back((int)p);
                }
            }
        } else {
            /* Came in from a host
             * Send to all other hosts and routers in group, and all groups
             */
             for (uint32_t p = 0; p < total_ports; p++) {
                 if (p != (uint32_t)port) {
                     outPorts.push_back((int)p);
                 }
             }
        }
    } else {
        route(port, 0, ev);
        outPorts.push_back(ev->getNextPort());
    }
}


internal_router_event* topo_hsgraph::process_InitData_input(RtrEvent* ev)
{
    topo_hsgraph::fishnetAddr destAddr;
    id_to_location(ev->request->dest, &destAddr);
    topo_hsgraph_event *tp_ev = new topo_hsgraph_event(destAddr);
    tp_ev->src_subnet = subnet;
    tp_ev->is_forwarded = false;
    tp_ev->setEncapsulatedEvent(ev);
    return tp_ev;
}

Topology::PortState topo_hsgraph::getPortState(int port) const
{
    if ((uint32_t)port < host_ports) return R2N;
    else return R2R;
}

std::string topo_hsgraph::getPortLogicalGroup(int port) const
{
    if ((uint32_t)port < host_ports) return "host";
    if ((uint32_t)port >= host_ports && (uint32_t)port < (host_ports + local_ports)) return "group";
    else return "global";
}

int topo_hsgraph::getEndpointID(int port)
{
    return (subnet* (routers_per_subnet * host_ports)) + router*host_ports + port;
}

void topo_hsgraph::id_to_location(int id, fishnetAddr *location) const
{
    if (id == INIT_BROADCAST_ADDR) {
        location->subnet = (uint32_t)INIT_BROADCAST_ADDR;
        location->router = (uint32_t)INIT_BROADCAST_ADDR;
        location->host = (uint32_t)INIT_BROADCAST_ADDR;
    } else {
        uint32_t hosts_per_subnet = host_ports * routers_per_subnet;
        location->subnet = id / hosts_per_subnet;
        location->router = (id % hosts_per_subnet) / host_ports;
        location->host = id % host_ports;
    }
}


uint32_t topo_hsgraph::port_for_router(uint32_t dest_router) const 
{
    // just looking up routing table
    return host_ports + routing_table[router][dest_router];
}


// see if the target router is a neighbor of this router
bool topo_hsgraph::is_neighbor(uint32_t tgt_rtr, uint32_t this_rtr) const
{
    for (uint32_t i = 0; i < local_ports; i++) {
        if (tgt_rtr == neighbor_table[this_rtr][i]) {
            return true;
        }
    }
    return false;
}


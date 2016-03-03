#!/bin/bash


sst --model-options="--topo=dragonfly --shape=7:2:2:4 --numCores=2 --netFlitSize=8B --netPktSize=1024B --netBW=1GB/s --embermotifLog=./motif --loadFile=loadfile " emberLoad.py

#sst --model-options="--topo=torus --shape=3x4x6 --numCores=2 --netFlitSize=8B --netPktSize=1024B --netBW=1GB/s --embermotifLog=./motif --loadFile=loadfile " emberLoad.py

rm core.*


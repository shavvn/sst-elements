#! /usr/bin/env python

import sys,os
from subprocess import call
import CrossProduct
from CrossProduct import *
import hashlib
import binascii

config = "emberLoad.py"

tests = []
networks = [] 


net = { 'topo' : 'torus',
        'args' : [ 
                    [ '--shape', ['41x41', '44x44', '52x52', '75x75', 
                                  '12x12x12', '14x14x14', '18x18x18'] ] 
                 ]
      }

networks.append(net)


net = { 'topo' : 'fattree',
        'args' : [  
                    ['--shape',   ['8,8:16,16:16', '8,8:16,16:20', '16,16:16,16:20', 
                                   '4,4:4,4:8,8:16', '4,4:8,8:8,8:16', '4,4:8,8:8,8:20']],
                 ]
      }

networks.append(net)

net = { 'topo' : 'diameter2',
        'args' : [
                    [ '--shape', ['43:1', '47:1', '55:1', '79:1'] ]
                 ]
      }
      
networks.append(net)


net = { 'topo' : 'dragonfly',
        'args' : [
                    [ '--shape', ['19:5:5:10', '20:6:5:10', '23:6:6:12'] ]
                 ]
      }
      
networks.append(net)


test = { 'motif' : 'AllPingPong',
         'args'  : [ 
                        [ 'iterations'  , ['1','10']],
                        [ 'messageSize' , ['0','1','10000','20000']] 
                   ] 
        }

tests.append( test )

test = { 'motif' : 'Allreduce',
         'args'  : [  
                        [ 'iterations'  , ['1','10']],
                        [ 'count' , ['1']] 
                   ] 
        }

tests.append( test )

test = { 'motif' : 'Barrier',
         'args'  : [  
                        [ 'iterations'  , ['1','10']]
                   ] 
        }

tests.append( test )

test = { 'motif' : 'PingPong',
         'args'  : [  
                        [ 'iterations'  , ['1','10']],
                        [ 'messageSize' , ['0','1','10000','20000']] 
                   ] 
        }

tests.append( test )

test = { 'motif' : 'Reduce',
         'args'  : [  
                        [ 'iterations'  , ['1','10']],
                        [ 'count' , ['1']] 
                   ] 
        }

tests.append( test )

test = { 'motif' : 'Ring',
         'args'  : [  
                        [ 'iterations'  , ['1','10']],
                        [ 'messagesize' , ['0','1','10000','20000']] 
                   ] 
        }

tests.append( test )

output_file = sys.argv[1]
f = open(output_file, "wb")


for network in networks :
    for test in tests :
        for x in CrossProduct( network['args'] ) :
            for y in CrossProduct( test['args'] ):
                call("mpirun -n 12 sst --model-options=\"--topo={0} {1} --cmdLine=\\\"Init\\\" --cmdLine=\\\"{2} {3}\\\"--cmdLine=\\\"Fini\\\"\" {4}".format(network['topo'], x, test['motif'], y, config), shell=True, stdout=f, stderr=f)
f.close()

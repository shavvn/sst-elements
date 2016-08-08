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
                    [ '--shape', ['100x100', '242x242', '338x338', 
                                  '22x22x22', '38x38x38', '47x47x47'] ] 
                 ]
      }

networks.append(net)


net = { 'topo' : 'fattree',
        'args' : [  
                    ['--shape',   ['16,16:16,16:40', '16,16:32,32:40', '32,32:48,48:64'
                                   '8,8:8,8:8,8:20', '8,8:8,8:16,16:20', '8,8:16,16:16,16:32', '16,16:16,16:16,16:24']],
                 ]
      }

networks.append(net)


net = { 'topo' : 'dragonfly',
        'args' : [
                    [ '--shape', ['27:7:7:14', '39:10:10:20', 
                                  '43:11:11:22', '47:12:12:24', '50:13:13:26'] ]
                 ]
      }
      
networks.append(net)


net = { 'topo' : 'fishlite',
        'args' : [
                    [ '--shape', ['7:1', '11:1', '17:1', '19:1'] ]  # 
                 ]
      }
      
networks.append(net)


net = { 'topo' : 'fishnet',
        'args' : [
                    [ '--shape', ['7:1', '11:1', '17:1', '19:1'] ]  # 
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

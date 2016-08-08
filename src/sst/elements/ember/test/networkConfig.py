import sys

class TopoInfo:
	def getNumNodes(self):
		pass
	def getNetworkParams(self):
		pass

class TorusInfo(TopoInfo):
	def __init__( self, config ):

		args = config.split(':')
		shape = args[0]
		width = 1
		local_ports = 1

		if len( args ) > 1:
			local_ports = int( args[1] )

		if len( args ) > 2:
			width = int( args[2] )
		
		self.params = {}
		self.params["num_dims"] = self.calcNumDim(shape)
		self.params["torus:shape"] = shape
		self.params["torus:width"] = self.calcWidth(shape,width)
		self.params["torus:local_ports"] = local_ports 
		self.numNodes = self.calcNumNodes( shape ) * local_ports

	def getNetworkParams(self):
		return self.params

	def getNumNodes(self):
		return self.numNodes

	def calcNumDim(self,shape):
		return len( shape.split( 'x' ) )

	def calcNumNodes(self,shape):
		tmp = shape.split( 'x' )
		num = 1
		for d in tmp:
			num = num * int(d)
		return num

	def calcWidth(self,shape,width):
		tmp = len( shape.split( 'x' ) ) - 1
		retval = str(width) 
		count = 0
		while ( count < tmp ):
			retval += "x" + str(width)
			count  += 1
		return retval


class FattreeInfo(TopoInfo):
	def __init__( self, shape ):
		self.params = {}
		self.numNodes = self.calcNumNodes(shape)
		self.params["fattree:shape"] = shape
                
	def getNetworkParams(self):
		return self.params

	def getNumNodes(self):
		return self.numNodes 

        def calcNumNodes(self, shape):
                levels = shape.split(":")

                total_hosts = 1;
                for l in levels:
                        links = l.split(",")
                        total_hosts = total_hosts * int(links[0])

                return total_hosts

class DragonFlyInfo(TopoInfo):
	def __init__( self, shape ):
		radix, lcl, glbl, nRtrs = shape.split(':')
		self.params = {}
		hostsPerGroup = int(nRtrs) * int(lcl)
		nGrp = int(nRtrs) * int(glbl) + 1
		self.params["router_radix"] = radix
		self.params["dragonfly:shape"] = "" 
		self.params["dragonfly:hosts_per_router"] = lcl
		self.params["dragonfly:routers_per_group"] = nRtrs
		self.params["dragonfly:intergroup_per_router"] = glbl
		self.params["dragonfly:num_groups"] =  nGrp
		self.params["dragonfly:algorithm"] =  "minimal" 

		self.numNodes = nGrp * hostsPerGroup 
                
	def getNetworkParams(self):
		return self.params

	def getNumNodes(self):
		return self.numNodes 

class DragonFly2Info(TopoInfo):
	def __init__( self, shape ):
		lcl, nRtrs, glbl, nGrp = shape.split(':')
		self.params = {}
		hostsPerGroup = int(nRtrs) * int(lcl)
		self.params["dragonfly:shape"] = "" 
		self.params["dragonfly:hosts_per_router"] = lcl
		self.params["dragonfly:routers_per_group"] = nRtrs
		self.params["dragonfly:intergroup_links"] = glbl
		self.params["dragonfly:num_groups"] =  nGrp
		self.params["dragonfly:algorithm"] =  "minimal" 

                print lcl
                print nRtrs
                print glbl
                print nGrp
                
		self.numNodes = int(nGrp) * hostsPerGroup
                print self.numNodes
                
	def getNetworkParams(self):
		return self.params

	def getNumNodes(self):
		return self.numNodes 
        
        
class Diameter2Info(TopoInfo):
	def __init__(self, config):
		self.params = {}
		radix, hosts = [int(p) for p in config.split(':')]
		self.file_name = "netlists/MMS.%d.adj.txt" % radix
		self.hosts_per_router = hosts 
		self.routers_per_net = 0
		self.local_ports = 0 
		self._parse_adj_file(self.file_name)
		self.numNodes = self.hosts_per_router * self.routers_per_net
		self.params.update({
			"topology": "merlin.diameter2",
			"num_vns": 1,
			"diameter2:file": self.file_name,
			"diameter2:hosts_per_router": self.hosts_per_router,
			"diameter2:interconnect": "none",
			"diameter2:algorithm": "minimal"
		})

	def _parse_adj_file(self, file_name):
		with open(file_name, "r") as fp:
			first_line = next(fp)
			num_nodes, num_links = first_line.rstrip().split(" ")
			num_nodes = int(num_nodes)
			num_links = int(num_links)
			self.routers_per_net = num_nodes
			self.local_ports = num_links * 2 / num_nodes
			fp.close()

	def getNetworkParams(self):
		return self.params

	def getNumNodes(self):
		return self.numNodes

        
class FishnetInfo(Diameter2Info):
    def __init__(self, config):
        Diameter2Info.__init__(self, config)
        del self.params
        self.params = {
            "topology": "merlin.diameter2",
            "num_vns": 1,
            "fishnet:file": self.file_name,
            "fishnet:hosts_per_router": self.hosts_per_router,
            "diameter2:algorithm": "minimal"
        }
        self.numNodes *= (self.numNodes + 1)
        print self.numNodes
        print self.params

        
class FishliteInfo(Diameter2Info):
    def __init__(self, config):
        Diameter2Info.__init__(self, config)
        del self.params
        self.params = {
            "topology": "merlin.diameter2",
            "num_vns": 1,
            "fishlite:file": self.file_name,
            "fishlite:hosts_per_router": self.hosts_per_router,
            "diameter2:algorithm": "minimal"
        }
        self.numNodes *= (self.numNodes + 1)   
        

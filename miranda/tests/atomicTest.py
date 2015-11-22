import sst
import sys,getopt


# Tell SST what statistics handling we want
sst.setStatisticLoadLevel(0)

nQuads = 1
nCPU = nQuads*4
nIncr = 128
max_address = 64
coherence_protocol = "MESI"
busLat = "50ps"
netLat = "6ns"
coreNetBW = "36GB/s"
memNetBW = "36GB/s"
xbarBW = coreNetBW
flit_size = "72B"
memDebug = 7
clock = "2.4GHz"
use_atomics = "yes"
do_fence = "no"
do_verify = "yes"


router = sst.Component("router", "merlin.hr_router")
router.addParams({"topology": "merlin.singlerouter",
                "link_bw": coreNetBW,
                "xbar_bw": xbarBW,
                "input_latency": "6ns",
                "output_latency": "6ns",
                "input_buf_size" : "4KiB",
                "output_buf_size" : "4KiB",
                "flit_size" : flit_size,
                "id": 0
                })
rtrPortCount = 0


def doQuad(num):
    sst.pushNamePrefix("q%d"%num)

    bus = sst.Component("membus", "memHierarchy.Bus")
    bus.addParams({
        "bus_frequency" : clock,
        "bus_latency_cycles" : 1,
        })

    l2 = sst.Component("l2cache_q%d"%num, "memHierarchy.Cache")
    l2.addParams({
        "coherence_protocol": coherence_protocol,
        "cache_frequency": "1GHz",
        "replacement_policy": "lru",
        "cache_size": "128KB",
        "associativity": 16,
        "cache_line_size": 64,
        "access_latency_cycles": 23,
        "low_network_links": 1,
        "high_network_links": 1,
        "mshr_num_entries" : 4096,
        "L1": 0,
        "directory_at_next_level": 1,
        "network_address": num,
        "network_bw": coreNetBW,
        "statistics": 1,
        "debug_level" : 6,
        "debug": memDebug
        })
    link = sst.Link("l2cache_%d_netlink"%num)
    link.connect((l2, "directory", netLat), (router, "port%d"%num, netLat))
    link = sst.Link("l2cache_%d_link"%num)
    link.connect((l2, "high_network_0", busLat), (bus, "low_network_0", busLat))

    for cpu in range(4):
        sst.pushNamePrefix("c%d"%cpu)

        # Define the simulation components
        comp_cpu = sst.Component("cpu", "miranda.BaseCPU")
        comp_cpu.addParams({
            "verbose" : 0,
            "generator" : "miranda.GUPSGenerator",
            "clock" : clock,
            "generatorParams.verbose" : 0,
            "generatorParams.count" : nIncr,
            "generatorParams.max_address" : max_address,
            "generatorParams.use_atomics" : use_atomics,
            "generatorParams.issue_op_fances" : do_fence,
            "generatorParams.verify" : do_verify,
            "printStats" : 1,
        })
    
        # Enable statistics outputs
        comp_cpu.enableAllStatistics({"type":"sst.AccumulatorStatistic"})
        
        comp_l1cache = sst.Component("l1cache", "memHierarchy.Cache")
        comp_l1cache.addParams({
            "access_latency_cycles" : 2,
            "cache_frequency" : clock,
            "replacement_policy" : "lru",
            "coherence_protocol" : coherence_protocol,
            "associativity" : 4,
            "cache_line_size" : 64,
            "prefetcher" : "cassini.StridePrefetcher",
            "debug": memDebug,
            "low_network_links" : 1,
            "statistics" : 1,
            "L1" : 1,
            "cache_size" : "32KB"
        })
        link_cpu_cache_link = sst.Link("link_cpu_cache_link%d"%cpu)
        link_cpu_cache_link.connect( (comp_cpu, "cache_link", "1000ps"), (comp_l1cache, "high_network_0", "1000ps") )
        link_mem_bus_link = sst.Link("link_mem_bus_link%d"%cpu)
        link_mem_bus_link.connect( (comp_l1cache, "low_network_0", "50ps"), (bus, "high_network_%d"%cpu, "50ps") )
    
        # Enable statistics outputs
        comp_l1cache.enableAllStatistics({"type":"sst.AccumulatorStatistic"})
        sst.popNamePrefix()
    
    sst.popNamePrefix()


# Build quads
for q in range(nQuads):
    doQuad(q)


comp_memory = sst.Component("memory", "memHierarchy.MemController")
comp_memory.addParams({
      "coherence_protocol" : coherence_protocol,
      "backend.access_time" : "100 ns",
      "backend.mem_size" : "4096",
      "clock" : "500MHz"
})

dc = sst.Component("dc_nid%d"%nQuads, "memHierarchy.DirectoryController")
dc.addParams({
            "coherence_protocol": coherence_protocol,
            "network_bw": memNetBW,
            "addr_range_start": 0,
            "addr_range_end":  4096*1024*1024,
            "entry_cache_size": 128*1024, #Entry cache size of mem/blocksize
            "clock": "1GHz",
            "debug": memDebug,
            "statistics": 1,
            "network_address": nQuads
            })

memLink = sst.Link("dc_memLink")
memLink.connect((comp_memory, "direct_link", busLat), (dc, "memory", busLat))
netLink = sst.Link("dc_netlink")
netLink.connect((dc, "network", netLat), (router, "port%d"%nQuads, netLat))
router.addParam("num_ports", nQuads+1)

#include <loadMemory.h>

#include <mem/physical.hh>
#include <dummySystem.h>

#include <debug.h>
#include <process.h>

void loadMemory( string name, PhysicalMemory* memory, 
                                    const SST::Params& params )
{
    int num = 0;
    while ( 1 ) {
        Addr start,end;
        std::stringstream tmp;

        tmp << num; 

        std::string exe = 
                    params.find_string( tmp.str() + ".process.executable");
        if ( exe.size() == 0 ) {
            break;
        }

        DBGC(1, "%s.%s `%s`\n", name.c_str(), tmp.str().c_str(), exe.c_str() );

        start = params.find_integer( tmp.str() + ".physicalMemory.start" );
        end = params.find_integer( tmp.str() + ".physicalMemory.end", 0 );

        DBGC(1,"%s.%s.physicalMemory.start %#lx\n",name.c_str(),
                                tmp.str().c_str(),start);
        DBGC(1,"%s.%s.physicalMemory.end %#lx\n",name.c_str(),
                                tmp.str().c_str(),end);

        DummySystem* system = create_DummySystem( name + "." + tmp.str() +  
                            ".dummySystem", memory, Enums::timing, start, end);

        Process* process = newProcess( name + "." + tmp.str(), 
            params.find_prefix_params( tmp.str() + ".process."),
                                       system );
        process->assignThreadContext(num);

        // NOTE: system and process are not needed after
        // startup, how do we free them? 

        ++num;
    }
}

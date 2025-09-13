#include "cache.h"
#include <random>


uint32_t g_cfg_cache_size = 1024 * 1024; // 1MB
uint32_t g_cfg_line_size = 64;            // 64B
uint32_t g_cfg_num_ways = 4;              // 4-way set associative
uint32_t g_cfg_replacement_policy = 0; // 0: LRU, 1: FIFO
bool g_cfg_write_allocate = true; // true: Write Allocate, false: No Write Allocate
string g_cfg_trace_filename = "";
string g_cfg_stats_filename = "cache_stats.txt";



void printUsage()
{
        cout << "Usage: CacheSimulator.exe [options]\n";
        cout << "Options:\n";
        cout << "  -h                           Show this help message\n";
        cout << "  -trace <trace_file>          Specify trace input file\n";
        cout << "  -stat <stats_file>           Specify stats output file\n";
        cout << "  -cs <cache_size_KB>          Specify the cache size in KB. eg. -s 2 creates 2KB cache\n";
        cout << "  -ls <cache_line_size_B>      Specify cache line size in bytes (default 0: 64B), else: Absolute fixed size\n";
        cout << "  -w <num_of_ways>             Specify the associativity of cache. (default: Fully Associative. 0: Direct Mapped, else: specified sets).\n";
        cout << "  -rp <replacement_policy>     Specify the cache replacement policy. (default: 0: LRU, 1: FIFO)\n";
        cout << "  -wap <write_allocate_policy> Specify the write allocate policy. (default: 1: Write Allocate, 0: No Write Allocate)\n";
        cout << "Example: CacheSimulator.exe -cs 256 -ls 64 -w 4 -rp 0 -wap 0 -trace trace.txt -stat cache_stats.txt\n";
        cout << "This will run the simulation with a cache of size 256KB. Each cache line is 64B wide. Cache has 4-way set associativity. LRU replacement policy. Write-no-allocate policy.\n";
        cout << endl;
}

int parseOptions(int argc, char **argv)
{
    if (argc < 2)
    {
        return 0; // No options provided, use defaults
    }
    if (!strcmp(argv[1], "-h"))
    {
        printUsage();
        return 1;
    }

    *argv++;
    argc--;

    // Process command line arguments
    while (argc > 0)
    {
        if (strcmp(*argv, "-stat") == 0)
        {
            *argv++;
            argc--;
            g_cfg_stats_filename = *argv;
        }
        if (strcmp(*argv, "-trace") == 0)
        {
            *argv++;
            argc--;
            g_cfg_trace_filename = *argv;
        }
        if (strcmp(*argv, "-cs") == 0)
        {
            *argv++;
            argc--;
            g_cfg_cache_size = atoi(*argv) * 1024;
        }
        if (strcmp(*argv, "-ls") == 0)
        {
            *argv++;
            argc--;
            g_cfg_line_size = atoi(*argv);
            if (g_cfg_line_size <= 0)
            {
                cout << "Error: Cache line size must be greater than 0.\n";
                return 1;
            }
            if ((g_cfg_line_size & (g_cfg_line_size - 1)) != 0)
            {
                cout << "Error: Cache line size must be a power of 2.\n";
                return 1;
            }
            if (g_cfg_line_size > g_cfg_cache_size)
            {
                cout << "Error: Cache line size cannot be greater than cache size.\n";
                return 1;
            }
        }
        if (strcmp(*argv, "-w") == 0)
        {
            *argv++;
            argc--;
            g_cfg_num_ways = atoi(*argv);
            if (g_cfg_num_ways < 0)
            {
                cout << "Error: Number of ways must be non-negative.\n";
                return 1;
            }
            if (g_cfg_num_ways > (g_cfg_cache_size / g_cfg_line_size))
            {
                cout << "Error: Number of ways cannot be greater than number of lines in cache.\n";
                return 1;
            }
        }
        if (strcmp(*argv, "-rp") == 0)
        {
            *argv++;
            argc--;
            g_cfg_replacement_policy = atoi(*argv);
            if (g_cfg_replacement_policy < 0 || g_cfg_replacement_policy > 1)
            {
                cout << "Error: Replacement policy must be 0 (LRU) or 1 (FIFO).\n";
                return 1;
            }
        }
        if (strcmp(*argv, "-wap") == 0)
        {
            *argv++;
            argc--;
            int wap = atoi(*argv);
            if (wap == 1)
                g_cfg_write_allocate = true;
            else if (wap == 0)
                g_cfg_write_allocate = false;
            else
            {
                cout << "Error: Write allocate policy must be 0 (Write Allocate) or 1 (No Write Allocate).\n";
                return 1;
            }
        }

        *argv++;
        argc--;
    }

    return 0;
}



int main(int argc, char **argv)
{
    if (parseOptions(argc, argv) != 0)
    {
        return 1; // Error in parsing options
    }

    cout << "------------------------------------------\n";
    cout << "Cache Simulator\n";
    cout << "------------------------------------------\n";
    cout << "Cache size: \t\t" << g_cfg_cache_size << " bytes" << endl;
    cout << "Cache line size: \t" << g_cfg_line_size << " bytes" << endl;
    cout << "Number of ways: \t" << g_cfg_num_ways << endl;
    cout << "Replacement Policy: \t" << (g_cfg_replacement_policy == 0 ? "LRU" : "FIFO") << endl;
    cout << "Write Allocate Policy: \t" << (g_cfg_write_allocate ? "Write Allocate" : "No Write Allocate") << endl;
    cout << "Trace file: \t\t" << ((g_cfg_trace_filename == "")? "Not set. Synthetic random traffic" : g_cfg_trace_filename) << endl;
    cout << "Stats file: \t\t" << g_cfg_stats_filename << endl;

    cache l1_cache(g_cfg_cache_size, g_cfg_line_size, g_cfg_num_ways, g_cfg_replacement_policy, g_cfg_write_allocate, g_cfg_stats_filename);

    // Generate synthetic trace for testing
    cout << "Max addressable space: \t0x" << hex << g_cfg_cache_size - 1 << " bytes" << dec << endl;
    // Define range
    int min = 1;
    int max = g_cfg_cache_size - 1;

    // Initialize a random number generator
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> distrib(min, max);

    for (uint64_t request = 0; request < 100000; request++)
    {
        uint64_t address = ((distrib(gen) >> 6) << 6);        //Must be 64byte aligned
        bool is_write = (rand() % 2) == 0; // Randomly decide if it's a read or write
        //cout << (is_write ? "W " : "R ") << hex << address << dec << endl;
        l1_cache.accessAddress(address, is_write);
    }
    return 0;
}
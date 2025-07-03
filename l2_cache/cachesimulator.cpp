/*
Cache Simulator
Level one L1 and level two L2 cache parameters are read from file (block size, line per set and set per cache).
The 32 bit address is divided into tag bits (t), set index bits (s) and block offset bits (b)
s = log2(#sets)   b = log2(block size in bytes)  t=32-s-b
32 bit address (MSB -> LSB): TAG || SET || OFFSET

Tag Bits   : the tag field along with the valid bit is used to determine whether the block in the cache is valid or not.
Index Bits : the set index field is used to determine which set in the cache the block is stored in.
Offset Bits: the offset field is used to determine which byte in the block is being accessed.
*/

// (c) Preetham Rakshith Prakash 
// References :
// (1) https://rivoire.cs.sonoma.edu/cs351/other/cache_write.html
// (2) https://forum.huawei.com/enterprise/en/differences-between-disk-cache-write-through-and-write-back/thread/667215004455288832-667213859733254144
// (3) https://stackoverflow.com/questions/27087912/write-back-vs-write-through-caching

#include <algorithm>
#include <bitset>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <vector>
using namespace std;
//access state:
#define NA 0 // no action
#define RH 1 // read hit
#define RM 2 // read miss
#define WH 3 // Write hit
#define WM 4 // write miss
#define NOWRITEMEM 5 // no write to memory
#define WRITEMEM 6   // write to memory

struct config
{
    int L1blocksize;
    int L1setsize;
    int L1size;
    int L2blocksize;
    int L2setsize;
    int L2size;
};

/*********************************** ↓↓↓ Todo: Implement by you ↓↓↓ ******************************************/
// You need to define your cache class here, or design your own data structure for L1 and L2 cache

/*
A single cache block:
    - valid bit (is the data in the block valid?)
    - dirty bit (has the data in the block been modified by means of a write?)
    - tag (the tag bits of the address)
    - data (the actual data stored in the block, in our case, we don't need to store the data)
*/
struct CacheBlock
{
    bool valid = false;
    bool dirty = false;
    uint32_t tag;
    // we don't actually need to allocate space for data, because we only need to simulate the cache action
    // or else it would have looked something like this: vector<number of bytes> Data; 
};

/*
A CacheSet:
    - a vector of CacheBlocks
    - a counter to keep track of which block to evict next
*/
struct set
{   
    vector<struct CacheBlock> block_array;
    uint32_t round_robin_counter;
    // tips: 
    // Associativity: eg. resize to 4-ways set associative cache    
};

struct a_cache {
    uint32_t set_array_size;
    uint32_t block_array_size;
    uint32_t tag_bits;
    uint32_t offset_bits;
    vector<struct set> set_array;
};

class my_cache {

    public:

        struct a_cache make_cache(uint32_t cache_size, uint32_t set_size, uint32_t block_size) {
            
            struct a_cache Ln;
            
            uint32_t set_array_size = 0;
            if (set_size == 0) {
                set_array_size = 1;
                set_size = (cache_size * 1024) / block_size;
            } else {
                set_array_size = (cache_size * 1024) / (set_size * block_size);
            }

            Ln.block_array_size = set_size;
            Ln.set_array_size = set_array_size;
            Ln.set_array = vector< struct set>(Ln.set_array_size);
            Ln.offset_bits = log2(block_size);
            Ln.tag_bits = 32 - floor(log2(set_array_size)) - Ln.offset_bits;

            for (uint32_t i = 0; i < Ln.set_array_size; i++) {
                Ln.set_array[i].block_array = vector<struct CacheBlock>(Ln.block_array_size);
                Ln.set_array[i].round_robin_counter = 0;
            }
            return Ln;
        }

        uint32_t get_set_index(struct a_cache* Ln, uint32_t address) {
            if (Ln -> set_array_size == 1) {
                return 0;
            }
            return (address << Ln -> tag_bits) >> (Ln -> tag_bits + Ln -> offset_bits);
        }

        bool hit_or_miss(struct a_cache* Ln, uint32_t address) {
            uint32_t set_index = get_set_index(Ln, address);
            uint32_t tag = (address >> (32 - Ln -> tag_bits));
 
            for (uint32_t i = 0; i < Ln -> block_array_size ; i++) {
                if (Ln -> set_array[set_index].block_array[i].valid == true) {
                    if (Ln -> set_array[set_index].block_array[i].tag == tag) {
                        return true;
                    }
                }
            }            
            return false; 
        }

        bool is_set_full(struct a_cache* Ln, uint32_t address) {
            uint32_t set_index = get_set_index(Ln, address);
            uint32_t tag = (address >> (32 - Ln -> tag_bits));

            for (uint32_t i = 0; i < Ln -> block_array_size; i++) {
                if (Ln -> set_array[set_index].block_array[i].valid == false) {
                    return false;
                }
            }
            return true;
        }

        uint64_t evict(struct a_cache* Ln, uint32_t address) {
            uint32_t set_index = get_set_index(Ln, address);
            uint32_t tag = (address >> (32 - Ln -> tag_bits));

            uint32_t evicted_block = Ln -> set_array[set_index].block_array[Ln -> set_array[set_index].round_robin_counter].tag;
            uint64_t dirty_bit = Ln -> set_array[set_index].block_array[Ln -> set_array[set_index].round_robin_counter].dirty;

            Ln -> set_array[set_index].block_array[Ln -> set_array[set_index].round_robin_counter].valid = false;
            Ln -> set_array[set_index].round_robin_counter += 1;
            Ln -> set_array[set_index].round_robin_counter %= Ln -> block_array_size;

            evicted_block <<= 32 - Ln -> tag_bits;
            return ((evicted_block | ((address << Ln -> tag_bits) >> Ln -> tag_bits)) + (dirty_bit << 32)); 
        }

        bool place_block(struct a_cache* Ln, uint64_t addr) {
            bool dirty_bit = ((addr >> 32) == 1 ) ? true: false;
            uint32_t address = addr;
            uint32_t set_index = get_set_index(Ln, address);
            uint32_t tag = (address >> (32 - Ln -> tag_bits));

            for (uint32_t i = 0; i < Ln -> block_array_size; i++) {
                if (Ln -> set_array[set_index].block_array[i].valid == false) {
                    Ln -> set_array[set_index].block_array[i].valid = true;
                    Ln -> set_array[set_index].block_array[i].tag = tag;
                    Ln -> set_array[set_index].block_array[i].dirty = dirty_bit;
                    return true;
                }
            }
            return false;
        }

        uint64_t invalidate_block(struct a_cache* Ln, uint32_t address) {
            uint32_t set_index = get_set_index(Ln, address);
            uint32_t tag = (address >> (32 - Ln -> tag_bits));

            for (uint32_t i = 0; i < Ln -> block_array_size; i++) {
                if (Ln -> set_array[set_index].block_array[i].tag == tag && Ln -> set_array[set_index].block_array[i].valid == true) {
                    Ln -> set_array[set_index].block_array[i].valid = false;
                    bool is_dirty = Ln -> set_array[set_index].block_array[i].dirty;
                    return (is_dirty == true) ? 1: 0;
                }
            }
            return 0;
        }

        bool set_dirty_bit(struct a_cache* Ln, uint32_t address) {
            uint32_t set_index = get_set_index(Ln, address);
            uint32_t tag = (address >> (32 - Ln -> tag_bits));

            for (uint32_t i = 0; i < Ln -> block_array_size; i++) {
                if (Ln -> set_array[set_index].block_array[i].tag == tag && Ln -> set_array[set_index].block_array[i].valid == true) {
                    Ln -> set_array[set_index].block_array[i].dirty = true;
                    return true;
                }
            }
            return false;
        }
};

/*********************************** ↑↑↑ Todo: Implement by you ↑↑↑ ******************************************/

int main(int argc, char *argv[])
{

    config cacheconfig;
    ifstream cache_params;
    string dummyLine;
    cache_params.open(argv[1]);
    while (!cache_params.eof())                   // read config file
    {
        cache_params >> dummyLine;                // L1:
        cache_params >> cacheconfig.L1blocksize;  // L1 Block size
        cache_params >> cacheconfig.L1setsize;    // L1 Associativity
        cache_params >> cacheconfig.L1size;       // L1 Cache Size
        cache_params >> dummyLine;                // L2:
        cache_params >> cacheconfig.L2blocksize;  // L2 Block size
        cache_params >> cacheconfig.L2setsize;    // L2 Associativity
        cache_params >> cacheconfig.L2size;       // L2 Cache Size
    }
    ifstream traces;
    ofstream tracesout;
    string outname;
    outname = string(argv[2]) + ".out";
    traces.open(argv[2]);
    tracesout.open(outname.c_str());
    string line;
    string accesstype;     // the Read/Write access type from the memory trace;
    string xaddr;          // the address from the memory trace store in hex;
    unsigned int addr;     // the address from the memory trace store in unsigned int;
    bitset<32> accessaddr; // the address from the memory trace store in the bitset;




/*********************************** ↓↓↓ Todo: Implement by you ↓↓↓ ******************************************/
    // Implement by you:
    // initialize the hirearch cache system with those configs
    // probably you may define a Cache class for L1 and L2, or any data structure you like
    if (cacheconfig.L1blocksize!=cacheconfig.L2blocksize){
        printf("please test with the same block size\n");
        return 1;
    }

    my_cache my_cache;
    
    struct a_cache L1 = my_cache.make_cache(cacheconfig.L1size, cacheconfig.L1setsize, cacheconfig.L1blocksize);
    struct a_cache L2 = my_cache.make_cache(cacheconfig.L2size, cacheconfig.L2setsize, cacheconfig.L2blocksize);
  
    // cache c1(cacheconfig.L1blocksize, cacheconfig.L1setsize, cacheconfig.L1size,
    //          cacheconfig.L2blocksize, cacheconfig.L2setsize, cacheconfig.L2size);
    int L1AcceState = NA; // L1 access state variable, can be one of NA, RH, RM, WH, WM;
    int L2AcceState = NA; // L2 access state variable, can be one of NA, RH, RM, WH, WM;
    int MemAcceState = NOWRITEMEM; // Main Memory access state variable, can be either NA or WH;

    if (traces.is_open() && tracesout.is_open())
    {
        while (getline(traces, line))
        { // read mem access file and access Cache

            istringstream iss(line);
            if (!(iss >> accesstype >> xaddr)){
                break;
            }
            stringstream saddr(xaddr);
            saddr >> std::hex >> addr;
            accessaddr = bitset<32>(addr);
            uint64_t address = addr; //////////////////////////////////
            // access the L1 and L2 Cache according to the trace;
            if (accesstype.compare("R") == 0)  // a Read request
            {   
                if (my_cache.hit_or_miss(&L1, address) == true) {
                    L1AcceState = RH;
                    L2AcceState = NA;
                    MemAcceState = NOWRITEMEM;
                } else if (my_cache.hit_or_miss(&L2, address) == true) {
                
                    L1AcceState = RM;
                    L2AcceState = RH;
                    
                    uint64_t dirty = my_cache.invalidate_block(&L2, address);
                    address |= (dirty << 32);

                    if (my_cache.is_set_full(&L1, address) == false) {
                        my_cache.place_block(&L1, address);
                        MemAcceState = NOWRITEMEM;
                    } else {
                        uint64_t evicted_block = my_cache.evict(&L1, address);
                        my_cache.place_block(&L1, address);
                        if (my_cache.is_set_full(&L2, evicted_block) == false) {
                            my_cache.place_block(&L2, evicted_block);
                            MemAcceState = NOWRITEMEM;
                        } else {
                            uint64_t l2_evicted_block = my_cache.evict(&L2, evicted_block);
                            my_cache.place_block(&L2, evicted_block);

                            if ((l2_evicted_block >> 32) == 1) {
                                MemAcceState = WRITEMEM;
                            } else {
                                MemAcceState = NOWRITEMEM;
                            }
                        }
                        
                    } 

                } else {

                    L1AcceState = RM;
                    L2AcceState = RM;

                    if (my_cache.is_set_full(&L1, address) == false) {
                        my_cache.place_block(&L1, address);
                        MemAcceState = NOWRITEMEM;
                    } else {
                       
                        uint64_t evicted_block = my_cache.evict(&L1, address);
                        my_cache.place_block(&L1, address);

                        if (my_cache.is_set_full(&L2, evicted_block) == false) {
                            my_cache.place_block(&L2, evicted_block);
                            MemAcceState = NOWRITEMEM;
                        } else {
                            uint64_t evicted_block_second = my_cache.evict(&L2, evicted_block);
                            my_cache.place_block(&L2, evicted_block);
                            if ((evicted_block_second >> 32) == 1) {
                                MemAcceState = WRITEMEM;
                            } else {
                                MemAcceState = NOWRITEMEM;
                            }
                        }
                    } 
                }
            } else { 
                
                if (my_cache.hit_or_miss(&L1, address) == true) {
                    my_cache.set_dirty_bit(&L1, address);
                    L1AcceState = WH;
                    L2AcceState = NA;
                    MemAcceState = NOWRITEMEM;
                } else if (my_cache.hit_or_miss(&L2, address) == true) {
                    my_cache.set_dirty_bit(&L2, address);
                    L1AcceState = WM;
                    L2AcceState = WH;
                    MemAcceState = NOWRITEMEM;
                } else {
                    L1AcceState = WM;
                    L2AcceState = WM;
                    MemAcceState = WRITEMEM;
                }
            }
/*********************************** ↑↑↑ Todo: Implement by you ↑↑↑ ******************************************/

            // Grading: don't change the code below.
            // We will print your access state of each cycle to see if your simulator gives the same result as ours.
            tracesout << L1AcceState << " " << L2AcceState << " " << MemAcceState << endl; // Output hit/miss results for L1 and L2 to the output file;
        }
        traces.close();
        tracesout.close();
    }
    else
        cout << "Unable to open trace or traceout file ";

    return 0;
}

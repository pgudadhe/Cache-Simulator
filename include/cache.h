#pragma once

#include "cache_common.h"
#include <vector>
#include <fstream>

struct cache_line
{
    bool valid;
    uint64_t tag;
    uint64_t last_access_time; // For LRU
    uint64_t load_time;        // For FIFO

    cache_line() : valid(false), tag(0), last_access_time(0), load_time(0) {}
};

class cache
{
public:
    cache(uint32_t size_bytes, uint32_t line_size, uint32_t num_ways, uint32_t replacement_policy, bool write_allocate, string stats_filename);
    ~cache();

    void printStats();  
    void resetStats();

    uint64_t getTag(uint64_t address) const
    {
        return FIELD_EXT_64(address, m_tag_fld_msb, m_tag_fld_lsb);
    };
    uint64_t getIndex(uint64_t address) const
    {
        return FIELD_EXT_64(address, m_index_fld_msb, m_index_fld_lsb);
    };
    uint64_t getOffset(uint64_t address) const
    {
        return FIELD_EXT_64(address, m_offset_fld_msb, m_offset_fld_lsb);
    };

    void accessAddress(uint64_t address, bool is_write);
    uint32_t findReplacementWay(uint32_t index);
    uint32_t findReplacementWayLRU(uint32_t index);
    uint32_t findReplacementWayFIFO(uint32_t index);
    int findEmptyCacheLineWay(uint32_t index);

private:
    uint32_t m_cache_size; // in bytes
    uint32_t m_line_size;  // in bytes
    uint32_t m_num_ways;   // associativity
    uint32_t m_replacement_policy; // 0: LRU, 1: FIFO
    bool m_write_allocate; // true: Write Allocate, false: No Write Allocate
    uint32_t m_num_lines;
    uint32_t m_num_sets;
    uint64_t m_current_time = 0;

    uint32_t m_offset_bits = 0;
    uint32_t m_index_bits = 0;
    uint32_t m_tag_bits = 0;
    uint32_t m_offset_fld_lsb = 0;
    uint32_t m_offset_fld_msb = 0;
    uint32_t m_index_fld_lsb = 0;
    uint32_t m_index_fld_msb = 0;
    uint32_t m_tag_fld_lsb = 0;
    uint32_t m_tag_fld_msb = 0;

    vector<vector<cache_line>> m_cache; 


    // Statistics
    string m_stats_filename = "";
    fstream m_stats_file;
    uint32_t m_stat_hits = 0;
    uint32_t m_stat_misses = 0;
    uint32_t m_stat_accesses = 0;
};


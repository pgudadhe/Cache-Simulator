#include "cache.h"



cache::cache(uint32_t size_bytes, uint32_t line_size, uint32_t num_ways, uint32_t replacement_policy, bool write_allocate, string stats_filename)
    : m_cache_size(size_bytes), m_line_size(line_size), m_num_ways(num_ways), m_replacement_policy(replacement_policy),
      m_stat_hits(0), m_stat_misses(0), m_stat_accesses(0), m_write_allocate(true), m_stats_filename(stats_filename)
{
    if (m_line_size <= 0 || (m_line_size & (m_line_size - 1)) != 0)
    {
        throw invalid_argument("Cache line size must be a positive power of 2.");
    }
    if (m_cache_size <= 0 || m_cache_size % m_line_size != 0)
    {
        throw invalid_argument("Cache size must be a positive multiple of line size.");
    }
    if (m_num_ways < 0 || (m_num_ways > (m_cache_size / m_line_size)))
    {
        throw invalid_argument("Number of ways must be between 0 (direct-mapped) and the number of lines in the cache.");
    }
    if (m_replacement_policy < 0 || m_replacement_policy > 1)
    {
        throw invalid_argument("Replacement policy must be 0 (LRU) or 1 (FIFO).");
    }

    m_stats_file.open(m_stats_filename, ios::out);
    if (!m_stats_file.is_open())
    {
        throw runtime_error("Failed to open stats file: " + m_stats_filename);
    }   

    m_num_lines = m_cache_size / m_line_size;
    m_stats_file << "Cache Size (KB) \t\t" << m_cache_size / 1024 << endl;
    m_stats_file << "Line Size (bytes) \t\t" << m_line_size << endl;
    m_stats_file << "Number of Ways \t\t\t" << (m_num_ways == 0 ? 1 : m_num_ways) << endl;
    m_stats_file << "Replacement Policy \t\t" << (m_replacement_policy == 0 ? "LRU" : "FIFO") << endl;
    m_stats_file << "Write Allocate Policy \t\t" << (m_write_allocate ? "Write Allocate" : "No Write Allocate") << endl;

    m_num_sets = (m_num_ways == 0) ? m_num_lines : (m_num_lines / m_num_ways);

    m_offset_bits = (uint32_t)log2(m_line_size);
    m_index_bits = (uint32_t)log2(m_num_sets);
    m_tag_bits = 64 - m_index_bits - m_offset_bits;

    m_stats_file << "Offset bits: \t\t\t" << m_offset_bits << endl;
    m_stats_file << "Index bits: \t\t\t" << m_index_bits << endl;
    m_stats_file << "Tag bits: \t\t\t" << m_tag_bits << endl;

    m_offset_fld_lsb = 0;
    m_offset_fld_msb = m_offset_bits - 1;
    m_index_fld_lsb = m_offset_bits;
    m_index_fld_msb = m_offset_bits + m_index_bits - 1;
    m_tag_fld_lsb = m_offset_bits + m_index_bits;
    m_tag_fld_msb = 63;

    m_cache.resize(m_num_ways, vector<cache_line>(m_num_sets));
}

cache::~cache()
{
    // Put stats in the stats file
    printStats();

    if (m_stats_file.is_open())
    {
        m_stats_file.close();
    }
}

void cache::resetStats()
{
    m_stat_hits = 0;
    m_stat_misses = 0;
    m_stat_accesses = 0;
    m_current_time = 0;
}

void cache::printStats()
{
    cout << "------------------------------------------\n";
    cout << "Cache Statistics:\n";
    cout << "------------------------------------------\n";
    cout << "  Cache Size: \t\t" << m_cache_size / 1024  << " KB\n";
    cout << "  Line Size: \t\t" << m_line_size << " bytes\n";
    cout << "  Number of Ways: \t" << (m_num_ways == 0 ? 1 : m_num_ways) << "\n";
    cout << "  Replacement Policy: \t" << (m_replacement_policy == 0 ? "LRU" : "FIFO") << "\n";
    cout << "  Write Allocate: \t" << (m_write_allocate ? "Yes" : "No") << "\n";
    cout << "  Total Accesses: \t" << m_stat_accesses << "\n";
    m_stats_file << "Total Accesses: \t\t" << m_stat_accesses << "\n";
    cout << "  Total Hits: \t\t" << m_stat_hits << "\n";
    m_stats_file << "Total Hits: \t\t\t" << m_stat_hits << "\n";
    cout << "  Total Misses: \t" << m_stat_misses << "\n";
    m_stats_file << "Total Misses: \t\t\t" << m_stat_misses << "\n";
    double hit_rate = (m_stat_accesses > 0) ? (static_cast<double>(m_stat_hits) / m_stat_accesses) * 100.0 : 0.0;
    cout << "  Hit Rate: \t\t" << hit_rate << "%\n";
    m_stats_file << "Hit Rate: \t\t\t" << hit_rate << "%\n";
    cout << "  Miss Rate: \t\t" << (100.0 - hit_rate) << "%\n";
    m_stats_file << "Miss Rate: \t\t\t" << (100.0 - hit_rate) << "%\n";
    cout << "------------------------------------------\n";
}

void cache::accessAddress(uint64_t address, bool is_write)
{
    m_stat_accesses++;
    uint64_t index = getIndex(address);
    uint64_t tag = getTag(address);

    // Check for hit
    bool hit = false;
    for (uint32_t way = 0; way < (m_num_ways == 0 ? 1 : m_num_ways); way++)
    {
        if (m_cache[way][index].valid && m_cache[way][index].tag == tag)
        {
            // Cache hit
            m_stat_hits++;
            m_cache[way][index].last_access_time = m_current_time; // Update last access time for LRU
            hit = true;
        }
    }

    if(!hit)
    {
        // Cache miss
        m_stat_misses++;

        if(is_write & !m_write_allocate)
        {
            // For no allocate, we do not load the block into cache on a write miss
            m_current_time++;
            return;
        }
        else
        {
            // First check for an empty line
            int32_t empty_way = findEmptyCacheLineWay((uint32_t)index);

            if(empty_way != -1)
            {
                // Found an empty line, use it
                m_cache[empty_way][index].valid = true;
                m_cache[empty_way][index].tag = tag;
                m_cache[empty_way][index].last_access_time = m_current_time; // For LRU
                m_cache[empty_way][index].load_time = m_current_time;        // For FIFO
                m_current_time++;
                return;
            }
            else
            {
                // No empty line, need to replace one
                uint32_t way_to_replace = findReplacementWay((uint32_t)index);
                m_cache[way_to_replace][index].valid = true;
                m_cache[way_to_replace][index].tag = tag;
                m_cache[way_to_replace][index].last_access_time = m_current_time; // For LRU
                m_cache[way_to_replace][index].load_time = m_current_time;        // For FIFO
            }

        }
    }   
    m_current_time++;
}

uint32_t cache::findReplacementWay(uint32_t index)
{
    if(m_replacement_policy == 0)
    {
        return findReplacementWayLRU(index);
    }
    else
    {
        return findReplacementWayFIFO(index);
    }
}

uint32_t cache::findReplacementWayLRU(uint32_t index)
{
    uint32_t lru_way = 0;
    uint64_t min_time = m_cache[0][index].last_access_time;

    for (uint32_t way = 1; way < (m_num_ways == 0 ? 1 : m_num_ways); way++)
    {
        if (m_cache[way][index].last_access_time < min_time)
        {
            min_time = m_cache[way][index].last_access_time;
            lru_way = way;
        }
    }
    return lru_way;
}

uint32_t cache::findReplacementWayFIFO(uint32_t index)
{
    uint32_t fifo_way = 0;
    uint64_t min_time = m_cache[0][index].load_time;

    for (uint32_t way = 1; way < (m_num_ways == 0 ? 1 : m_num_ways); way++)
    {
        if (m_cache[way][index].load_time < min_time)
        {
            min_time = m_cache[way][index].load_time;
            fifo_way = way;
        }
    }
    return fifo_way;
}

int cache::findEmptyCacheLineWay(uint32_t index)
{
    for (uint32_t way = 0; way < (m_num_ways == 0 ? 1 : m_num_ways); way++)
    {
        if (!m_cache[way][index].valid)
        {
            return way;
        }
    }
    return -1; // No empty line found
}
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <set>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <random>
#include <algorithm>

// #include <assert.h>
// #include <stdio.h>


using namespace std;


struct addre
{
    uint32_t page_index;
    uint32_t cacheline_index;   // Only the last 1 byte is used

    // Comparison operators to enable set operations
    bool operator<(const addre& other) const {
        return page_index < other.page_index || (page_index == other.page_index && cacheline_index < other.cacheline_index);
    }

    bool operator==(const addre& other) const {
        return page_index == other.page_index && cacheline_index == other.cacheline_index;
    }
};



int main(){

    /*                                         Part 1: Wrote Log Indexing Time                                      */

    std::vector<std::unordered_map<uint8_t, uint32_t>*> array_of_2nd_level_log_tables;
    std::unordered_map<uint32_t, std::unordered_map<uint8_t, uint32_t>*> the_1st_level_log_table;

    std::set<addre> addr_pool;

    //Initialization of array_of_2nd_log_tables, suppose a 128MB log
    int64_t maxN_log = 128 * 1024 * 1024 / 64;

    uint8_t random8bit; //Cacheline index in one page
    uint32_t random32bit; //Page index as a 32-bit value

    random32bit = rand() | (rand() << 16); // A initial page index

    // Fill the index table
    for (size_t i = 0; i < maxN_log/2; i++)
    {
        random8bit = (rand() % 256) % 64; 
        if (rand()%4==1)
        {
            random32bit = rand() | (rand() << 16); // A new page, otherwise keep old page
        }

        uint32_t log_offset = i;

        if (the_1st_level_log_table.find(random32bit)==the_1st_level_log_table.end())
        {
            std::unordered_map<uint8_t, uint32_t>* new_2_table = new std::unordered_map<uint8_t, uint32_t>;
            the_1st_level_log_table[random32bit] = new_2_table;
            array_of_2nd_level_log_tables.push_back(new_2_table);
        }

        std::unordered_map<uint8_t, uint32_t>* the_2_table = the_1st_level_log_table[random32bit];
        (*the_2_table)[random8bit] = log_offset;

        addr_pool.insert({random32bit, (uint32_t)random8bit});
    }

    // Create a vector to store the elements of the addr_pool
    std::vector<addre> myVector;

    // Traverse the set and push each element into the vector 
    myVector.reserve(addr_pool.size() * 2);
    for (const auto& element : addr_pool) {
        myVector.push_back(element);
    }
    while (myVector.size() < addr_pool.size() * 2)   // Add some accesses that will miss, because we are reading the write log for every access, so the hit ratio is not high
    { 
        random8bit = (rand() % 256) % 64;
        random32bit = rand() | (rand() << 16);
        myVector.push_back({random32bit, (uint32_t)random8bit});
    }
    

    // Obtain a random seed
    std::random_device rd;
    std::mt19937 gen(rd());

    // Shuffle the vector
    std::shuffle(myVector.begin(), myVector.end(), gen);


    int32_t sum; // Prevent the O3 compiler to optimize out the indexing process

    // Start the clock
    auto start = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < 100000000; i++)
    {
        uint32_t i_page = myVector[i%myVector.size()].page_index;   // sequential access, I think the prefetcher can work well to bring the myVector into CPU cache
        uint8_t i_cacheline = (uint8_t)(myVector[i%myVector.size()].cacheline_index);
        std::unordered_map<uint32_t, std::unordered_map<uint8_t, uint32_t>*>::iterator it;
        if ((it=the_1st_level_log_table.find(i_page))!=the_1st_level_log_table.end())
        {
            //std::unordered_map<uint8_t, uint32_t>* the_2_table = the_1st_level_log_table[i_page];
            if ((*((*it).second)).find(i_cacheline)!=(*((*it).second)).end())
            {
                //int32_t log_idx = (*the_2_table)[i_cacheline];  // Find the log idx
                sum++; // Prevent the O3 compiler to optimize out the indexing process
            }
        }
    }

    // End the clock
    auto end = std::chrono::high_resolution_clock::now();

    // Calculate the duration
    std::chrono::duration<double> duration = end - start;

    // Output the duration in seconds
    std::cout << "Elapsed time for 100000000 times write log lookup: " << duration.count() << " seconds." << std::endl;
    std::cout << "Sum result " << sum << ", lol." << std::endl;

    for (auto& element : array_of_2nd_level_log_tables) //Free allocated memory
    {
        delete element;
    }
    





    /*                                         Part 2: SSD Cache Indexing Time                                      */

    //Assume we have a 1GB SSD DRAM Cache
    std::unordered_map<uint32_t, uint32_t> ssd_cache;
    vector<uint32_t> addr_set;
    addr_set.reserve(262144*2);

    //Fill data
    for (size_t i = 0; i < 262144; i++)
    {
        random32bit = rand() | (rand() << 16);
        ssd_cache[random32bit] = i;
        addr_set.push_back(random32bit);
    }

    for (size_t i = 0; i < 262144; i++)
    {
        random32bit = rand() | (rand() << 16);
        addr_set.push_back(random32bit);
    }

    // Shuffle the vector
    std::shuffle(addr_set.begin(), addr_set.end(), gen);


    //Test
     // Start the clock
    start = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < 100000000; i++)
    {
        uint32_t i_page = addr_set[i%addr_set.size()];   // sequential access, I think the prefetcher can work well to bring the myVector into CPU cache

        if (ssd_cache.find(i_page)!=ssd_cache.end())
        {
            sum++; // Prevent the O3 compiler to optimize out the indexing process
        }
    }

    // End the clock
    end = std::chrono::high_resolution_clock::now();

    // Calculate the duration
    duration = end - start;

    // Output the duration in seconds
    std::cout << "Elapsed time for 100000000 times cache lookup: " << duration.count() << " seconds." << std::endl;
    std::cout << "Sum result " << sum << ", lol." << std::endl;

    
    

    
    return 0;
}
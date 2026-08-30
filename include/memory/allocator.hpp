#ifndef __MEMORY_ARENA_HPP_INCLUDED__
#define __MEMORY_ARENA_HPP_INCLUDED__

#include "utils/constants.hpp"

#include <new>
#include <list>
#include <type_traits>
#include <vector>

/*
    Arena based allocator which allows for memory in 16 base 
    aligned contiguous regions
*/
class allocator {

    static inline constexpr size_t KiB_256 = 262144;

public:

    allocator(const size_t block_size = KiB_256)   : block_size(block_size) {};
    ~allocator() noexcept
    {
        // Iterate through destructors (backwords)
        for(int i = destructors.size() - 1; i >= 0; --i)
        {
            const destructor_pack& pack = destructors[i];
            pack.destructor_fn_ptr(pack.obj);
        }

        aligned_free(current_block);
        for(std::pair<size_t, uint8_t*>& block: used_blocks)        aligned_free(block.second);
        for(std::pair<size_t, uint8_t*>& block: available_blocks)   aligned_free(block.second);
    }

    void* alloc(size_t bytes)
    {
        // round up the address to a multiple of 16
        bytes = (bytes + 15) & ~15; 

        // no space for the data
        if(current_block_position + bytes > current_allocated_size)
        {
            if(current_block != nullptr)
            {
                used_blocks.push_back(std::make_pair(current_allocated_size, current_block));
                current_block = nullptr;
            }

            // search for a freed block
            for(auto iter = available_blocks.begin(); iter != available_blocks.end(); ++iter)
            {
                if(iter->first >= bytes)
                {
                    current_allocated_size = iter->first;
                    current_block = iter->second;
                    available_blocks.erase(iter);
                    break;
                }
            }

            if(current_block == nullptr)
            {
                // Allocated enough memory if no block available... 
                current_allocated_size = std::max(bytes, block_size);
                current_block = (uint8_t*)aligned_alloc(current_allocated_size * sizeof(uint8_t));
            }
            current_block_position = 0;
        }

        void* result = current_block + current_block_position;
        current_block_position += bytes;
        return result;
    }

    template<typename T>
    T* alloc(const size_t cnt = 1, bool run_constructor = true)
    {
        T* mem = static_cast<T*>(alloc(sizeof(T) * cnt));

        if(run_constructor)
        {
            for(size_t i = 0; i < cnt; ++i)
            {
                new (&mem[i]) T(); // placement new
            }

            // Has a destructor
            if constexpr(!std::is_trivially_destructible_v<T>)
            {
                for(size_t i = 0; i < cnt; ++i)
                {
                    destructors.push_back({
                        &mem[i],
                        [] (void* ptr)
                        {
                            static_cast<T*>(ptr)->~T(); // call the destructor for the block of memory
                        }
                    });
                }
                
            }
        }
        
        return mem;
    }

private:

    const size_t    block_size;                         // <- global size for a block
    size_t          current_block_position  = 0;        // <- offset from the block
    size_t          current_allocated_size  = 0;        // <- how much is allocted from the current block
    uint8_t        *current_block           = nullptr;  // <- position in memory    
         
    std::list<std::pair<size_t, uint8_t*>> used_blocks, available_blocks; // <- memory

    struct destructor_pack
    {
        void* obj;
        void (*destructor_fn_ptr)(void* obj);
    };

    std::vector<destructor_pack> destructors;
};

#endif
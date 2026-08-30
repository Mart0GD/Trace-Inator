#ifndef __TAGGED_POINTER_HPP_INCLUDED__
#define __TAGGED_POINTER_HPP_INCLUDED__

#include "utils/constants.hpp"
#include "memory/type_pack.hpp"


template<typename... Ts>
class tagged_pointer {
public:
    using Types = type_pack<Ts...>; // <- alias for all the defined types

    constexpr tagged_pointer()                      : bits(0) {};
    constexpr tagged_pointer(std::nullptr_t)        : bits(0) {};

    template<typename T>
    explicit tagged_pointer(const T* ptr)
    {
        static_assert(alignof(T) >= 8);

        uintptr_t iptr = reinterpret_cast<uintptr_t>(ptr); // <- get numerical value of pointer
        constexpr size_t type_idx = type_index<T>();

        bits = iptr | type_idx; // <- last three bits are always zero 
    }

    template<typename T>
    bool is()    const { return tag() == type_index<T>(); } 
    size_t tag() const { return bits & tag_mask; }

    void* ptr() const  { return reinterpret_cast<void*>(bits & ptr_mask);}

    template<typename T>
    T* get() const  
    { 
        if(is<T>()) return reinterpret_cast<T*>(ptr());
        return nullptr;
    }

private:

    template<typename T> 
    static constexpr size_t type_index()
    {
        // removes const, constexpr, volotile etc...
        using Type = typename std::remove_cv_t<T>; 

        // zero reserved for nullptr
        if constexpr(std::is_same_v<Type, std::nullptr_t>) return 0;
        return 1 + index_of<Type, Types>::count;
    }


private:
    static constexpr size_t     tag_bits    = 3; // <- max 7 types for encoding + nullptr
    static constexpr uint64_t   tag_mask    = ((1ull << tag_bits) - 1);
    static constexpr uint64_t   ptr_mask    = ~tag_mask;

    uintptr_t                   bits        = 0;
};

#endif
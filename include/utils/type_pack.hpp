#ifndef __TYPE_PACK_HPP_INCLUDED__
#define __TYPE_PACK_HPP_INCLUDED__

#include <type_traits>

/*
    Type structure used to represent a collection of other types
*/
template<typename... Ts>
struct type_pack
{
    static constexpr size_t count = sizeof...(Ts);  // <- number of types
};


// -- SEARCHING IN A TYPE PACK --


template<typename T, typename Pack>
struct index_of;

/*
    Case 1: Type not found in type pack, the assert is type dependant and it will always be false!
*/
template<typename T>
struct index_of<T, type_pack<>>
{
    static constexpr size_t count = 0;
    static_assert(!std::is_same_v<T,T>, "Type not in type pack!");
};

/*
    Case 2: The first type T matches a type in the pack, stop recursion
*/
template<typename T, typename... Ts>
struct index_of<T, type_pack<T, Ts...>>
{
    static constexpr size_t count = 0;
};

/*
    Case 3: No match, continue with the next types
*/
template <typename T, typename U, typename... Ts>
struct index_of<T, type_pack<U, Ts...>> {
    static constexpr int count = 1 + index_of<T, type_pack<Ts...>>::count;
};


#endif
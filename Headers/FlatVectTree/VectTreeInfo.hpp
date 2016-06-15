#ifndef VECT_TREE_INFO
#define VECT_TREE_INFO

#include "../MexMem.hpp"

// Code to get information of a particular Nested MexVector Type
template <typename T, typename B = void> struct getTreeInfoBasic
{
	static const uint32_t depth = uint32_t(-1);
};

template <typename T, class Al> struct getTreeInfoBasic<MexVector<T, Al> > {
	static constexpr uint32_t depth = 0;
	typedef T type;
};

template <typename T, class Al> struct getTreeInfoBasic<MexVector<MexVector<T, Al> > > {
	static constexpr uint32_t depth = getTreeInfoBasic<MexVector<T> >::depth + 1;
	typedef typename getTreeInfoBasic<MexVector<T> >::type type;
};

// This is a wrapper that removes const, volatile and references from types
template <typename T>
struct getTreeInfo : public getTreeInfoBasic<typename std::decay<T>::type> {};

#endif
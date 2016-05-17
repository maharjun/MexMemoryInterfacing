#ifndef VECT_TREE_INFO
#define VECT_TREE_INFO

#include "../MexMem.hpp"

// Code to get information of a particular Nested MexVector Type
template <typename T, typename B = void> struct getTreeInfo
{
	static const int depth = -1;
};

// This is a specialization that removes const, volatile and references from types
template <typename T> 
struct getTreeInfo<
		T, 
		typename std::enable_if <
			std::is_const<T>::value || 
			std::is_volatile<T>::value || 
			std::is_reference<T>::value
		>::type
	>
	{
	static constexpr int depth = getTreeInfo<
		typename std::remove_reference<
			typename std::remove_cv<T>::type
		>::type
	>::depth;
	typedef
        typename getTreeInfo<
            typename std::remove_reference<
                typename std::remove_cv<T>::type
            >::type
        >::type
        type;
};

template <typename T, class Al> struct getTreeInfo<MexVector<T, Al> > {
	static constexpr int depth = 0;
	typedef T type;
};
template <typename T, class Al> struct getTreeInfo<MexVector<MexVector<T, Al> > > {
	static constexpr int depth = getTreeInfo<MexVector<T> >::depth + 1;
	typedef typename getTreeInfo<MexVector<T> >::type type;
};

#endif
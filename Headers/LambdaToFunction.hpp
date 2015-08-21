#ifndef LAMBDA_TO_FUNCTION
#define LAMBDA_TO_FUNCTION

#include <iostream>
#include <functional>
#include <type_traits>

#define NonDeduc(T) typename std::decay< T >::type

template<typename T>
struct memfun_type
{
	using type = void;
};

template<typename Ret, typename Class, typename... Args>
struct memfun_type<Ret(Class::*)(Args...) const>
{
	using type = std::function<Ret(Args...)>;
};

template<typename F>
typename memfun_type<decltype(&F::operator())>::type
FFL(F const &func)
{ // Function from lambda !
	return typename memfun_type<decltype(&F::operator())>::type(func);
}

#endif

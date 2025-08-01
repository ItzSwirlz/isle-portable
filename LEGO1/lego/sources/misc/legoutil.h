#ifndef __LEGOUTIL_H
#define __LEGOUTIL_H

// Exclude from modern compilers due to clash with mxutilities.h
#ifndef COMPAT_MODE

template <class T>
inline T Min(T p_t1, T p_t2)
{
	return p_t1 < p_t2 ? p_t1 : p_t2;
}

template <class T>
inline T Min(T p_t1, T p_t2, T p_t3)
{
	return Min(p_t1, Min(p_t2, p_t3));
}

template <class T>
inline T Max(T p_t1, T p_t2)
{
	return p_t1 > p_t2 ? p_t1 : p_t2;
}

template <class T>
inline T Max(T p_t1, T p_t2, T p_t3)
{
	return Max(p_t1, Max(p_t2, p_t3));
}

template <class T>
inline T Abs(T p_t)
{
	return p_t < 0 ? -p_t : p_t;
}

#endif

template <class T>
inline void Swap(T& p_t1, T& p_t2)
{
	T t = p_t1;
	p_t1 = p_t2;
	p_t2 = t;
}

// TEMPLATE: BETA10 0x10073c20
// Swap

template <class T>
inline T DToR(T p_d)
{
	return p_d * 3.1416F / 180.0F;
}

template <class T>
inline T RToD(T p_r)
{
	return p_r * 180.0F / 3.1416F;
}

template <class... Ts>
struct overloaded : Ts... {
	using Ts::operator()...;
};

template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

#endif // __LEGOUTIL_H

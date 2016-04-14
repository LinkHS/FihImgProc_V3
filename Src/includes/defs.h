#ifndef __FIH_IMGPROC_DEFS__
#define __FIH_IMGPROC_DEFS__

typedef unsigned char uchar;
typedef unsigned int uint;

namespace FIH
{
template<class T> const T& min (const T& a, const T& b)    { return (a<b)?a:b; }
template<class T> const T& max (const T& a, const T& b)    { return (a<b)?b:a; }
template<typename _Tp> static inline _Tp saturate_cast(int v)  { return _Tp(v); }
template<> inline uchar saturate_cast<uchar>(int v)        { return (uchar)((unsigned)v <= 255 ? v : v > 0 ? 255 : 0); }
}

/* definition to expand macro then apply to pragma message */
#define VALUE_TO_STRING(x) #x
#define VALUE(x) VALUE_TO_STRING(x)
#define VAR_NAME_VALUE(var) #var "="  VALUE(var)

#endif
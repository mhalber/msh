// This is an experimental file to demonstrate minimal template programming in C.
// With a terrible abuse of preprocessor power we are able to have this file copy itself for each type.
// Everytime it defines a the required types and function for each requested type.

#ifndef T

// Type request
#  define T int
#  define POSTFIX i
# include __FILE__
#  undef POSTFIX
#  undef T
#  define T float
#  define POSTFIX f
#  include __FILE__
#  undef POSTFIX
#  undef T 
#  define T double
#  define POSTFIX d
#  include __FILE__

#undef NAMESPACE

#else

// We need to do namespaces by hand in C
#define NAMESPACE msh

// All tremble 'fore the Mighty Preprocessor!
#define _APPEND(a,b) a ## b
#define APPEND(a,b) _APPEND(a,b)
#define _MERGE(a, b) a ## _ ## b
#define MERGE(a, b) _MERGE(a, b)
#define XTYPE_NAME(name, postfix) name##postfix
#define TYPE_NAME(name, postfix) XTYPE_NAME(name, postfix)
#define FUNC(width, op) MERGE( MERGE(NAMESPACE, APPEND(width, POSTFIX)), op)

#define VEC2 MERGE( TYPE_NAME( MERGE( NAMESPACE, vec2), POSTFIX), t)
#define VEC3 MERGE( TYPE_NAME( MERGE( NAMESPACE, vec3), POSTFIX), t)
#define VEC4 MERGE( TYPE_NAME( MERGE( NAMESPACE, vec4), POSTFIX), t)

// Write your code (almost) as usual
typedef union VEC3
{
  T data[3];
  struct{ T x; T y; T z; };
} VEC3;

VEC3
FUNC(vec3,init)( T x, T y, T z )
{
  return (VEC3){ .x = x, .y = y, .z = z };
}

VEC3
FUNC(vec3,zeros)()
{
  return (VEC3){ .x = 0, .y = 0, .z = 0 };
}

VEC3
FUNC(vec3,ones)()
{
  return (VEC3){ .x = 1, .y = 1, .z = 1 };
}

VEC3
FUNC(vec3,add)( const VEC3 a, const VEC3 b )
{
  return (VEC3){ .x = a.x + b.x, .y = a.y + b.y, .z = a.z + b.z };
}

VEC3
FUNC(vec3,sub)( const VEC3 a, const VEC3 b )
{
  return (VEC3){ .x = a.x - b.x, .y = a.y - b.y, .z = a.z - b.z };
}

VEC3
FUNC(vec3, mul)( const VEC3 a, const VEC3 b)
{
  return (VEC3){ .x = a.x * b.x, .y = a.y * b.y, .z = a.z * b.z };
}

VEC3
FUNC(vec3, div)( const VEC3 a, const VEC3 b)
{
  return (VEC3){ .x = a.x / b.x, .y = a.y / b.y, .z = a.z / b.z };
}

VEC3
FUNC(scalar_vec3,add)( const T b, const VEC3 a )
{
  return (VEC3){ .x = a.x + b, .y = a.y + b, .z = a.z + b };
}

VEC3
FUNC(scalar_vec3,sub)( const T b, const VEC3 a )
{
  return (VEC3){ .x = a.x - b, .y = a.y - b, .z = a.z - b };
}

VEC3
FUNC(scalar_vec3, mul)( const T b, const VEC3 a )
{
  return (VEC3){ .x = a.x * b, .y = a.y * b, .z = a.z * b };
}

VEC3
FUNC(scalar_vec3, div)( const T b, const VEC3 a )
{
  double denom = (1.0 / (double)b);
  return (VEC3){ .x = a.x * denom, .y = a.y * denom, .z = a.z * denom };
}

T
FUNC(vec3,dot)( const VEC3 a, const VEC3 b )
{
  T out = ( a.x * b.x + a.y * b.y + a.z * b.z );
  return out;
}

#endif
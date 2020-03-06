#ifndef SM_RAY_H_
#define SM_RAY_H_
/**************************************************************
*
*		ray.h
*
*		class ray
*
*		actual ray for raycasting		
*
**************************************************************/

#include "vec3.h"

class ray {
public:
	// constructors
	__device__ ray() {};
	__device__ ray(const vec3 &origin, const vec3 &direcion) : o(origin), d(normalize(direcion)) {};

	// methods
	__device__ vec3 origin() const		{ return o; };
	__device__ vec3 direction() const	{ return d; };
	
	int x, y;	// pixel information

private:
	// variables
	vec3 o;		// origin
	vec3 d;		// direction
};

#endif // !SM_SMATH_H_

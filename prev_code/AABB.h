#ifndef _SM_AABB_H_
#define _SM_AABB_H_

// ******************************************* //
//	
//		AABB.h (Axis-Aligned Bounding Box)
//
//		This code is modified sources by
//		scratch pixel, pbrt
//
// ******************************************* //

#include "ray.h"

class AABB {
public:
	AABB();
	AABB(const vec3 &p);
	AABB(const vec3 &pMin, const vec3 &pMax);

	// operators
	const vec3& operator[] (int i) const;
	vec3& operator[] (int i);
	AABB operator+ (const vec3 &p);
	AABB operator+ (const AABB &b);
	AABB operator- (const AABB &b);

	// methods
	bool intersect(const ray&) const;	// ray intersection
	bool intersect(const ray&, float &tmin) const;
	bool overlaps(const AABB &b) const;
	bool isInside(const vec3 &p) const;
	bool isInsideExclusive(const vec3 &p) const;
	vec3 offset(const vec3 &p) const;

	inline AABB expand(float ds);

	// accessors
	vec3  centroid() const;
	vec3  corner(int corner) const;
	vec3  diagonal() const;
	float surfaceArea() const;
	float volume() const;
	int	  maxExtent() const;

	// variables
	vec3 pMin, pMax;
};



#endif // !_SM_AABB_H_

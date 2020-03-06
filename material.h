#ifndef SM_MATERIAL_H_
#define SM_MATERIAL_H_

/*****************************************************
*
*		material.h
*
*		Stores material data for the object
*
*		- Loading texture/normal files automatically
*		  enables the flag use_texture/normal.
*
******************************************************/

#include "vec3.h"

class material{
public:
	// Integral Light - light planes
	// DR - D/R material
	enum type {INTEGRAL_LIGHT, DR};
	
	// constructors
	__device__ material() {};
	__device__ material(const color &diffuse, int material_type) : cDiffuse(diffuse), material_type(material_type){};

	// getter methods
	__device__ int   matType()	const { return material_type; };
	__device__ color Diffuse()	const { return cDiffuse; };

private:
	color cDiffuse;	// reflective colors
	int	  material_type;
};

#endif // !SM_MATERIAL_H_

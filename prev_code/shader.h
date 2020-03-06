#ifndef SM_SHADER_H_
#define SM_SHADER_H_
/********************************************************************************
*
*		shader.h
*
*		Shader class - controls over computing surface colors.
*		2 types of shading is available:
*			- classical shading (Phong)
*			- d/r shading
*
*		NOTE: SHADING PARAMETERS ARE AVAILABLE BY DEFINE CONSTANTS
*
********************************************************************************/

#include "setting.h"
#include "mesh.h"
#include <map>
#include <algorithm>

class renderer;

class shader {
public:
	static shader* instance();

	// shader setup
	//void register_renderer(renderer *r);

	// main shading methods
	color getColor(const ray &r, const hitrec &hit, const renderer *render) const;
	color getColor_DR(const hitrec &hit, const renderer *raytracer) const;

	// helper method
	vec3 computeSampledPos(int i, int j, const plane &pl, const renderer *raytracer) const;

private:

	// singleton object
	static shader *instanceObj;

	// constructors
	shader();
	~shader();

	shader(shader const&) = delete;
	shader& operator=(shader const&) = delete;

	// pointer to the renderer
	//renderer* raytracer;
};

#endif // !SM_SHADER_H_
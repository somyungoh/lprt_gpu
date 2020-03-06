#include "light.h"


// *** Base Light Class *** //

//constructors
light::light() {};
light::light(const vec3 &center, const color &c, float intensity, int TYPE)
	: o(center), c(c), intensity(intensity), type(TYPE){};
light::~light() {};

// getter & setters
vec3	light::origin()			const { return o; };
color	light::get_color()		const { return c; };
int		light::get_type()		const { return type; };
void	light::set_intensity(float strength) { intensity = strength; };
float	light::get_intensity() const { return 0; };

// *** Point Light *** //

pointLight::pointLight() {};
pointLight::pointLight(const vec3 &center, const color &c, float intensity)
	: light(center, c, intensity, light::POINT) {};
pointLight::~pointLight() {};

float pointLight::get_intensity() const { return intensity; };


// *** Area Light *** //

areaLight::areaLight() {};
areaLight::areaLight(const vec3 &center, const vec3 &normal, const vec3 &up, const color &c, float intensity, float sx, float sy)
	: plane(center, normal, up, material(), sx, sy, -10), light(center, c, intensity, light::AREA) { size = sqrt(sx *sx + sy * sy); };
areaLight::~areaLight() {};

float areaLight::get_intensity() const { return intensity; };
float areaLight::getSize() const { return size; };


// *** D/R Area Light *** //

drAreaLight::drAreaLight() {};
drAreaLight::drAreaLight(const vec3 &center, const vec3 &normal, const vec3 &up, const color &c, float intensity, float sx, float sy)
	: plane(center, normal, up, material(), sx, sy, -10), light(center, c, intensity, light::DRAREA) {};
drAreaLight::~drAreaLight() {};

float drAreaLight::get_intensity(const ray &r) const { 

	// intersection test
	hitqueue hits;
	this->intersection(r, hits);

	if (hits.empty()) return 0;		// 0 energy

	hitrec intersection = hits.top();

	//float u = linearCurve(intersection.u);
	//float v = linearCurve(intersection.v);
	
	float u = cubicBezier(intersection.u);
	float v = cubicBezier(intersection.v);

	//float u = cubicBSpline(intersection.u);
	//float v = cubicBSpline(intersection.v);

	float energy = u * v;

	return energy * intensity;		// light energy X light intensity
};


// *** D/R Disk Light *** //

drDiskLight::drDiskLight() {};
drDiskLight::drDiskLight(const vec3 &center, const vec3 &normal, const vec3 &up, const color &c, float intensity, float r_)
	: plane(center, normal, up, material(), -10), light(center, c, intensity, light::DRAREA), r(r_) {

		// even though it is a infinite plane,
		// if we set a hack a size, it will be used when computing the proportional size
		setSx(r_ * 2);
		setSy(r_ * 2);
	};
drDiskLight::~drDiskLight() {};

float drDiskLight::get_UIntensity(const ray &ray) const { 

	// intersection test
	hitqueue hits;
	this->intersection(ray, hits);

	if (hits.empty()) return 0;		// 0 energy

	hitrec intersection = hits.top();
	
	float u = intersection.u;
	float v = intersection.v;

	float p = (u*u + v*v - r*r) / r*r;
 
	if(p < 0) p = 0;

	float energy = std::pow(M_E, -p);

	return energy * 1 * intensity;		// light energy X light intensity
};

float drDiskLight::get_PIntensity(const ray &ray) const { 

	// intersection test
	hitqueue hits;
	this->intersection(ray, hits);

	if (hits.empty()) return 0;		// 0 energy

	hitrec intersection = hits.top();
	
	float u = intersection.u;
	float v = intersection.v;

	float p = (u*u + v*v) / (r*r);
	if(p < 0) p = 0;

	float energy = std::pow(M_E, -p);

	return energy * 0.5 * intensity;		// light energy X light intensity
};

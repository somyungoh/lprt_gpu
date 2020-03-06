#ifndef SM_LIGHT_H_
#define SM_LIGHT_H_
/**************************************************************
*
*		light.h
*
*		class light
*
**************************************************************/

#include "vec3.h"
#include "shape.h"

// *****	base class of the light		***** //

class light {
public:

	// light type enumeration
	enum TYPE {POINT, AREA, DRAREA};
	
	//constructors
	light();
	light(const vec3 &center, const color &c, float intensity, int TYPE);
	virtual ~light();

	// getter & setters
	vec3	origin()		const;
	color	get_color()		const;
	int		get_type()		const;
	void	set_intensity(float strength);
	virtual float get_intensity() const;
	
protected:
	vec3	o;
	color	c;
	float	intensity;
	int		type;
};



// *****	light subclasses		***** //


//	point light
class pointLight : public light {

public:

	// constructor
	pointLight();
	pointLight(const vec3 &center, const color &c, float intensity);
	~pointLight();

	// get light intensity
	float get_intensity() const override;

};


// area light
class areaLight : public plane, public light{

public:
	// constructor
	areaLight();
	areaLight(const vec3 &center, const vec3 &normal, const vec3 &up, const color &c, float intensity, float sx, float sy);
	~areaLight();

	// get light intensity
	float get_intensity() const override;
	float getSize()	const;

	float size;
};


// d/r area light
class drAreaLight : public light, public plane {

public:
	// constructor
	drAreaLight();
	drAreaLight(const vec3 &center, const vec3 &normal, const vec3 &up, const color &c, float intensity, float sx, float sy);
	~drAreaLight();

	// get light intensity at intersecton point of the plane
	float get_intensity(const ray &r) const;
};


// d/r disk light 
class drDiskLight : public light, public plane {

public:
	// constructor
	drDiskLight();
	drDiskLight(const vec3 &center, const vec3 &normal, const vec3 &up, const color &c, float intensity, float r);
	~drDiskLight();

	// get light intensity at intersecton point of the plane
	float get_UIntensity(const ray &r) const;	// umbra
	float get_PIntensity(const ray &r) const;	// penumbra

private:
	float r;	// radius
};

#endif // !SM_LIGHT_H_

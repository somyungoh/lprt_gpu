#ifndef SM_CAMERA_H_
#define SM_CAMERA_H_
/*****************************************************
*
*		camera.h
*
*		moveable camera class.
*		The camera coordinate system is based on
*		right-handed coordinate.
*		That is forward is +z direction,
*		looking at -z direction.
*
******************************************************/

#include "ray.h"

class camera{
public:
	//construcors
	__device__ camera() {};
	__device__ camera(const vec3 &origin, const vec3 &vup, const vec3 &view, float vertical_fov, int Nx_, int Ny_)
		: origin(origin), vFOV(vertical_fov), lookAt(view), Nx(Nx_), Ny(Ny_){
		
		forward	= normalize(origin - lookAt);					// camera coordinate
		right	= normalize(cross(normalize(vup), forward));
		up		= normalize(cross(forward, right));
		
		aspectR = ((float)Nx / Ny);		// aspect ratio
		fovX = aspectR * tan((vFOV / 2) * (M_PI / 180));
		fovY = tan((vFOV / 2) * (M_PI / 180));
		pixX = (2 * (1 / (float)Nx)) * fovX;
		pixY = (2 * (1 / (float)Ny)) * fovY;
	};


	// camera control
	__device__ void move(const vec3 &p) { 
		
		origin = origin + p;

		forward	 = normalize(origin - lookAt);					// reset camera coordinate
		right	 = normalize(cross(normalize(up), forward));
		up		 = normalize(cross(forward, right)); 
	};

	__device__ void zoom(float f) { vFOV = vFOV + f; };

	__device__ void rotate(float angle) {
		
		vec3 axis(0, 1, 0);
		origin	= origin	* (float)cos(angle * M_PI / 180.f) 
							+ cross(axis, origin) * (float)sin(angle * M_PI / 180.f) 
							+ axis * dot(axis, origin) * (float)(1 - cos(angle * M_PI / 180.f));
		forward = forward	* (float)cos(angle * M_PI / 180.f) 
							+ cross(axis, forward) * (float)sin(angle * M_PI / 180.f) 
							+ axis * dot(axis, forward) * (float)(1 - cos(angle * M_PI / 180.f));
		right	= right		* (float)cos(angle * M_PI / 180.f) 
							+ cross(axis, right) * (float)sin(angle * M_PI / 180.f) 
							+ axis * dot(axis, right) * (float)(1 - cos(angle * M_PI / 180.f));
		up		= up		* (float)cos(angle * M_PI / 180.f) 
							+ cross(axis, up) * (float)sin(angle * M_PI / 180.f) 
							+ axis * dot(axis, up) * (float)(1 - cos(angle * M_PI / 180.f));
	}


	// getters
	__device__ vec3 Origin()  const { return origin; };
	__device__ vec3 Forward() const { return forward; };
	__device__ vec3 Up()	   const { return up; };

	// Precondition: input pixel position of viewport
	// returns ray object that corresponds to input pixel
	__device__ const ray getRay(float x, float y) const{

		ray r(origin, normalize(computeWorldPosAt(x, y)));
		r.x = x;
		r.y = y;

		return r;
	};

	// generates a plane object that corresponds
	// to the viewport plane
	__device__ const plane getPixel(int x, int y) const {

		// compute pixel plane
		return plane(computeWorldPosAt(x, y) + origin, forward, up, material(color(1), 0), pixX, pixY, y * Nx + x);
	};

	float pixX, pixY;		// size of the pixel

private:
	vec3 origin;		// origin
	float vFOV;			// Field of view (vertical)
	float aspectR;		// aspect ratio
	float fovX, fovY;	// aspect ratio - fov
	float Nx, Ny;

	vec3 up;			// camera coordinate system
	vec3 forward;	
	vec3 right;
	vec3 lookAt;

	__device__ vec3 computeWorldPosAt(float x, float y) const {

		// local space
		float ndcX = (x + 0.5f) / (float)Nx;		// ndc (normalized device coordinate) space
		float ndcY = (y + 0.5f) / (float)Ny;
		float cameraX = (2.f * ndcX - 1) * fovX;	// screen space
		float cameraY = (2.f * ndcY - 1) * fovY;
		float cameraZ = -1.f;

		// world space
		vec3 u = right * cameraX;
		vec3 v = up * cameraY;
		vec3 p = u + v - forward;

		return p;
	};
};

#endif // !SM_CAMERA_H_

#ifndef SM_SHAPE_H_
#define SM_SHAPE_H_
/*****************************************************
*
*		shape.h
*
*		Base shape class and collection of shapes:
*		Plane, Sphere, Triangle
*
******************************************************/

#include "ray.h"
#include "material.h"
// #include "bvh.h"
#include "vec3.h"
//#include "hit.h"
#include "hitqueue.h"


class mesh;

class shape {
public:
	//constructors
	__device__ shape();
	__device__ shape(const vec3 &center, const material &m, float size, int TYPE, int ID);
	__device__ virtual ~shape();

	enum TYPE {PLANE, SPHERE, TRIANGLE, RECTANGLE, MESH};

	__device__ int	 getID()	  const { return id; };
	__device__ int	 getType()	  const { return shapeType; };
	__device__ float get_size()	  const { return shapeSize; };
	__device__ vec3	 get_center() const { return center; };
	__device__ material get_mat() const { return mat; }
	//__device__ AABB	 getBounds()	 const { return bounds; };
	__device__ vec3	 Nx() const { return nx; };
	__device__ vec3	 Ny() const { return ny; };
	__device__ vec3	 Nz() const { return nz; };
	__device__ virtual bool  intersection(const ray &ray, hitqueue &hits) const { return 0; };
	__device__ virtual void	 computeUV(const vec3 &intersection_point, float &u, float &v) const {};
	__device__ void	 rotate(float angle, const vec3 &axis, float dt) {

		// rotate coordinates based on rodrigues formula
		float aRad = angle * M_PI / 180.f;
		nx = nx * cos(aRad) + cross(axis, nx) * sin(aRad) + axis * dot(axis, nx) * (1 - cos(aRad));
		ny = ny * cos(aRad) + cross(axis, ny) * sin(aRad) + axis * dot(axis, ny) * (1 - cos(aRad));
		nz = nz * cos(aRad) + cross(axis, nz) * sin(aRad) + axis * dot(axis, nz) * (1 - cos(aRad));
	};
	
protected:
	int		id;
	int		shapeType;
	float	shapeSize;
	vec3	center;			// center
	vec3	nx, ny, nz;		// local coordinates (x, y, z)
	material mat;
	//AABB	bounds;
};


// ----------	Plane	-------------//

class plane : public shape {
public:
	//constructors
	__device__ plane() {};
	__device__ plane(const vec3 &center, const vec3 &normal, const vec3 &up, const material &m, int ID)
		: shape(center, m, 0, PLANE, ID), n(normalize(normal)){
			
			// compute normal
			ny = up;
			nz = normal;
			nx = cross(ny, nz);

			sx = INFINITY;
			sy = INFINITY;
				
			// compute bounding box

			vec3 p1 = center - nx * sx * 0.5f - ny * sy * 0.5f - nz * 0.0001f;
			vec3 p2 = center + nx * sx * 0.5f + ny * sy * 0.5f + nz * 0.0001f;
			// bounds = AABB(
			// 	vec3(std::min(p1.x, p2.x), std::min(p1.y, p2.y), std::min(p1.z, p2.z)),
			// 	vec3(std::max(p1.x, p2.x), std::max(p1.y, p2.y), std::max(p1.z, p2.z))
			// );
		};
	__device__ plane(const vec3 &center, const vec3 &normal, const vec3 &up, const material &m, float sx_, float sy_, int ID)
		: shape(center, m, sqrt(sx_ * sx_ + sy_ * sy_), RECTANGLE, ID), n(normalize(normal)), sx(sx_), sy(sy_) {
		
		// compute normal
		ny = up;
		nz = normal;
		nx = cross(ny, nz);

		// compute bounding box

		vec3 p1 = center - nx * sx * 0.5f - ny * sy * 0.5f - nz * 0.0001f;
		vec3 p2 = center + nx * sx * 0.5f + ny * sy * 0.5f + nz * 0.0001f;
		// bounds = AABB(
		// 	vec3(std::min(p1.x, p2.x), std::min(p1.y, p2.y), std::min(p1.z, p2.z)),
		// 	vec3(std::max(p1.x, p2.x), std::max(p1.y, p2.y), std::max(p1.z, p2.z))
		// );
	};

	__device__ vec3  normal() const { return n; };
	__device__ void  setSx(float sx_) { sx = sx_; shapeSize = sqrt(sx * sx + sy * sy); };
	__device__ void  setSy(float sy_) { sy = sy_; shapeSize = sqrt(sx * sx + sy * sy); };
	__device__ float Sx()	  const { return sx; };
	__device__ float Sy()	  const { return sy; };

	__device__ bool intersection(const ray &ray, hitqueue &hits) const {

		// check intersection
		vec3 intersection_point;

		vec3 oc = center - ray.origin();
		float dotNL = dot(ray.direction(), n);

		// compute t and intersection point
		float t = dot(oc, n) / dotNL;
		intersection_point = ray.origin() + ray.direction() * t;

		if (shapeType == PLANE) {
			if (t > 1e-6) {

				float u, v;

				computeUV(intersection_point, u, v);

				hits.push(hitrec(t, (shape*)this, u, v, intersection_point, n));
				return true;
			}
		}
		else if (shapeType == RECTANGLE) {

			vec3 projected_vector = intersection_point - center;

			float dotPnX = dot(projected_vector, nx);
			float dotPnY = dot(projected_vector, ny);
			
			if (t > EPSILON 
				&& dotPnX >= -0.5f * sx && dotPnX < sx * 0.5f
				&& dotPnY >= -0.5f * sy && dotPnY < sy * 0.5f) {

				float u = 0, v = 0;

				computeUV(intersection_point, u, v);

				hits.push(hitrec(t, this, u, v, intersection_point, n));
				return true;
			}
		}

		return false;
	};

	__device__ void computeUV(const vec3 &intersection_point, float &u, float &v) const {
		// compute u,v
		if (shapeType == PLANE) {
			float x = dot(center - intersection_point, Nx());
			float y = dot(center - intersection_point, Ny());

			u = x;
			v = y;

			// u = x - (int)x;
			// v = y - (int)y;
			// if (u < 0) u += 1;
			// if (v < 0) v += 1;
		}
		else if (shapeType == RECTANGLE) {
			// compute u,v
			float x = dot(intersection_point - center, Nx());
			float y = dot(intersection_point - center, Ny());
			u = x / Sx() + 0.5;		// add 0.5 cuz we pivot is the center of the plane
			v = y / Sy() + 0.5;
		}
	};

private:
	vec3 n;
	float sx, sy;
};


// ----------	Sphere	-------------//

class sphere : public shape {
public:
	// constructors
	__device__ sphere() {};
	__device__ sphere(const vec3 &center, float r, const material &m, int ID) : sphere(center, r, m, vec3(1, 0, 0), vec3(0, 1, 0), ID) { };
	__device__ sphere(const vec3 &center, float r, const material &m, const vec3 &right, const vec3 &up, int ID)
		: shape(center, m, 2 * r, SPHERE, ID), r(r) {
		
		// compute normal
		nx = right;
		ny = up;
		nz = cross(nx, ny);
		shapeSize = 2 * r;

		// compute bounding box
		//bounds = AABB(center - r, center + r);
	};

	__device__ bool intersection(const ray &ray, hitqueue &hits) const {
		vec3 oc = ray.origin() - center;				 // sphere center -> ray origin
														// solving quadric equation for t
		//float a = dot(ray.direction(), ray.direction()); //since we use normalized, its always 1
		//float b = 2 * dot(oc, ray.direction());
		//float c = dot(oc, oc) - r * r;
		//float D = b * b - 4 * a*c;	// Determinant

		vec3 intersection_point;

		float b = dot(oc, ray.direction());
		float c = dot(oc, oc) - r * r;
		float h = b * b - c;
		
		if (h < 0.0) return false; // no intersection
		h = sqrt(h);
		
		float t_min = -b - h;
		float t_max = -b + h;

		// compute stuff
		if (t_max < 0) return false;
		float t = t_max;
		
		vec3 normal;
		float u, v;

		intersection_point = ray.origin() + ray.direction() * t;
		normal = normalize(intersection_point - center);
		computeUV(intersection_point, u, v);
		u = 1 - u;	// needs to be flipped
		v = 1 - v;

		// clamping
		u = u < 0 ? -u : 1 - fmod(u, 1.f);
		v = v < 0 ? -v : 1 - fmod(v, 1.f);
		
		hits.push(hitrec(t, this, u, v, intersection_point, normal));

		// if both positive, insert one more hit.
		if (t_min > 0) {

			t = t_min;

			intersection_point = ray.origin() + ray.direction() * t;
			normal = normalize(intersection_point - center);
			computeUV(intersection_point, u, v);
			u = 1 - u;	// needs to be flipped
			v = 1 - v;

			// clamping
			u = u < 0 ? -u : 1 - fmod(u, 1.f);
			v = v < 0 ? -v : 1 - fmod(v, 1.f);

			hits.push(hitrec(t, this, u, v, intersection_point, normal));
		}
		return true;
	};

	__device__ void computeUV(const vec3 &intersection_point, float &u, float &v) const {

		vec3 N = normalize(intersection_point - center);
		float cospi = dot(Ny(), N);
		float xx = dot(Nz(), N);
		float yy = dot(Nx(), N);
		float cos2PI = yy / sin(acos(cospi));

		u = acos(cos2PI) / (2 * M_PI);
		v = acos(cospi) / M_PI;
		u *= 2;
	};

	__device__ float radius() const { return r; };

private:
	float r;			//radius
};


// ----------	Triangle	-------------//

// class triangle :public shape {
// public:
// 	// constructors
// 	triangle();
// 	triangle(int v1, int v2, int v3, const material &m, mesh *parent, int ID);	// points in CCW  
// 	~triangle();

// 	void setNormal(int n0, int n1, int n2);
// 	void setUV(int uv0, int uv1, int uv2);
// 	//void setParent(mesh *parent);

// 	// getters
// 	bool intersection(const ray &ray, hitqueue &hits) const override;
// 	void computeUV(const vec3 &intersection_point, float &u, float &v) const override;

// private:
// 	// ID to the parent mesh's array
// 	int v[3];	// vertex
// 	int n[3];	// normal
// 	int uv[3];	// uv

// 	mesh   *parent;	// if triangle is part of the mesh
// 	vec3	normal;
// 	float   A;
// };


#endif // !SM_SHAPE_H_
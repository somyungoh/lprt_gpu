#ifndef SM_HIT_H_
#define SM_HIT_H_

#include "vec3.h"
// #include <queue>

class shape;

// struct for recording hits
struct hitrec {
	const shape* object = nullptr;
	float t;
	float u, v;
	float w;	// weight
	vec3  hitP;
	vec3  normal;

	__device__ hitrec() {};
	__device__ hitrec(float _t, const shape *obj, float u, float v, const vec3 &hitP, const vec3 &normal)
		: t(_t), object(obj), u(u), v(v), hitP(hitP), normal(normal) {};
};

// compare helper
struct compareHitRec {
	bool operator ()(const hitrec &h1, const hitrec &h2) const {
		if (h2.t < h1.t) return true;
		else return false;
	}
};


// this is for storing all the hit objects from raycast
// typedef std::priority_queue<hitrec, std::vector<hitrec>, compareHitRec> hitqueue;

#endif // !SM_HIT_H_

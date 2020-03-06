#include "AABB.h"

AABB::AABB() {
	float minNum = std::numeric_limits<float>::lowest();
	float maxNum = std::numeric_limits<float>::max();
	pMin = vec3(maxNum);
	pMax = vec3(minNum);
};
AABB::AABB(const vec3 &p) : pMin(p), pMax(p) {};
AABB::AABB(const vec3 &pMin_, const vec3 &pMax_) : pMin(pMin_), pMax(pMax_) {};
vec3 AABB::centroid() const { return (pMin + pMax) * 0.5f; };
vec3 AABB::corner(int corner) const { 
	return vec3((*this)[corner & 1].x, 
				(*this)[(corner & 2) ? 1 : 0].y, 
				(*this)[(corner & 4) ? 1 : 0].z
				);
};

bool AABB::intersect(const ray &r) const {
	float x;
	return intersect(r, x);
}

bool AABB::intersect(const ray &r, float &t) const
{
	const vec3 sign(r.direction().x < 0, r.direction().y < 0, r.direction().z < 0);

	const vec3 orig = r.origin();
	const vec3 invDir = 1.f / r.direction();
	float tmin, tmax, tymin, tymax, tzmin, tzmax;

	tmin  = ((*this)[(bool)sign[0]].x - orig.x) * invDir.x;
	tmax  = ((*this)[1 - (bool)sign[0]].x - orig.x) * invDir.x;
	tymin = ((*this)[(bool)sign[1]].y - orig.y) * invDir.y;
	tymax = ((*this)[1 - (bool)sign[1]].y - orig.y) * invDir.y;

	if ((tmin > tymax) || (tymin > tmax))
		return false;

	if (tymin > tmin)
		tmin = tymin;
	if (tymax < tmax)
		tmax = tymax;

	tzmin = ((*this)[(bool)sign[2]].z - orig.z) * invDir.z;
	tzmax = ((*this)[1 - (bool)sign[2]].z - orig.z) * invDir.z;

	if ((tmin > tzmax) || (tzmin > tmax))
		return false;

	if (tzmin > tmin)
		tmin = tzmin;
	if (tzmax < tmax)
		tmax = tzmax;

	t = tmin;

	return true;
}

// access pMin/pMax by index
const vec3& AABB::operator[] (int i) const { return i > 0 ? pMax : pMin; };
vec3& AABB::operator[] (int i) { return i > 0 ? pMax : pMin; };

// union(expand) with input point
AABB AABB::operator+ (const vec3 &p) {

	vec3 newMin(
		std::min(this->pMin.x, p.x),
		std::min(this->pMin.y, p.y),
		std::min(this->pMin.z, p.z)
	);

	vec3 newMax(
		std::max(this->pMax.x, p.x),
		std::max(this->pMax.y, p.y),
		std::max(this->pMax.z, p.z)
	);

	return AABB(newMin, newMax);
};

// union(expand) with another bounding box
AABB AABB::operator+ (const AABB &b) {

	vec3 newMin(
		std::min(this->pMin.x, b.pMin.x),
		std::min(this->pMin.y, b.pMin.y),
		std::min(this->pMin.z, b.pMin.z)
	);

	vec3 newMax(
		std::max(this->pMax.x, b.pMax.x),
		std::max(this->pMax.y, b.pMax.y),
		std::max(this->pMax.z, b.pMax.z)
	);

	return AABB(newMin, newMax);
}

// intersect(shrink) with another bounding box
AABB AABB::operator- (const AABB &b) {

	vec3 newMin(
		std::max(this->pMin.x, b.pMin.x),
		std::max(this->pMin.y, b.pMin.y),
		std::max(this->pMin.z, b.pMin.z)
	);

	vec3 newMax(
		std::min(this->pMax.x, b.pMax.x),
		std::min(this->pMax.y, b.pMax.y),
		std::min(this->pMax.z, b.pMax.z)
	);

	return AABB(newMin, newMax);
}

bool AABB::overlaps(const AABB &b) const {
	bool x = (this->pMax.x >= b.pMin.x) && (this->pMin.x <= b.pMax.x);
	bool y = (this->pMax.y >= b.pMin.y) && (this->pMin.x <= b.pMax.y);
	bool z = (this->pMax.z >= b.pMin.z) && (this->pMin.x <= b.pMax.z);

	return (x && y && z);
};

bool AABB::isInside(const vec3 &p) const {
	return (p.x >= this->pMin.x && p.x <= this->pMax.x &&
			p.y >= this->pMin.y && p.y <= this->pMax.y &&
			p.z >= this->pMin.z && p.z <= this->pMax.z );
};

bool AABB::isInsideExclusive(const vec3 &p) const {
	return (p.x >= this->pMin.x && p.x < this->pMax.x &&
			p.y >= this->pMin.y && p.y < this->pMax.y &&
			p.z >= this->pMin.z && p.z < this->pMax.z);
};

AABB AABB::expand(float ds){
		return AABB(this->pMin - vec3(ds), this->pMax + vec3(ds));
}

vec3 AABB::offset(const vec3 &p) const {
	vec3 o = p - pMin;
	if (pMax.x > pMin.x) o.x /= pMax.x - pMin.x;
	if (pMax.y > pMin.y) o.y /= pMax.y - pMin.y;
	if (pMax.z > pMin.z) o.z /= pMax.z - pMin.z;

	return o;
}


vec3  AABB::diagonal() const { return pMax - pMin; };

float AABB::surfaceArea() const {
	vec3 d = diagonal();
	return 2 * (d.x * d.y + d.x * d.z + d.y * d.z);
};

float AABB::volume() const {
	vec3 d = diagonal();
	return d.x * d.y * d.z;
};

// returns index of the longest axis
int	  AABB::maxExtent() const {
	vec3 d = diagonal();

	if (d.x > d.y && d.x > d.z)
		return 0;
	else if (d.y > d.z)
		return 1;
	else
		return 2;
};
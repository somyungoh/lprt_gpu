#ifndef _SM_BVH_H_
#define _SM_BVH_H_

/*************************************************************************
*
*		bvh.h
*
*		Implementation of Bounding Volume Hierarchies (BVH)
*		acceleration structure. This is primitive based
*		partition and consists 3 different partition algorithm - 
*		midpoint, equal subset, surface area heuristic (sah).
*
*		This code referenced and modified the book "Physically Based
*		Rendering" chaper4.3, Bounding Volume Hierarchies.
*		https://www.pbrt.org/
*
**************************************************************************/

#include <vector>
#include "AABB.h"
#include "ray.h"
#include "hit.h"

class shape;
class BVHAccel {

	struct shapeInfo {

		shapeInfo() {};
		shapeInfo(size_t shapeNum, const AABB &bounds)
			: shapeNum(shapeNum), bounds(bounds), centroid((bounds.pMin + bounds.pMax) * 0.5f) {};
		
		size_t shapeNum;
		AABB bounds;
		vec3 centroid;
	};

	// node structure for BVH build
	struct BVHBuildNode {
		
		void initLeaf(int first, int n, const AABB &b) {
			firstShapeOffset = first;
			nShapes = n;
			bounds = b;
			children[0] = children[1] = nullptr;	// leaf node is determined by nullptr
		}

		void initInterior(int axis, BVHBuildNode *c0, BVHBuildNode *c1) {
			children[0] = c0;
			children[1] = c1;
			bounds = c0->bounds + c1->bounds;
			splitAxis = axis;
			nShapes = 0;
		}

		AABB bounds;
		BVHBuildNode *children[2];
		int splitAxis, firstShapeOffset, nShapes;
	};

	struct LinearBVHNode {

		AABB bounds;
		union 
		{
			int shapesOffset;		// leaf
			int secondChildOffset;	// interior
		};
		uint16_t nShapes;		// 0 -> interior nodes
		uint8_t  axis;			// interior node : xyz
		uint8_t pad[1];			// ensure 32 byte total size
	};
	
	struct BucketInfo {
		int count = 0;
		AABB bounds;
	};

public:

	enum partitionType { MIDPOINT, EQUALSUBSET, SAH };

	//constructor
	BVHAccel();
	BVHAccel(const std::vector<shape*> &shapes, int maxShapesInNode, int partitionType);
	~BVHAccel();

	// building methods
	BVHBuildNode* recursiveBuild(std::vector<shapeInfo> &shapeInfo,
		int start, int end, int *totalNodes, std::vector<shape*> &orderedShapes);

	// traversal methods
	bool intersect(const ray &ray, hitqueue &hits) const;
	bool isEmpty() const;
	
private:
	int maxShapesInNode;
	int PARTITION;
	std::vector<shape*> shapes;
	LinearBVHNode *nodes = nullptr;
	
	int flattenBVHTree(BVHBuildNode *node, int *offset);
};

#endif
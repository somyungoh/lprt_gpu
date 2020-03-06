#include "bvh.h"
#include "shape.h"

BVHAccel::BVHAccel() {};
BVHAccel::BVHAccel(const std::vector<shape*> &shapes, int maxShapesInNode, int partitionType)
	: shapes(shapes), maxShapesInNode(std::min(255, maxShapesInNode)), PARTITION(partitionType){

	if (shapes.size() == 0) return;
	
	// -------		Build BVH		------ //
	
	// 1. initialize primitive info
	std::vector<shapeInfo> shapeInfo(shapes.size());
	for (size_t i = 0; i < shapeInfo.size(); i++) { shapeInfo[i] = { i, shapes[i]->getBounds() }; };

	// 2. build BVH tree
	int totalNodes = 0;
	std::vector<shape*> orderedShapes;
	orderedShapes.reserve(shapes.size());
	BVHBuildNode *root = recursiveBuild(shapeInfo, 0, shapes.size(), &totalNodes, orderedShapes);
	this->shapes.swap(orderedShapes);
	shapeInfo.resize(0);
	
	// 3. compute representation of depth-first traversal
	nodes = new LinearBVHNode[totalNodes];
	int offset = 0;
	flattenBVHTree(root, &offset);
};
BVHAccel::~BVHAccel() {};


// building method
BVHAccel::BVHBuildNode* BVHAccel::recursiveBuild(
	std::vector<shapeInfo> &bvHShapeInfo, int start, int end, int *totalNodes,
	std::vector<shape*> &orderedShapes) {

	// create node
	BVHBuildNode *node = new BVHBuildNode();
	(*totalNodes)++;
	
	// compute bounds for all shapes in BVH node
	AABB topBound;
	for (int i = start; i < end; i++) 
		topBound = topBound + bvHShapeInfo[i].bounds;
	
	int nShapes = end - start;

	if (nShapes == 1) {
		// create leaf node

		int firstPrimOffset = orderedShapes.size();
		for (int i = start; i < end; i++) {
			int shapeNum = bvHShapeInfo[i].shapeNum;
			orderedShapes.push_back(shapes[shapeNum]);
		}
		node->initLeaf(firstPrimOffset, nShapes, topBound);
		return node;
	}
	else {
		// compute bound of shape centroids, choose split dimension dim
		// the split axis is chosen by axis with the largest extent
		AABB centroidBounds;
		for (int i = start; i < end; i++) {
			centroidBounds = centroidBounds + bvHShapeInfo[i].centroid;
		}
		int dim = centroidBounds.maxExtent();

		// partition shapes into two sets and build children
		int mid = (start + end) / 2;
		if (centroidBounds.pMax[dim] == centroidBounds.pMin[dim]) {
			// create leaf node
			int firstShapeOffset = orderedShapes.size();
			for (int i = start; i < end; i++) {
				int shapeNum = bvHShapeInfo[i].shapeNum;
				orderedShapes.push_back(shapes[shapeNum]);
			}
			node->initLeaf(firstShapeOffset, nShapes, topBound);
			return node;
		}
		else {
			// partition by method
			switch (PARTITION) {

				// partition shapes using midpoints
			case MIDPOINT: {
				float pmid = (centroidBounds.pMin[dim] + centroidBounds.pMax[dim]) / 2;
				shapeInfo *midPtr =
					std::partition(&bvHShapeInfo[start], &bvHShapeInfo[end - 1] + 1, [dim, pmid](const shapeInfo &pi) { return pi.centroid[dim] < pmid; });
				mid = midPtr - &bvHShapeInfo[0];

				// if there is too many overlapping boxes, it may fail to construct.
				// in that case, we split using equally subset method.
				if (mid != start && mid != end) break;
			}
			case EQUALSUBSET: {
				mid = (start + end) / 2;
				std::nth_element(&bvHShapeInfo[start], &bvHShapeInfo[mid], &bvHShapeInfo[end - 1] + 1,
					[dim](const shapeInfo &a, const shapeInfo &b) { return a.centroid[dim] < b.centroid[dim]; }
				);
				break;
			}
			case SAH:
			default: {
				if (nShapes <= 2) {
					// partition primitives into equally sized subsets
					mid = (start + end) / 2;
					std::nth_element(&bvHShapeInfo[start], &bvHShapeInfo[mid], &bvHShapeInfo[end - 1] + 1,
						[dim](const shapeInfo &a, const shapeInfo &b) { return a.centroid[dim] < b.centroid[dim]; });
				}
				else {
					// allocate BucketInfo for SAH partition buckets
					constexpr int nBuckets = 12;
					BucketInfo buckets[nBuckets];

					// init. BucketInfo for SAH partition buckets
					for (int i = start; i < end; i++) {
						int b = nBuckets * centroidBounds.offset(bvHShapeInfo[i].centroid)[dim];
						
						if (b == nBuckets) b = nBuckets - 1;
						buckets[b].count++;
						buckets[b].bounds = buckets[b].bounds + bvHShapeInfo[i].bounds;
					}

					// Compute costs for splitting after each bucket
					float cost[nBuckets - 1];
					for (int i = 0; i < nBuckets - 1; i++) {
						AABB b0, b1;
						int count0 = 0, count1 = 0;
						for (int j = 0; j <= i; j++) {
							b0 = b0 + buckets[j].bounds;
							count0 += buckets[j].count;
						}
						for (int j = i + 1; j < nBuckets; j++) {
							b1 = b1 + buckets[j].bounds;
							count1 += buckets[j].count;
						}
						cost[i] = 1 + (count0 * b0.surfaceArea() + count1 * b1.surfaceArea()) / topBound.surfaceArea();
					}

					// Find bucket to split at that minimizes SAH metric
					float minCost = cost[0];
					int minCostSplitBucket = 0;
					for (int i = 1; i < nBuckets - 1; i++) {
						if(cost[i] < minCost){
							minCost = cost[i];
							minCostSplitBucket = i;
						}
					}

					// Either create leaf or split primitives at selected SAH bucket
					float leafCost = nShapes;
					if (nShapes > maxShapesInNode || minCost < leafCost) {
						shapeInfo *pmid = std::partition(&bvHShapeInfo[start], &bvHShapeInfo[end - 1] + 1, 
							[=](const shapeInfo &pi) {
								int b = nBuckets * centroidBounds.offset(pi.centroid)[dim]; 
								if (b == nBuckets) b = nBuckets - 1;
								return b <= minCostSplitBucket; });
						mid = pmid - &bvHShapeInfo[0];
					}
					// create leaf BVHBuild node
					else {
						int firstPrimOffset = orderedShapes.size();
						for (int i = start; i < end; i++) {
							int shapeNum = bvHShapeInfo[i].shapeNum;
							orderedShapes.push_back(shapes[shapeNum]);
						}
						node->initLeaf(firstPrimOffset, nShapes, topBound);
						return node;
					}
				}
				break;
			}
			}

			// build nodes
			node->initInterior(dim,
				recursiveBuild(bvHShapeInfo, start, mid, totalNodes, orderedShapes),
				recursiveBuild(bvHShapeInfo, mid, end, totalNodes, orderedShapes));
		}
	}

	return node;
};


// this method converts BVH tree into compact structure
int BVHAccel::flattenBVHTree(BVHBuildNode *node, int *offset) {

	LinearBVHNode *linearNode = &nodes[*offset];
	linearNode->bounds = node->bounds;
	int myOffset = (*offset)++;
	
	if (node->nShapes > 0) {
		linearNode->shapesOffset = node->firstShapeOffset;
		linearNode->nShapes = node->nShapes;
	}
	else {
		// create interior flattened BVH node
		linearNode->axis = node->splitAxis;
		linearNode->nShapes = 0;
		flattenBVHTree(node->children[0], offset);
		linearNode->secondChildOffset = flattenBVHTree(node->children[1], offset);
	}

	return myOffset;
}


// traversal method
bool BVHAccel::intersect(const ray &r, hitqueue &hits) const {

	bool intersection = false;
	
	// follow ray through BVH nodes to find primitive intersections
	int toVisitOffset = 0;
	int currentNodeIndex = 0;
	int nodesToVisit[64];

	while (true) {
		const LinearBVHNode *node = &nodes[currentNodeIndex];

		// check ray against BVH node
		if (node->bounds.intersect(r)) {
			// intersetct ray with primitives in leaf BVH node
			if (node->nShapes > 0) {
				// intersect ray with primitives in leaf BVH node
				for (int i = 0; i < node->nShapes; i++) {
					if (shapes[node->shapesOffset + i]->intersection(r, hits))
						intersection = true;
				}
				if (toVisitOffset == 0) break;
				currentNodeIndex = nodesToVisit[--toVisitOffset];
			}
			else {
				// put far BVH node on nodesToVisit stack, advance to near node
				const vec3 invDir = 1.f / r.direction();
				int dirIsNeg[3] = { invDir.x < 0, invDir.y < 0, invDir.z < 0 };

				if (dirIsNeg[node->axis]) {
					nodesToVisit[toVisitOffset++] = currentNodeIndex + 1;
					currentNodeIndex = node->secondChildOffset;
				}
				else {
					nodesToVisit[toVisitOffset++] = node->secondChildOffset;
					currentNodeIndex = currentNodeIndex + 1;

				}
			}
		}
		else {
			if (toVisitOffset == 0) break;
			currentNodeIndex = nodesToVisit[--toVisitOffset];
		}
	}
	return intersection;
}

bool BVHAccel::isEmpty() const {
	
	if(nodes == nullptr)
		return true;
	return false;
}
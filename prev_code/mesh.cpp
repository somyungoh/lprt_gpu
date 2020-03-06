#include "mesh.h"

using namespace std;

// constructors
mesh::mesh() {};
mesh::mesh(const vec3 &center, const material &m, int ID) : shape(center, m, 0, MESH, ID) {};
mesh::~mesh() {};

void mesh::translate(const vec3 &tx, const vec3 &ty, const vec3 &tz) {};
void mesh::rotate(float angle, const vec3 &axis, float dt) {
};

// this intersection is called 
// when it is not using acceleration method
bool mesh::intersection(const ray &ray, hitqueue &hits) const {
	
	int intersection = false;

	// bounding box
	if (!bounds.intersect(ray)) return false;
	
	// raycast using bvh acceleration
	if (!bvhTree.isEmpty()) { 
		bvhTree.intersect(ray, hits); 
	}
	// ray cast all triangles (brute-force)
	else {
		for (shape* face : faces) {
			intersection += face->intersection(ray, hits);
		}
	}
	return intersection;
};

/*************************************************************************************
*
*	Object Loader
*
*	Implementation of a object loading funtion.
*
*	this code is written from
*	http://www.opengl-tutorial.org/beginners-tutorials/tutorial-7-model-loading/
*
**************************************************************************************/
bool mesh::load_objFile(const std::string &filename) {

	std::vector<int>  vertexIndices, uvIndices, normalIndices;
	
	FILE *file = fopen(filename.c_str(), "r");	// open file
	vec3 centerPoint(0, 0, 0);

	float minX, minY, minZ;	// variable for determining size
	float maxX, maxY, maxZ;
	minX = minY = minZ = 10000;
	maxX = maxY = maxZ = -10000;

	if (file == NULL) {		// check failure
		std::cout << "OBJLOAD::File loading failed." << endl;
		return false;
	}
	std::cout << "OBJLOAD::Start loading OBJ: " << filename << endl;

	while (1) {			// loop until the end of the file
		
		// 1. read header

		char lineHeader[128];		// header, such as v, vt, vn ....
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)				// stop reading if EOF
			break;

		// 2. parse with headers

		// v - vertex
		if (strcmp(lineHeader, "v") == 0) {
			vec3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			vertices.push_back(vertex);
			centerPoint += vertex;		// add up vertex points

			minX = std::min(minX, vertex.x);
			minY = std::min(minY, vertex.y);
			minZ = std::min(minZ, vertex.z);
			maxX = std::max(maxX, vertex.x);
			maxY = std::max(maxY, vertex.y);
			maxZ = std::max(maxZ, vertex.z);
		}
		// vt - UV texture position
		else if (strcmp(lineHeader, "vt") == 0) {
			vec3 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y);
			uvs.push_back(uv);
		}
		// vn - vertex normal
		else if (strcmp(lineHeader, "vn") == 0) {
			vec3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			normals.push_back(normal);
		}
		// f - faces
		else if (strcmp(lineHeader, "f") == 0) {
			int v_index[3], uv_index[3], n_index[3];

			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n",
				&v_index[0], &uv_index[0], &n_index[0],
				&v_index[1], &uv_index[1], &n_index[1],
				&v_index[2], &uv_index[2], &n_index[2]);
			
			if (matches != 9) {				// file format did not match to the reader
				printf("OBJLOAD::Face format - v/vt/n did not match.\n");
				return false;
			}

			vertexIndices.push_back(v_index[0] - 1);		// - 1, because obj file starts index with 1.
			vertexIndices.push_back(v_index[1] - 1);
			vertexIndices.push_back(v_index[2] - 1);
			uvIndices.push_back(uv_index[0] - 1);
			uvIndices.push_back(uv_index[1] - 1);
			uvIndices.push_back(uv_index[2] - 1);
			normalIndices.push_back(n_index[0] - 1);
			normalIndices.push_back(n_index[1] - 1);
			normalIndices.push_back(n_index[2] - 1);
		}

	}	// end of while() - reading file

	// generate triangles
	for (int i = 0; i < vertexIndices.size() / 3; i++) {
	
		triangle* face;
		face = new triangle(vertexIndices[i * 3 + 0], vertexIndices[i * 3 + 1], vertexIndices[i * 3 + 2], mat, this, i + id);
		face->setNormal(normalIndices[i * 3 + 0], normalIndices[i * 3 + 1], normalIndices[i * 3 + 2]);
		face->setUV(uvIndices[i * 3 + 0], uvIndices[i * 3 + 1], uvIndices[i * 3 + 2]);
	
		faces.push_back(face);
	}
	
	// compute meta info.
	vec3 minP	= vec3(minX, minY, minZ);
	vec3 maxP	= vec3(maxX, maxY, maxZ);
	center		= centerPoint / (float)vertices.size();	// evaluate center point
	shapeSize	= glm::distance(minP, maxP);			// evaluate object size
	bounds		= AABB(minP, maxP);						// generate bounding box

	std::cout << "OBJLOAD::OBJ Load Complete.\tvertices: " << vertices.size() << "\tfaces: " << faces.size() << endl;
};


/*************************************************************************************
*
*	Locater Loader
*
*	Load locater position that is placed in the estimate of that object/
*
**************************************************************************************/

bool mesh::load_locaterFile(const std::string &filename){

	FILE *file = fopen(filename.c_str(), "r");	// open file

	if (file == NULL) {		// check failure
		std::cout << "LOCATERLOAD::File loading failed." << endl;
		return false;
	}

	std::cout << "OBJLOAD::Start loading Locater: " << filename << endl;
	
	// loop until the end of the file
	while (1) {			
		vec3 location;
		int res = fscanf(file, "%f,%f,%f\n", &location.x, &location.y, &location.z);
		
		// stop reading if EOF
		if (res == EOF)	
			break;
		
		// save it
		locaterP.push_back(location);
	}

	std::cout << "LOCATERLOAD::File load success" << std::endl;
	return true;
}

// build BVHTree with input
void mesh::buildBVHTree(int maxShapesInNode, int partitionType) {
	bvhTree = BVHAccel(faces, maxShapesInNode, partitionType);
};
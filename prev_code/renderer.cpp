#include "renderer.h"
#include "shader.h"

/************************************************************************
*
*		renderer.cpp
*
*		main class that controls overall rendering process
*
**************************************************************************/
#define STRATIFIED_SAMPLE

//**************************************//
//				constructors			//
//**************************************//
renderer::renderer() {};
renderer::renderer(int win_w, int win_h, const Setting &setting)
	: winW(win_w), winH(win_h), setting(setting) {
	
	render_map		= new float[winW * winH * 3];
	anti_aliasing	= false;
	sample_index	= 0;

	set_setting(setting);
	
	scene_ = std::make_shared<scene>();
	scene_->init_scene(setting);
	
	// generate shader
	//shader::instance()->register_renderer(this);

	// build bvh tree
	if (setting.BVHAccleration) buildBVH();
};
renderer::~renderer() {}


//**************************************//
//		THE MAIN rendering method		//
//**************************************//

void renderer::render() {

	// log render time
	using Time = std::chrono::high_resolution_clock;
	using fsec = std::chrono::duration<float>;
	auto t0 = Time::now();
	
	for (int y = 0; y < winH; y++) {
#pragma omp parallel for
		for (int x = 0; x < winW; x++) {
			
			color pixel_color(0);		// color of current pixel

			for (int i = 0; i < 9; i++) {	// x9 multi-sampling

				//	generate ray	//
				float xf = x;
				float yf = y;
				samplePos(xf, yf, i);			// grab sampling position
				
				// ********     R   A   Y   C   A   S   T     ******** //
				ray	r	= scene_->cam.getRay(xf, yf);	// get ray from world coordinate
				color c = raycast(r);					

				if (!anti_aliasing) { pixel_color = c; break; }
				else pixel_color += c / 9.0f;

				sample_index = i;
			}

			// copy to render map
			render_map[(y * winW + x) * 3 + 0] = pixel_color.r;
			render_map[(y * winW + x) * 3 + 1] = pixel_color.g;
			render_map[(y * winW + x) * 3 + 2] = pixel_color.b;
		}
		if (setting.printLog) printf("Render:: render progress %.3f%% \n", ((float)y/winH) * 100.f);
	}

	// log render time
	auto t1 = Time::now();
	fsec fs = t1 - t0;
	printf("Render:: render complete. Time - %.5fs \n", fs.count());
};


//**************************************//
//		 render control methods			//
//**************************************//

int renderer::changeSampleNum(int di){
	setting.SampleNum += di;
	setting.SampleNum = setting.SampleNum < 1 ? 1 : setting.SampleNum;
	return setting.SampleNum;
}

float renderer::changeP(float df){
	setting.P += df;
	return setting.P;
}

float renderer::changeS(float ds){
	setting.S += ds;
	return setting.S;
}

float renderer::changeBrightness(float dt){
	setting.brightness += dt;
	setting.brightness = setting.brightness < 0 ? 0 : setting.brightness;
	return setting.brightness;
}

void renderer::changePLPO(float dt){
	setting.t_plpo += dt;
	setting.t_plpo = setting.t_plpo < 0 ? 0 : setting.t_plpo;
	setting.t_plpo = setting.t_plpo > 1 ? 1 : setting.t_plpo;
	printf("RENDERER::t_plpo change: %.3f\n", setting.t_plpo);
}

void renderer::changePrj(float dt){
	setting.t_prj += dt;
	setting.t_prj = setting.t_prj < 0 ? 0 : setting.t_prj;
	setting.t_prj = setting.t_prj > 1 ? 1 : setting.t_prj;
	printf("RENDERER::t_prj change: %.3f\n", setting.t_prj);
}

void renderer::changeDisplay(Setting::DISPLAY d){
	setting.display = d;
}

void renderer::changeUPDisplay(Setting::UPDISPLAY d){
	setting.updisplay = d;
}

void renderer::set_setting(const Setting &_setting) {
	setting = _setting;
}

// antialiasing ON/OF
bool renderer::toggle_antialiasing() { return anti_aliasing = !anti_aliasing; };

// copy main rendered image map
void renderer::get_image(float *&map) {
	
	// generate temporary map
	float* tempMap = new float[winW * winH * 3];
#pragma omp parallel for
	for (int i = 0; i < winW*winH * 3; i++) { tempMap[i] = render_map[i] * setting.brightness; }
	
	// change pointers
	delete[] map;		// deallocate memory
	map = tempMap;		// assign new pointer
};

void renderer::cam_zoom(float vfov) { scene_->cam_zoom(vfov); };
void renderer::cam_move(const vec3 &pos) { scene_->cam_move(pos); };
void renderer::cam_rot(float angle) { scene_->cam_rot(angle); }


//**************************************//
//			raycast methods				//
//**************************************//

// cast ray with given viewport pixel position
// and compute the corresponding color.
color renderer::raycast(const ray &r) {

	//	1. intersection test	//
	hitqueue hits;				// intersection object storage

	if (setting.BVHAccleration) bvhTree.intersect(r, hits);
	else intersection_test(r, hits);

	//	2. compute colors	//
		
	return shader::instance()->getColor_DR(hits.top(), this);
	//return shader::instance()->getColor(r, hits.top(), this);
}



// raycast in D/R ray
// D/R ray is a secondary ray that shoots towards the light from the primary hit.
float renderer::raycast_DR(const hitrec &hit, const vec3 &targetP, const shape* targetObj, const light* light) const {

	// ready to perform intersection test
	float R = 0;
	float t_start = 0;	// t of origin point(start) = 0
	float t_end = 0;
	float LIGHT_ENERGY = 0;

	bool  isInside;
	bool  selfIntersect;
	isInside = selfIntersect = hit.object->getID() == targetObj->getID();

	vec3	 new_origin = hit.hitP - hit.normal * setting.DIG;	// dig inside
	vec3	 new_direction = normalize(targetP - new_origin);	// towards center of the light
	ray		 new_ray(new_origin, new_direction);
	hitqueue hits;


	// ---	Step1. Perform intersection test with current target object		--- //

	targetObj->intersection(new_ray, hits);	// Intersection Test!


	while (!hits.empty()) {
		if (isInside) {	// going in -> out
//		if (dot(new_direction, hits.top().normal) >= 0) {	// going in -> out

			t_end = hits.top().t;
			float new_r = t_end - t_start;

			if (selfIntersect)	// R0 term happens here
				R += new_r;
			else
				R += (new_r / (t_start + setting.A_ZI));

			isInside = false;
		}
		else {				// going out -> in
			t_start = hits.top().t;
			isInside = true;
		}

		if (!hits.empty()) hits.pop();	// done with current hit object
	}

	// return R0 Term
	if (selfIntersect) return R;


	// ---	Step2. Perform intersection test with light	--- //

	// clear previous queue
	hitqueue emptyQueue;
	std::swap(hits, emptyQueue);

	if (light->get_type() == light::DRAREA) {

		LIGHT_ENERGY = ((drAreaLight*)(light))->get_intensity(new_ray);
	}
	else if (light->get_type() == light::POINT) {

		LIGHT_ENERGY = light->get_intensity();
	}

	// RN Term
	return R * LIGHT_ENERGY;
};



//**************************************//
//		 rendering helper methods		//
//**************************************//


// Simulate intersection test with given ray and shape.
// Returns the nearest object that is hit, and corresponding t value.
void renderer::intersection_test(const ray &r, hitqueue &hits) const {

	for (int i = 0; i < scene_->shapes.size(); i++) {
		float t, u, v;
		vec3 normal;

		// perform intersection test
		bool result = scene_->shapes[i]->intersection(r, hits);
	}
};

// generate sampling point
// for multisampling
void renderer::samplePos(float &x, float &y, int iteration) {
	if		(iteration == 0) { x = x - 0.333; y = y - 0.333; }
	else if (iteration == 1) { x = x;		  y = y - 0.333; }
	else if (iteration == 2) { x = x + 0.333; y = y - 0.333; }
	else if (iteration == 3) { x = x - 0.333; y = y; }
	else if (iteration == 4) { x = x;		  y = y; }
	else if (iteration == 5) { x = x + 0.333; y = y; }
	else if (iteration == 6) { x = x - 0.333; y = y + 0.333; }
	else if (iteration == 7) { x = x;		  y = y + 0.333; }
	else if (iteration == 8) { x = x + 0.333; y = y + 0.333; }
}

void renderer::buildBVH() {

	printf("BVHAccel:: Start Building BVH Tree...\n");

	for (int i = 0; i < scene_->shapes.size(); i++) {

		// build bvh for meshes
		shape* currentShape = scene_->shapes[i];
		if (currentShape->getType() == shape::MESH) {
			dynamic_cast<mesh*>(currentShape)->buildBVHTree(setting.BVHMaxShapes, setting.BVHSplitMethod);
		}
	}
	bvhTree = BVHAccel(scene_->shapes, setting.BVHMaxShapes, setting.BVHSplitMethod);
	
	printf("BVHAccel:: Construction finished.\n");
}
#ifndef SM_RENDERER_H_
#define SM_RENDERER_H_
/**************************************************************
*
*		renderer.h
*
*		class renderer
*
*		Main renderer class which controls overall
*		scene and raytracing.
*
**************************************************************/

#include <math.h>
#include <memory>
#include <iostream>
#include <chrono>
#include "scene.h"
#include "shape.h"
#include "setting.h"
#include "bvh.h"

class shader;

class renderer {
public:

	// constructors
	renderer();
	renderer(int win_x, int win_y, const Setting &setting);
	~renderer();

	// main render method
	void render();

	// getter & setter
	void get_image(float *&map);
	void set_setting(const Setting &setting);
	
	// user control methods
	int changeSampleNum(int di);
	float changeP(float df);
	float changeS(float ds);
	float changeBrightness(float dt);
	bool toggle_antialiasing();
	void changePLPO(float dt);
	void changePrj(float dt);
	void changeDisplay(Setting::DISPLAY d);
	void changeUPDisplay(Setting::UPDISPLAY d);
	void cam_zoom(float vfov);
	void cam_move(const vec3 &pos);
	void cam_rot(float angle);

	// primary raycast method
	color raycast(const ray &r);
	float raycast_DR(const hitrec &hit, const vec3 &targetP, const shape* targetObj, const light* light) const;

	std::shared_ptr<scene> scene_;		// main scene
	Setting setting;					// render setting	

	BVHAccel bvhTree;
	
	
private:
	// member variables
	int		winW, winH;
	float*	render_map;
	bool	anti_aliasing;
	int		sample_index;	// antialiasing sample index
	//static shader  *shader;		// main shader
	
	// helper methods
	void intersection_test(const ray &r, hitqueue &hits) const;
	void samplePos(float &x, float &y, int iteration);
	void buildBVH();
};

#endif // !SM_RENDERER_H_
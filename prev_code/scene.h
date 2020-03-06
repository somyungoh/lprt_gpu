#ifndef _SM_SCENE_H_
#define _SM_SCENE_H_

/**************************************************************
*
*		scene.h
*
*		class scene
*
*		- stores all the objects, lights
*
**************************************************************/

#include "setting.h"
#include "light.h"
#include "camera.h"
#include "mesh.h"

class scene{

public:
	scene();
	~scene();

	scene(scene const&) = delete;
	scene& operator=(scene const&) = delete;

	// initialize
	void init_scene(const Setting &setting);

	// camera control
	void cam_zoom(float vfov);
	void cam_move(const vec3 &pos);
	void cam_rot(float angle);

	camera	 cam;
	std::vector<shape*> shapes;	// array of implicit objects
	std::vector<light*> lights;	// array of lights
};

#endif // !_SM_SCENE_H_

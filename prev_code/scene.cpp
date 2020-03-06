#include "scene.h"

scene::scene(){}
scene::~scene() {};

// camera control
void scene::cam_zoom(float vfov) { cam.zoom(vfov); };
void scene::cam_move(const vec3 &pos) { cam.move(pos); };
void scene::cam_rot(float angle) { cam.rotate(angle); }

//******* CHANGE THIS to add/remove objects
// scene initialization: camera, scene objects.
void scene::init_scene(const Setting &setting) {
	
	//	*****	Main Camera		***** //

	vec3 camPos(0, 1.2, 5);
	vec3 camUp(0, 1, 0);
	vec3 camLookat(0, 0.6, 0);
	//vec3 camLookat(0, 0.7f, 0);
	float vFOV = 35;

	cam = camera(camPos, camUp, camLookat, vFOV, setting.width, setting.height);


	//	*****	Materials		***** //

	// selected colors (colour lover's)
	color C_BG(63 / 255.f, 184 / 255.f, 175 / 255.f);
	color C_Floor(127 / 255.f, 199 / 255.f, 175 / 255.f);
	color C_Obj1(218 / 255.f, 216 / 255.f, 167 / 255.f);
	color C_Obj2(255 / 255.f, 158 / 255.f, 157 / 255.f);
	color C_Obj3(255 / 255.f, 61 / 255.f, 127 / 255.f);

	material M_Ground(C_Floor, material::DR);
	material M_BackGround(C_BG, material::DR);
	material M_ObjR(C_Obj1, material::DR);
	material M_ObjG(C_Obj2, material::DR);
	material M_ObjB(C_Obj3, material::DR);

	color RED(204 / 255.f, 51 / 255.f, 63 / 255.f);
	color ORANGE(235 / 255.f, 104 / 255.f, 65 / 255.f);
	color BLUE(0 / 255.f, 160 / 255.f, 176 / 255.f);
	color YELLOW(237 / 255.f, 201 / 255.f, 81 / 255.f);
	color GREEN(199 / 255.f, 244 / 255.f, 100 / 255.f);
	color PURPLE(254 / 255.f, 194 / 255.f, 250 / 255.f);

	std::vector<color> palatte;
	palatte.push_back(RED);
	palatte.push_back(GREEN);
	palatte.push_back(BLUE);
	palatte.push_back(ORANGE);
	palatte.push_back(YELLOW);
	palatte.push_back(PURPLE);

	material mat_Red(RED, material::DR);
	material mat_Blue(BLUE, material::DR);
	material mat_Yellow(YELLOW, material::DR);
	material mat_Green(GREEN, material::DR);
	material mat_Purple(PURPLE, material::DR);
	material mat_DR_White(color(1), material::DR);
	material mat_DR_Black(color(0), material::DR);
	material mat_DR_Gray(color(0.9), material::DR);


	//	*****	Light	***** //
	
	if (setting.light == Setting::POINT) {
		pointLight* L = new pointLight(vec3(0, 5, 0), color(1), 1.f);
		lights.push_back(L);
	}
	else if (setting.light == Setting::AREA) {
		areaLight* L = new areaLight(vec3(0, 3.0, 0), vec3(0, -1, 0), vec3(0, 0, -1), color(1), 1, 0.25, 0.25);	
		lights.push_back(L);
	}
	else if (setting.light == Setting::AREADR) {
		// drAreaLight* L = new drAreaLight(vec3(0, 3.f, 0), vec3(0, -1, 0), normalize(vec3(0, 0, -1)), color(1), 1, 1, 1);
		// drAreaLight* L = new drAreaLight(vec3drAreaLight* L = new drAreaLight(vec3(0, 5.0f, 0), vec3(0, -1, 0), normalize(vec3(0, 0, -1)), color(1), 1, 0.75f, 0.75f);
		drAreaLight* L = new drAreaLight(vec3(0, 5.0f, 0), vec3(0, -1, 0), normalize(vec3(0, 0, -1)), color(1), 1, 0.33f, 0.33f);
		lights.push_back(L);
	}
	
	
	//	*****	Objects		***** //

	// shapes.push_back(new sphere(vec3(0, 0, 0), 1.0, mat_DR_White, 300));
	// shapes.push_back(new sphere(vec3(0, 1, 0), 1.0, mat_DR_White, 300));
	
	shapes.push_back(new sphere(vec3(-0.7, 1.83, -0.4), 0.3, mat_DR_White, 301));
	shapes.push_back(new sphere(vec3(-0.5, 1.15, -0.3), 0.4, mat_DR_White, 300));
	shapes.push_back(new sphere(vec3(0.9, 0.8, 1), 0.3, mat_DR_White, 302));
	shapes.push_back(new sphere(vec3(-1.1, 0.8, -0.7), 0.35, mat_DR_White, 303));
	shapes.push_back(new sphere(vec3(-0.33, 0.431, 0.05), 0.43, mat_DR_White, 304));
	shapes.push_back(new sphere(vec3(-1.2, 0.85, 0.7), 0.4, mat_DR_White, 305));
	shapes.push_back(new sphere(vec3(0.15, 1.35, 0.5), 0.38, mat_DR_White, 303));
	shapes.push_back(new sphere(vec3(0.8, 0.7, 0), 0.6, mat_DR_White, 306));
	shapes.push_back(new sphere(vec3(0.9, 1.7, -0.13), 0.4, mat_DR_White, 307));

	// ** Capsules
	//mesh* capsuleV = new mesh(vec3(0, 0, 0), mat_Blue, 500);
	//mesh* capsuleH = new mesh(vec3(0, 0, 0), mat_Blue, 500);
	//capsuleV->load_objFile("Object/capsule_v.obj");
	//capsuleH->load_objFile("Object/capsule_h.obj");
	//shapes.push_back(capsuleH);
	
	// ** dodecahedron
	// mesh* dodecahedron = new mesh(vec3(0, 0, 0), mat_Blue, 500);
	// dodecahedron->load_objFile("Object/dodecahedron.obj");
	// shapes.push_back(dodecahedron);
	
	// ** dodecahedron - interpolated normals
	// mesh* dodecahedron = new mesh(vec3(0, 0, 0), mat_Blue, 500);
	// dodecahedron->load_objFile("Object/dodecahedron_irp.obj");
	// shapes.push_back(dodecahedron);
	
	// ** dodecahedrons
	// mesh* dodecahedrons = new mesh(vec3(0, 0, 0), mat_Blue, 500);
	// dodecahedrons->load_objFile("Object/dodecahedrons.obj");
	// shapes.push_back(dodecahedrons);
	
	// ** dodecahedron multiple sets
	//mesh* dodecahedron00 = new mesh(vec3(0, 0, 0), mat_Blue, 500);
	//mesh* dodecahedron01 = new mesh(vec3(0, 0, 0), mat_Blue, 500);
	//mesh* dodecahedron02 = new mesh(vec3(0, 0, 0), mat_Blue, 500);
	//dodecahedron00->load_objFile("Object/dodecahedron00.obj");
	//dodecahedron01->load_objFile("Object/dodecahedron01.obj");
	//dodecahedron02->load_objFile("Object/dodecahedron02.obj");
	//shapes.push_back(dodecahedron00);
	//shapes.push_back(dodecahedron01);
	//shapes.push_back(dodecahedron02);

	// ** bunny
	// mesh* bunny = new mesh(vec3(0, 0, 0), mat_DR_White, 500);
	// bunny->load_objFile("Object/bunny.obj");
	// // bunny->load_locaterFile("Object/bunny_locaters.txt");
	// shapes.push_back(bunny);

	// ** dragon
	// mesh* dragon = new mesh(vec3(0, 0, 0), mat_DR_White, 600);
	// dragon->load_objFile("Object/dragon.obj");
	// shapes.push_back(dragon);

	// ** pixel
	// mesh* pixel = new mesh(vec3(0, 0, 0), mat_Yellow, 700);
	// pixel->load_objFile("Object/pixel.obj");
	// shapes.push_back(pixel);

	// ** buddah
	// mesh* buddah = new mesh(vec3(0, 0, 0), mat_Yellow, 800);
	// buddah->load_objFile("Object/buddah_high.obj");
	// shapes.push_back(buddah);

	// shapes.push_back(new plane(vec3(-0.5697,0.7922,2.0064), cam.Forward(), cam.Up(), mat_Yellow, 0.0013, 0.0013, 1000));
	// shapes.push_back(new plane(vec3(0, 0, -10), vec3(0, 0, 1), vec3(1, 0, 0), mat_Red, 100, 100, 202));
	// shapes.push_back(new plane(vec3(0, -1, 0), vec3(0, 1, 0), vec3(0, 0, -1), mat_DR_White, 100, 100, 201));
	shapes.push_back(new plane(vec3(0, 0, 0), vec3(0, 1, 0), vec3(0, 0, -1), mat_DR_White, 100, 100, 201));
	
	
	// **** Load Light Samples from obj

	// ** cube
	// mesh* lightobj = new mesh(vec3(0, 0, 0), mat_Blue, 500);
	// lightobj->load_objFile("Object/Light_Samples/cube.obj");
	
	// // adding lights from dodecahedron
	// for(int i = 0; i < lightobj->vertices.size(); i++){

	// 	lights.push_back(new pointLight(
	// 		lightobj->vertices[i],
	// 		color(1,1,1),
	// 		//palatte[i % palatte.size()],
	// 		1.f)
	// 	);
	// }
};

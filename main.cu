#include <iostream>
#include <time.h>
#include "GL/freeglut.h"	// -lGL -lglut

#include "ray.h"
#include "hit.h"
//#include "material.h"
#include "shape.h"
#include "camera.h"

#define WIDTH   800
#define HEIGHT  600
#define tx      8
#define ty      8

// CUDA frame block
float *fb;
shape   **d_scene;
camera  **d_camera;

// GLUT display map
float *display_map;
 
// timing
float deltaTime;
float lastFrame;


using namespace std;


// limited version of checkCudaErrors from helper_cuda.h in CUDA examples
#define checkCudaErrors(val) check_cuda( (val), #val, __FILE__, __LINE__ )

void check_cuda(cudaError_t result, char const *const func, const char *const file, int const line) {
    if (result) {
        std::cerr << "CUDA error = " << static_cast<unsigned int>(result) << " at " <<
            file << ":" << line << " '" << func << "' \n";
        // Make sure we call CUDA Device Reset before exiting
        cudaDeviceReset();
        exit(99);
    }
}

// generate scene
__global__ void create_scene(shape **scene, camera **cam) {
	if (threadIdx.x == 0 && blockIdx.x == 0) {
        scene[0] = new plane(vec3(0, -1, 0), vec3(0, 1, 0), vec3(1, 0, 0),
                                material(color(1, 1, 1), material::DR), 100);
        scene[1] = new sphere(vec3(0, 1, 0), 0.5,
								material(color(1, 0, 0), material::DR), 200);
		*cam = new camera(vec3(0,1,-5),
						vec3(0, 1, 0),
						vec3(0, 0, 0),
						45, WIDTH, HEIGHT);
	}
}

__device__ color raycast(const ray &r){
	return color(0.7, 0.2, 0.3);
}


__global__ void render(float *fb, int max_x, int max_y, camera **cam, shape **world) {
    int i = threadIdx.x + blockIdx.x * blockDim.x;
    int j = threadIdx.y + blockIdx.y * blockDim.y;
	if((i >= max_x) || (j >= max_y)) return;
	
	// generate ray
	// float xf = x;
	// float yf = y;
	// samplePos(xf, yf, i);			// grab sampling position
	
	// ********     R   A   Y   C   A   S   T     ******** //
	ray	r	= (*cam)->getRay((float)i, (float)j);	// get ray from world coordinate
	color c(0,0,0);

	int pixel_index = j*max_x*3 + i*3;
	
    // fb[pixel_index + 1] = float(j) / max_y;
	// fb[pixel_index + 0] = float(i) / max_x;
	// fb[pixel_index + 2] = 0.2;
	
	// intersection test
	hitqueue hits;
	
	for(int i = 0; i < 2 ; i++)
		world[i]->intersection(r, hits);
	
	if(!hits.isEmpty()){
		// basic shading
		color c_obj = hits.top().object->get_mat().Diffuse();
		vec3  p_obj = hits.top().hitP;
		vec3  p_lgt = vec3(0, 2, -0.5);
		vec3  n_obj = hits.top().normal;
		c = c_obj * dot(normalize(p_lgt - p_obj), n_obj);
	}
	fb[pixel_index + 0] = c[0];
	fb[pixel_index + 1] = c[1];
	fb[pixel_index + 2] = c[2];

	// int pixel_index = j*max_x + i;
    // curandState local_rand_state = rand_state[pixel_index];
    // vec3 col(0,0,0);
    // for(int s=0; s < ns; s++) {
    //     float u = float(i + curand_uniform(&local_rand_state)) / float(max_x);
    //     float v = float(j + curand_uniform(&local_rand_state)) / float(max_y);
    //     ray r = (*cam)->get_ray(u,v);
    //     col += color(r, world, &local_rand_state);
    // }
    // rand_state[pixel_index] = local_rand_state;
    // col /= float(ns);
    // col[0] = sqrt(col[0]);
    // col[1] = sqrt(col[1]);
    // col[2] = sqrt(col[2]);
    // fb[pixel_index] = col;
}

// *************** GLUT Routine *********************


// void copy_to_display() {

//     for (int j = HEIGHT-1; j >= 0; j--) {
//         for (int i = 0; i < WIDTH; i++) {
//             size_t pixel_index = j*3*WIDTH + i*3;
            
//             float r = fb[pixel_index + 0];
//             float g = fb[pixel_index + 1];
//             float b = fb[pixel_index + 2];
//             int ir = int(255.99*r);
//             int ig = int(255.99*g);
//             int ib = int(255.99*b);
//         }
//     }
// }

// idle func
void idle() {

	glutPostRedisplay();
 	
	// compute time counts
	float currentFrame = 0.001f * glutGet(GLUT_ELAPSED_TIME); // since it is in milliseconds
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;	
	
	// display current fps
	char title[32];
	float currentFPS = 1.f / deltaTime;
	sprintf(title, "LPRT - GPU \t %.5f fps", currentFPS);
	glutSetWindowTitle(title);
}

// keyboard input funtion
void keyboard(unsigned char key, int x, int y)
{
}


// arrowkey input
void arrowkey(int key, int x, int y)
{
	switch (key) {
	case GLUT_KEY_UP:
		//raytracer.cam_move(vec3(0, CAM_MOVE_ADD, 0));
		cout << "INPUT::camera orientation change" << endl;
		break;

	case GLUT_KEY_DOWN:
		//raytracer.cam_move(vec3(0, -1 * CAM_MOVE_ADD, 0));
		cout << "INPUT::camera orientation change" << endl;
		break;

	case GLUT_KEY_LEFT:
		//raytracer.cam_rot(-1 * CAM_ROT_ANGLE);
		//raytracer.cam_move(vec3(-1 * CAM_MOVE_ADD, 0, 0));
		cout << "INPUT::camera orientation change" << endl;
		break;

	case GLUT_KEY_RIGHT:
		//raytracer.cam_rot(CAM_ROT_ANGLE);
		//raytracer.cam_move(vec3(CAM_MOVE_ADD, 0, 0));
		cout << "INPUT::camera orientation change" << endl;
		break;

	default:
		break;
	}

	glutPostRedisplay();		// redraw
}


// resize function
void resize(int w, int h) {
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, (w / 2), 0, (h / 2), 0, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}


// display function
void display() {

	// raytracer.render();		// MAIN RENDERING ROUTINE
	// raytracer.get_image(display_map);

	// cout << "MAIN::Render finished." << endl;

    
    clock_t start, stop;
    start = clock();
    // Render our buffer
    dim3 blocks(WIDTH/tx+1,HEIGHT/ty+1);
    dim3 threads(tx,ty);
    render<<<blocks, threads>>>(fb, WIDTH, HEIGHT, d_camera, d_scene);
    checkCudaErrors(cudaGetLastError());
    checkCudaErrors(cudaDeviceSynchronize());
    stop = clock();
    double timer_seconds = ((double)(stop - start)) / CLOCKS_PER_SEC;
    //std::cerr << "took " << timer_seconds << " seconds.\n";
	
	glClear(GL_COLOR_BUFFER_BIT);
	glRasterPos2i(0, 0);
	glDrawPixels(WIDTH, HEIGHT, GL_RGB, GL_FLOAT, fb);
	glutSwapBuffers();

}


// **************************************************


int main(int argc, char* argv[]) {

    //
    // CUDA Routine
    //

    int nx = WIDTH;
    int ny = HEIGHT;

    std::cerr << "Rendering a " << nx << "x" << ny << " image ";
    std::cerr << "in " << tx << "x" << ty << " blocks.\n";

    int num_pixels = nx*ny;
    size_t fb_size = 3*num_pixels*sizeof(float);

    // allocate FB
    checkCudaErrors(cudaMallocManaged((void **)&fb, fb_size));

    // allocate display ap
    display_map = new float[WIDTH * HEIGHT * 3];

	
	// scene setup
	checkCudaErrors(cudaMalloc((void **)&d_scene, 2*sizeof(shape *)));
	checkCudaErrors(cudaMalloc((void **)&d_camera, sizeof(camera *)));
    create_scene<<<1,1>>>(d_scene, d_camera);
    checkCudaErrors(cudaGetLastError());
    checkCudaErrors(cudaDeviceSynchronize());

	std::cout << "DEVICE::Scene initialization success.\n";
	


    //
    // GLUT Routine
    //

	//	GLUT Initialization
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(WIDTH, HEIGHT);

	//	GLUT Display
	glutCreateWindow("LPRT_GPU");
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(arrowkey);
	glutDisplayFunc(display);
	glutReshapeFunc(resize);
	glutIdleFunc(idle);

	glutMainLoop();


	checkCudaErrors(cudaFree(fb));
	
	delete[] display_map;

    return 0;
}
#include <iostream>
#include <time.h>
#include "GL/freeglut.h"	// -lGL -lglut

#define WIDTH   800
#define HEIGHT  600
#define tx      8
#define ty      8

// CUDA frame block
float *fb;
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

__global__ void render(float *fb, int max_x, int max_y) {
    int i = threadIdx.x + blockIdx.x * blockDim.x;
    int j = threadIdx.y + blockIdx.y * blockDim.y;
    if((i >= max_x) || (j >= max_y)) return;
    int pixel_index = j*max_x*3 + i*3;
    fb[pixel_index + 0] = float(i) / max_x;
    fb[pixel_index + 1] = float(j) / max_y;
    fb[pixel_index + 2] = 0.2;
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
    render<<<blocks, threads>>>(fb, WIDTH, HEIGHT);
    checkCudaErrors(cudaGetLastError());
    checkCudaErrors(cudaDeviceSynchronize());
    stop = clock();
    double timer_seconds = ((double)(stop - start)) / CLOCKS_PER_SEC;
    std::cerr << "took " << timer_seconds << " seconds.\n";


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

    return 0;
}
#ifndef SM_SETTING_H_
#define SM_SETTING_H_

#include "smath.h"

struct Setting {

	// Rendering Parameter
	enum MODE		{ CLASSIC, DR, RV, RVAA, UPRT };
	enum LIGHT		{ POINT, AREA, AREADR };
	enum SAMPLING	{ UNIFORM, UNIFORM_JITTER, STRATIFIED_JITTER };
	enum DISPLAY	{ NDOTL, DRRT, COMPOSIT };
	enum UPDISPLAY	{ UMBRA, PENUMBRA, BOTH };

	// General
	int   width, height;
	MODE  mode;
	LIGHT light;
	DISPLAY display;
	bool  printLog;
	float brightness;

	// BVH 
	bool BVHAccleration;
	int  BVHSplitMethod;
	int  BVHMaxShapes;
	
	// ***   Shading Methods   *** //

	// 1. Classic Shader Parameters 
	bool  AA;
	bool  flatShade;
	float RVAASoPow;	// powering term for So in RVAA
	int  SampleNum;
	float P; // LP Filter
	float S; // LP Filter

	// Area Light Sampling
	SAMPLING  SamplingMethod;

	// 2. DR Shader Parameter
	float DIG;
	float A_ZI;
	float K_R0;
	float K_RN;
	float K_TOTAL_DR_S;
	float EXP_TOTAL_DR_S;

	// 3.DRAA Shader Parameter
	// Sample point parameters
	UPDISPLAY updisplay;
	float prjAlpha;	// addition constant in projection step (t1-t0)/(t0 + prjAlpha);
	float t_plpo;	// interpolation parameter between Po / Pl (1:Po 0:Pl)
	float t_prj;	// projection parameter. Interpolation between (t0 : t1)

	// DR AA
	float DCAAPix_scale;
	float DCAAFx_scale;

	color bgColor;
};


#endif // !SM_SETTING_H_

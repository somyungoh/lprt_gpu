#include "renderer.h"
#include "shader.h"

shader* shader::instanceObj = nullptr;

// constructors
shader::shader() {};
shader::~shader() {};

// instance shader object
shader* shader::instance() {
	
	// generate instance
	if (!instanceObj) 
		instanceObj = new shader();
	
	return instanceObj;
}


color shader::getColor(const ray &r, const hitrec &hit, const renderer *render) const {

	// empty hit
	if (!&hit) return render->setting.bgColor;

	// ***   0. Prepare for liftoff   *** //

	// {hitqueue, energy}
	struct hitSet{
		hitqueue queue;
		float E;
		hitSet(const hitqueue &hq, float e) : queue(hq), E(e) {};
	};
	
	// ***	If we want to sample from multiple lights	*** //

	float maxcos=1.;
	float mincos=0.;


	int   N_SUBDIV = render->setting.SampleNum;
	float P 	   = render->setting.P;
	float S_NDOTL  = 0;		   // Sum N dot L (Depricated Now)
	// color SC	   = color(0); // Sum of colors	
	// color SC_NDOTL = color(0); // Sum of color and N dot L

	// case when we are computing the maximum
	float MAX_NDOTL = 0;

	for(light* l : render->scene_->lights){
		
		vec3 PO = l->origin();

		float offsetX = 0;
		float offsetY = 0;

		// Most likely not used.
		// if(render->setting.SamplingMethod == Setting::STRATIFIED_JITTER){
		// 	offsetX = rand() / (float)RAND_MAX - 0.5f;
		// 	offsetY = rand() / (float)RAND_MAX - 0.5f;
		// }

		// sum n dot l
		float NdotL = glm::dot(hit.normal, glm::normalize(PO - hit.hitP));
		NdotL = std::fmax(NdotL, 0);
		NdotL = (NdotL - mincos) / (maxcos - mincos);
		
		// sum up
		
		//// ................................TEST
		// powered light color
		// color CP = 	color(
		// 	std::pow(l->get_color().r, P),
		// 	std::pow(l->get_color().g, P),
		// 	std::pow(l->get_color().b, P)
		// );

		// SC_NDOTL += color(CP * std::pow(NdotL, P));
		// SC		 += color(1.f); 
		// SC		 += CP; 
		//// ....................................

		S_NDOTL += std::pow(NdotL, P);

		// MAX_NDOTL = std::fmax(MAX_NDOTL, NdotL);
	}

	// // Maximum - NdotL
	S_NDOTL  = std::pow(S_NDOTL, 1.f/P);
	S_NDOTL /= std::pow(4, 1.f/P);
	// S_NDOTL /= std::pow(render->scene_->lights.size(), 1.f/P);



	////................ TEST::Color Compositing (Depricated)

	// SC_NDOTL = color(
	// 	std::pow(SC_NDOTL.r, 1.f/P),
	// 	std::pow(SC_NDOTL.g, 1.f/P),
	// 	std::pow(SC_NDOTL.b, 1.f/P)
	// );

	// SC_NDOTL /= color(
	// 	std::pow(SC.r, 1.f/P),
	// 	std::pow(SC.g, 1.f/P),
	// 	std::pow(SC.b, 1.f/P)
	// );

	//// TEST END ..............................................


	// S_NDOTL = S_NDOTL > 1 ? 1 : S_NDOTL;
	// S_NDOTL = std::fmax(S_NDOTL, 0);
	
	//return hit.object->get_mat().Diffuse() * SC_NDOTL;
	return hit.object->get_mat().Diffuse() * S_NDOTL;

	// ------------------------------------------------------
	


	// ***	If we want to sample from a area light	*** //

	// // main light object
	// drAreaLight *light = (drAreaLight*)render->scene_->lights[0];
	
	// float maxcos=1.;
	// float mincos=0.;

	// // PO = (0,0) position of the light
	// vec3 PO = light->origin() - light->Nx() * light->Sx() * 0.5f - light->Ny() * light->Sy() * 0.5f;

	// int   N_SUBDIV = render->setting.SampleNum;
	// float P = render->setting.P;
	// float S_NDOTL  = 0;		// sum of N dot L
	// float S_OCCLU  = 0;		// sum of occlusion
	// bool  occluded = false;

	// // case when we are computing the maximum
	// float MAX_NDOTL = 0;

	// // subdivide into a uniform grid
	// for(int i = 0; i < N_SUBDIV; i++){
	// 	for(int j = 0; j < N_SUBDIV; j++){
			
	// 		// *****     Direct Illumination     ***** //

	// 		// sampling position - local
	// 		float dx = (i + 0.5f) / N_SUBDIV;
	// 		float dy = (j + 0.5f) / N_SUBDIV;
			
	// 		// sampling position - object (light)
	// 		vec3 sp = PO + 
	// 			light->Nx() * light->Sx() * dx + 
	// 			light->Ny() * light->Sy() * dy;

	// 		// sum n dot l
	// 		float NdotL = glm::dot(hit.normal, glm::normalize(sp - hit.hitP));
	// 		NdotL = std::fmax(NdotL, 0);
	// 		NdotL = (NdotL - mincos) / (maxcos - mincos);
	// 		S_NDOTL += std::pow(NdotL, P);

	// 		MAX_NDOTL = std::fmax(MAX_NDOTL, NdotL);


	// 		// *****     Indirect Illumination     ***** //

	// 		float offsetX = 0;
	// 		float offsetY = 0;

	// 		if(render->setting.SamplingMethod == Setting::STRATIFIED_JITTER){
	// 			offsetX = rand() / (float)RAND_MAX - 0.5f;
	// 			offsetY = rand() / (float)RAND_MAX - 0.5f;
	// 		}

	// 		// sampling position - local
	// 		float jx = (i + 0.5f + offsetX) / N_SUBDIV;
	// 		float jy = (j + 0.5f + offsetY) / N_SUBDIV;
			
	// 		// sampling position - object (light)
	// 		vec3 jp = PO + 
	// 			light->Nx() * light->Sx() * jx + 
	// 			light->Ny() * light->Sy() * jy;

	// 		// (Indirect Illumination) occlusion test
	// 		ray shadowRay(hit.hitP + hit.normal * 0.0001f, glm::normalize(jp - hit.hitP));
	// 		hitqueue hitqueue;
	// 		render->bvhTree.intersect(shadowRay, hitqueue);
	// 		float Wi = hitqueue.empty() ? 0.1 : 0.9;

	// 		S_OCCLU += std::pow(Wi, P);			
	// 	}
	// }

	// // Direct Illumination
	// S_NDOTL  = std::pow(S_NDOTL, 1.f/P);
	// S_NDOTL /= std::pow(N_SUBDIV * N_SUBDIV, 1.f/P);

	// S_NDOTL = S_NDOTL > 1 ? 1 : S_NDOTL;
	// S_NDOTL = std::fmax(S_NDOTL, 0);

	// // Indirect Illumination (Ocllusion)
	// S_OCCLU  = std::pow(S_OCCLU,  1.f/P);
	// S_OCCLU  / std::pow(N_SUBDIV * N_SUBDIV, 1.f/P);

	// S_OCCLU = S_OCCLU > 1 ? 1 : S_OCCLU;
	// S_OCCLU = std::fmax(S_OCCLU, 0);
	
	// return hit.object->get_mat().Diffuse() * (1 - S_OCCLU);

	// --------------------------------------------------------------------- //
}


//******************************//
//		D/R Shading	Methods		//
//******************************//

// main DR shading.
// input: original ray, DR light source, hit point
color shader::getColor_DR(const hitrec &hit, const renderer *raytracer) const {
	
	// if there is no hit, return background color
	if (!&hit) return raytracer->setting.bgColor;

	drAreaLight* light = (drAreaLight*)raytracer->scene_->lights[0];
	

	// initialize
	float R0 = 0;
	float RN = 0;
	float cosDR;

	// **** THIS IS TEMPORARY INITIALIZATION
	// **** WHERE WE ASSUME WE ONLY HAVE ONE LIGHT	


	// TODO: Loop through samples

	int   N_SUBDIV = raytracer->setting.SampleNum;
	bool  occluded = false;

	// case when we are computing the maximum
	float MAX_NDOTL = 0;


	// NEW::Storing R value for each light j
	float Rj[N_SUBDIV * N_SUBDIV];
			
	// subdivide into a uniform grid
	for(int i = 0; i < N_SUBDIV; i++){
		for(int j = 0; j < N_SUBDIV; j++){
			
			// NEW::Index for light j
			int LIGHT_INDEX = j * N_SUBDIV + i;


			// *****     Direct Illumination     ***** //

			// sampling position - local
			float dx = (i + 0.5f) / N_SUBDIV;
			float dy = (j + 0.5f) / N_SUBDIV;
			
			// sampling position - object (light)
			vec3 sp = light->origin() + 
				light->Nx() * light->Sx() * (dx - 0.5f) + 
				light->Ny() * light->Sy() * (dy - 0.5f);

			// *****     Indirect Illumination     ***** //

			float offsetX = 0;
			float offsetY = 0;

			// if(render->setting.SamplingMethod == Setting::STRATIFIED_JITTER){
			// 	offsetX = rand() / (float)RAND_MAX - 0.5f;
			// 	offsetY = rand() / (float)RAND_MAX - 0.5f;
			// }

					
			// *****	R0 TERM		***** //

			// compute primary surface scatter
			float dotNL = glm::dot(hit.normal, glm::normalize(sp - hit.hitP));
			float Ri0 = raytracer->setting.DIG * ((1.f + 0.03f) / (std::fmax(0, dotNL) + 0.03f));

			// add via LP-norm method
			R0 += std::pow(Ri0, raytracer->setting.P);

			// NEW::Locally Save R0, Rn
			float loc_R0 = std::pow(Ri0, raytracer->setting.P);
			float loc_Rn = 0.f;

			// *****	RN TERM		***** //

			// raycast to the light
			ray ray_occ(hit.hitP + hit.normal * 0.0001f, glm::normalize(sp - hit.hitP + hit.normal * 0.0001f));
			hitqueue hits;
			raytracer->bvhTree.intersect(ray_occ, hits);
						
			while (!hits.empty()) {

				// what kind of problem is this,,??
				if(hits.size() > 10000) return color(0, 0, 0);

				float t1 = -1, t0 = -1;

				// Out -> In
				hitrec hit1 = hits.top();
				hits.pop();

				// initialize properties for this round
				t0 = hit1.t;

				// In -> Out
				hitrec hit2 = hits.top();
				t1 = hit2.t;	// save t1

				// finish current round
				hits.pop();

				// std::cout << hit1.hitP.x << "," << hit1.hitP.y  << hit1.hitP.z << std::endl;
			
				// RN += std::pow(t1 - t0, raytracer->setting.P);					
				// RN += std::pow((t1 - t0) / t0, raytracer->setting.P);		
				RN += std::pow(2 * (t1 - t0) / (t0 + t1), raytracer->setting.P);
				
				// NEW::Local RN
				//loc_Rn += std::pow(2 * (t1 - t0) / (t0 + t1), raytracer->setting.P);
				loc_Rn += std::pow(2 * (t1 - t0) / (t0 + t1), raytracer->setting.P) / (float)(raytracer->scene_->shapes.size() - 1);
			}

			// NEW::Save total R value
			// Rj[LIGHT_INDEX] = std::pow(loc_R0 + loc_Rn, 1.f / raytracer->setting.P) / (float)(raytracer->scene_->shapes.size() - 1);
			Rj[LIGHT_INDEX] = std::pow(loc_R0 + loc_Rn, 1.f / raytracer->setting.P);
		}
	}


	// *****	TOTAL DR	***** //

	float DR = std::pow(R0 + RN, 1.f / raytracer->setting.P);
	// DR  = std::pow(DR, 1.f / raytracer->setting.P);
	DR /= std::pow(N_SUBDIV * N_SUBDIV, 1.f / raytracer->setting.P);


	// NEW::New combining Ri term
	float new_DR = 0;
	for(int j = 0; j < N_SUBDIV * N_SUBDIV; j++){
		// new_DR += std::pow(raytracer->setting.DIG / Rj[j], raytracer->setting.S);
		new_DR += (1.f / (float)(N_SUBDIV * N_SUBDIV)) * std::pow(raytracer->setting.DIG / Rj[j], raytracer->setting.S);
	}
	new_DR = std::pow(new_DR, 1.f / raytracer->setting.S);
	
	// DR = DR > 1 ? 1 : DR;
	// DR = std::fmax(DR, 0);
	
	new_DR = new_DR > 1 ? 1 : new_DR;
	new_DR = std::fmax(new_DR, 0);
	

	// computing surface color
	// color surface_color = hit.object->get_mat().Diffuse() * (1.f / DR);
	// color surface_color = hit.object->get_mat().Diffuse() * (raytracer->setting.DIG / DR);

	// NEW::Surface Color via new_DR
	color surface_color = hit.object->get_mat().Diffuse() * new_DR;

	return surface_color;
};


// helper function to compute sampled position
vec3 shader::computeSampledPos(int i, int j, const plane &pl, const renderer *raytracer) const {
	
	switch (raytracer->setting.SamplingMethod) {
	case Setting::UNIFORM:
		
		break;
	case Setting::UNIFORM_JITTER:

		break;
	case Setting::STRATIFIED_JITTER:

		break;
	}
};


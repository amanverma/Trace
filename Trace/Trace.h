#include <iosfwd>
#include <vector>
#include <fstream>
#include <boost/property_tree/ptree.hpp>
#include "Def.h"

namespace RayTracing
{
	//struct point{float x; float y; float z;};
	struct material{color diffuse; color specular; float power;float reflectance;};
	struct sphere{float radius;	point center; int materialId;};
	struct lightSource{point position; color intensity;};
	struct ray{point startPosition; vecteur direction;};
	struct scene
	{std::vector<material> materialContainer;
	std::vector<sphere> sphereContainer;
	std::vector<lightSource> lightContainer;
	int sceneHeight, sceneWidth;};

	class Tracer
	{
	public:
		Tracer(void);
		~Tracer();
		void SetImageWidth(float value);
		void SetImageHeight(float value);
		void SetNumberOfMaterials(int value);
		void SetNumberOfSpheres(int value);
		void SetNumberOfLights(int value);
		void GetNumberOfMaterials();
		void GetNumberOfSpheres();
		void GetNumberOfLights();
		bool ParseInput(boost::property_tree::ptree &pt, scene &sceneFile);
		bool CreateImage(char* input, scene &sceneFile);
		bool HitSphere(const ray &r, const sphere &s, float &t);
		
	};
}
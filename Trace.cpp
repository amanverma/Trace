#include "stdafx.h"
#include<iostream>
#include "Trace.h"
#include<vector>
#include<fstream>
#include <string>
#include "Parameters.h"
#include <boost/property_tree/ptree.hpp>
#include "Def.h"

using namespace std;
namespace RayTracing
{
Tracer::Tracer(void)
{
}
Tracer::~Tracer()
{
}
bool Tracer::ParseInput(boost::property_tree::ptree &pt, scene &sceneFile)
{

	int numberOfMaterials, numberOfSpheres, numberOfLights;
	std::string str; int i;
	numberOfMaterials = pt.get<int>("TraceParameters.SceneParameters.NumberOfMaterials.Value",0);
	numberOfSpheres = pt.get<int>("TraceParameters.SceneParameters.NumberOfSpheres.Value",0);
	numberOfLights = pt.get<int>("TraceParameters.SceneParameters.NumberOfLights.Value",0);
	sceneFile.sceneHeight = pt.get<int>("TraceParameters.SceneParameters.ImageHeight.Value",0);
	sceneFile.sceneWidth = pt.get<int>("TraceParameters.SceneParameters.ImageWidth.Value",0);
	sceneFile.lightContainer.resize(numberOfLights);
	sceneFile.materialContainer.resize(numberOfMaterials);
	sceneFile.sphereContainer.resize(numberOfSpheres);

	for(i=0;i<numberOfMaterials;i++)
	{
		std::ostringstream oss;
		oss<<i;
		str = "TraceParameters.MaterialParameters.Material"+ oss.str();
		cout<<str<<endl;
		sceneFile.materialContainer[i].reflectance = pt.get<float>((string)(str+".ReflectionCoefficient.Value"),0.5);
		sceneFile.materialContainer[i].blue = pt.get<float>(str+".DiffuseParameters.DiffuseX.Value",1.0);
		sceneFile.materialContainer[i].green = pt.get<float>(str+".DiffuseParameters.DiffuseY.Value",1.0);
		sceneFile.materialContainer[i].red = pt.get<float>(str+".DiffuseParameters.DiffuseZ.Value",1.0);
	}
	for(i=0;i<numberOfSpheres;i++)
	{
		std::ostringstream oss;
		oss<<i;
		str = "TraceParameters.SphereParameters.Sphere"+ oss.str();
		cout<<str<<endl;
		sceneFile.sphereContainer[i].radius = pt.get<float>(str+".Size.Value",50);
		sceneFile.sphereContainer[i].center.x = pt.get<float>(str+".CenterParameters.CenterX.Value",1);
		sceneFile.sphereContainer[i].center.y = pt.get<float>(str+".CenterParameters.CenterY.Value",1);
		sceneFile.sphereContainer[i].center.z = pt.get<float>(str+".CenterParameters.CenterZ.Value",1);
		sceneFile.sphereContainer[i].materialId = pt.get<float>(str+".MaterialId.Value",1);
	}
	for(i=0;i<numberOfLights;i++)
	{
		std::ostringstream oss;
		oss<<i;
		str = "TraceParameters.LightSourceParameters.LightSource"+ oss.str();
		cout<<str<<endl;
		sceneFile.lightContainer[i].position.x = pt.get<float>(str+".Position.PositionX.Value",0);
		sceneFile.lightContainer[i].position.y = pt.get<float>(str+".Position.PositionY.Value",0);
		sceneFile.lightContainer[i].position.z = pt.get<float>(str+".Position.PositionZ.Value",0);
		sceneFile.lightContainer[i].blue = pt.get<float>(str+".Intensity.IntensityX.Value",0.6);
		sceneFile.lightContainer[i].green = pt.get<float>(str+".Intensity.IntensityY.Value",0.7);
		sceneFile.lightContainer[i].red = pt.get<float>(str+".Intensity.IntensityZ.Value",1.0);
	}
	return true;
}
	
bool Tracer::HitSphere(const ray &r, const sphere &s, float &t) 
 { 
    // Intersection of a ray and a sphere
	 vecteur dist = s.center-r.startPosition;
	/*dist.x = s.center.x - r.startPosition.x;
	dist.y = s.center.y - r.startPosition.y;
	dist.z = s.center.z - r.startPosition.z;
	*/
	 float B = r.direction*dist; 
		//(r.direction[0]*dist.x+r.direction[1]*dist.y+r.direction[2]*dist.z);
	 float D = B*B-dist*dist+s.radius*s.radius;
		//(B*B - (dist.x*dist.x+dist.y+dist.y+dist.z*dist.z)+s.radius*s.radius);
    if (D < 0.0f) 
        return false; 
    float t0 = B - sqrtf(D); 
    float t1 = B + sqrtf(D);
    bool retvalue = false;  
    if ((t0 > 0.1f) && (t0 < t)) 
    {
        t = t0;
        retvalue = true; 
    } 
    if ((t1 > 0.1f) && (t1 < t)) 
    {
        t = t1; 
        retvalue = true; 
    }
    return retvalue; 
 }

bool Tracer::CreateImage(char *inp, scene &sceneFile)
{
	cout<<"jfljdslfjdsjfds"<<endl;
	cout<<sceneFile.sceneHeight<<endl<<sceneFile.sceneWidth<<endl;
	cout<<sceneFile.materialContainer[0].blue;
	ofstream imageFile(inp,ios_base::binary);
    if (!imageFile)
        return false; 
    // Addition of the TGA header
    imageFile.put(0).put(0);
    imageFile.put(2);        /* RGB not compressed */

    imageFile.put(0).put(0);
    imageFile.put(0).put(0);
    imageFile.put(0);

    imageFile.put(0).put(0); /* origin X */ 
    imageFile.put(0).put(0); /* origin Y */

	imageFile.put((unsigned char)(sceneFile.sceneWidth & 0x00FF)).put((unsigned char)((sceneFile.sceneWidth & 0xFF00) / 256));
	imageFile.put((unsigned char)(sceneFile.sceneHeight & 0x00FF)).put((unsigned char)((sceneFile.sceneHeight & 0xFF00) / 256));
    imageFile.put(24);       /* 24 bit bitmap */ 
    imageFile.put(0); 
    // end of the TGA header 

    // Scanning 
	for (int y = 0; y < sceneFile.sceneHeight; ++y) { 
		for (int x = 0; x < sceneFile.sceneWidth; ++x) {
        float red = 0, green = 0, blue = 0;
        float coef = 1.0f;
        int level = 0; 

		ray viewRay;
		viewRay.direction.resize(3);
		viewRay.startPosition.x = float(x);
		viewRay.startPosition.y = float(y);
		viewRay.startPosition.z = float(-1000.0f);
		viewRay.direction[0] = 0.0f;
		viewRay.direction[1] = 0.0f;
		viewRay.direction[2] = 1.0f;
		do
        { 
            // Looking for the closest intersection
            float t = 2000.0f;
            int currentSphere= -1;

            for (unsigned int i = 0; i < sceneFile.sphereContainer.size(); ++i) 
            { 
                if (HitSphere(viewRay, sceneFile.sphereContainer[i], t)) 
                {
                    currentSphere = i;
                }
            }

            if (currentSphere == -1)
                break;

			point newStart = {(viewRay.startPosition.x + t * viewRay.direction[0]),(viewRay.startPosition.y + t * viewRay.direction[1]),(viewRay.startPosition.z + t * viewRay.direction[2])};
			//Normal at the point of intersection for spheres
			vecteur n = newStart-sceneFile.sphereContainer[currentSphere].center;
			//{(newStart.x - sceneFile.sphereContainer[currentSphere].center.x),(newStart.y - sceneFile.sphereContainer[currentSphere].center.y),(newStart.z - sceneFile.sphereContainer[currentSphere].center.z)};
			float temp = (n.x*n.x+n.y*n.y+n.z*n.z);
            if (temp == 0.0f) 
                break; 

            temp = 1.0f / sqrtf(temp); 
			n.x = temp*n.x; n.y = temp*n.y; n.z = temp*n.z;

            material currentMat = sceneFile.materialContainer[sceneFile.sphereContainer[currentSphere].materialId]; 

            //Calculate the Illumination Value at that Point
			cout<<"d;jf;dj;kfjd;sf"<<sceneFile.sphereContainer.size()<<endl;
            for (unsigned int j = 0; j < sceneFile.lightContainer.size(); ++j) {
                lightSource current = sceneFile.lightContainer[j];
				vecteur dist =current.position-newStart;
				//{(current.position.x - newStart.x),(current.position.y-newStart.y),(current.position.z - newStart.z)};
				if (((n.x* dist.x)+(n.y* dist.y)+(n.z* dist.z)) <= 0.0f)
                    continue;
                float t = sqrtf(dist.x*dist.x+dist.y+dist.y+dist.z*dist.z);
                if ( t <= 0.0f)
                    continue;
                ray lightRay;
				lightRay.startPosition.x = newStart.x; lightRay.startPosition.y = newStart.y; lightRay.startPosition.z = newStart.z;
				lightRay.direction[0] = (1/t) * dist.x; lightRay.direction[1] =(1/t) * dist.y; lightRay.direction[2] =(1/t) * dist.z;

                // computation of the shadows
                bool inShadow = false; 
                for (int i = 0; i < sceneFile.sphereContainer.size(); ++i) {
                    if (HitSphere(lightRay, sceneFile.sphereContainer[i], t)) {
                        inShadow = true;
                        break;
                    }
                }
                if (!inShadow) {
                    // lambert
					float lambert = ((lightRay.direction[0] * n.x)+(lightRay.direction[1] * n.y)+(lightRay.direction[2] * n.z)) * coef;
                    red += lambert * current.red * currentMat.red;
                    green += lambert * current.green * currentMat.green;
                    blue += lambert * current.blue * currentMat.blue;
                }
            }

            // We iterate on the next reflection
			coef *= currentMat.reflectance;
			float reflet = 2.0f * ((viewRay.direction[0] * n.x)+(viewRay.direction[1] * n.y)+(viewRay.direction[2] * n.z));
			viewRay.startPosition = newStart;
			viewRay.direction[0] = viewRay.direction[0] - reflet * n.x;
			viewRay.direction[1] = viewRay.direction[1] - reflet * n.y;
			viewRay.direction[2] = viewRay.direction[2] - reflet * n.z;

            level++;
        } 
        while ((coef > 0.0f) && (level < 10));   
		imageFile.put(min(blue*255.0f,255.0f)).put(min(green*255.0f, 255.0f)).put(min(red*255.0f, 255.0f));
		//imageFile.put(min((blue*255.0,255.0)).put(min(green*255.0, 255.0)).put(min(red*255.0, 255.0));

    }
    }
    return true;
}
}






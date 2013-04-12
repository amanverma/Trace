#include "stdafx.h"
#include<iostream>
#include "Trace.h"
#include<vector>
#include<fstream>
#include <string>
#include "Parameters.h"
#include <boost/property_tree/ptree.hpp>
#include "Def.h"
#include <vtkImageWriter.h>
#include <vtkImageData.h>
#include <vtkStructuredPointsWriter.h>
#include <vtkNew.h>
#include <algorithm>
#include <limits>
#include <math.h>
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
		//cout<<str<<endl;
		sceneFile.materialContainer[i].reflectance = pt.get<float>((string)(str+".ReflectionCoefficient.Value"),0.5);
		sceneFile.materialContainer[i].diffuse.red = pt.get<float>(str+".DiffuseParameters.DiffuseX.Value",1.0);
		sceneFile.materialContainer[i].diffuse.green = pt.get<float>(str+".DiffuseParameters.DiffuseY.Value",1.0);
		sceneFile.materialContainer[i].diffuse.blue = pt.get<float>(str+".DiffuseParameters.DiffuseZ.Value",1.0);
		sceneFile.materialContainer[i].specular.red = pt.get<float>(str+".SpecularParameters.SpecularX.Value",1.0);
		sceneFile.materialContainer[i].specular.green = pt.get<float>(str+".SpecularParameters.SpecularY.Value",1.0);
		sceneFile.materialContainer[i].specular.blue = pt.get<float>(str+".SpecularParameters.SpecularZ.Value",1.0);
		sceneFile.materialContainer[i].power = pt.get<float>(str+".Power.Value",1.0);
	}
	for(i=0;i<numberOfSpheres;i++)
	{
		std::ostringstream oss;
		oss<<i;
		str = "TraceParameters.SphereParameters.Sphere"+ oss.str();
		//cout<<str<<endl;
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
		//cout<<str<<endl;
		sceneFile.lightContainer[i].position.x = pt.get<float>(str+".Position.PositionX.Value",0);
		sceneFile.lightContainer[i].position.y = pt.get<float>(str+".Position.PositionY.Value",0);
		sceneFile.lightContainer[i].position.z = pt.get<float>(str+".Position.PositionZ.Value",0);
		sceneFile.lightContainer[i].intensity.red = pt.get<float>(str+".Intensity.IntensityX.Value",0.6);
		sceneFile.lightContainer[i].intensity.blue = pt.get<float>(str+".Intensity.IntensityY.Value",0.7);
		sceneFile.lightContainer[i].intensity.green = pt.get<float>(str+".Intensity.IntensityZ.Value",1.0);
	}
	return true;
}
	
bool Tracer::HitSphere(const ray &r, const sphere &s, float &t) 
 { 
    // Intersection of a ray and a sphere
	 vecteur dist = s.center-r.startPosition;
	 float B = r.direction*dist; 
	 float D = B*B-dist*dist+s.radius*s.radius;
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
	vtkImageData *image = vtkImageData::New();
	image->SetDimensions(sceneFile.sceneWidth,sceneFile.sceneHeight,1);
	image->SetSpacing(1.0,1.0,1.0);
	image->SetOrigin(0,0,0);
	image->SetScalarTypeToUnsignedChar();
	image->SetNumberOfScalarComponents(3);
	image->AllocateScalars();
	image->Update();
    // Scanning 
	for (int y = 0; y < sceneFile.sceneHeight; ++y) 
	{ 
		for (int x = 0; x < sceneFile.sceneWidth; ++x)
		{
			color output = {0.0f,0.0f,0.0f};
			float coef = 1.0f;
			int level = 0; 
			//We shoot ray(startingPoint, direction) in Z direction and then we look for intersections with objects
			ray viewRay = { {float(x), float(y), -1000.0f}, { 0.0f, 0.0f, 1.0f}};
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
				if (currentSphere  == -1) //ray does not hits so we ignore it and move on 
					break;
				
				//Other Wise we find the new starting point for the reflected ray at the intersection point
				point newStart = viewRay.startPosition+t*viewRay.direction;
				
				//We then find the Normal at the point of intersection for spheres
				vecteur n = newStart-sceneFile.sphereContainer[currentSphere].center;
				float temp = n*n;
				if (temp == 0.0f) 
					break; 
				temp = 1.0f / sqrtf(temp); 
				n = temp*n;

				//We then get the material properties by gettting the materialId of the intersected sphere

				material currentMat = sceneFile.materialContainer[sceneFile.sphereContainer[currentSphere].materialId]; 
				
				//Calculate the Illumination Value at that Point of Intersection

				for (unsigned int j = 0; j < sceneFile.lightContainer.size(); ++j) //Two light sources, can be more too
				{
					lightSource currentLight = sceneFile.lightContainer[j];
					vecteur dist =currentLight.position-newStart;
					if(n*dist<=0.0f)
						continue;
					float t = sqrtf(dist.x*dist.x+dist.y+dist.y+dist.z*dist.z);
					if ( t <= 0.0f)
						continue;
					ray lightRay;
					lightRay.startPosition = newStart;
					lightRay.direction = (1/t) * dist; //We get the unit vector 
					// computation of the shadows
					bool inShadow = false; 
					for (int i = 0; i < sceneFile.sphereContainer.size(); ++i) 
					{
						if (HitSphere(lightRay, sceneFile.sphereContainer[i], t)) 
						{
							//We shoot ray from the intersection point towards the source, 
							//if it hits something else before coming to source, it forms a shadow
							inShadow = true;
							break;
						}
					}
					if (!inShadow) 
					{
						/*IMPORTANT THING TO NOTE IS DIFFUSE TERM IS NOT EFFECTED BY THE VIEWER
						DIRECTION HOWEVER, SPECULAR TERM DEPENDS UPON VIEWER DIRECTION- SPECULAR
						TERM IS LARGE ONLY WHEN VIEWER DIRECTION ALIGNS WITH REFLECTION DIRECTION*/

						// lambert
						float lambert = (lightRay.direction * n) * coef;
						output.red += lambert * currentLight.intensity.red * currentMat.diffuse.red;
						output.green += lambert * currentLight.intensity.green * currentMat.diffuse.green;
						output.blue += lambert * currentLight.intensity.blue * currentMat.diffuse.blue;

						//IMPLEMENTATION OF PHONG SHADING MODEL//			

						//float reflet = 2.0f * (lightRay.direction * n);
						//vecteur phongDir = lightRay.direction - reflet * n;

						////phongDir is nothing but the reflected ray vector R = I - 2(I*N)N//

						//float phong =std::max(phongDir * viewRay.direction,0.0f); //phong dependency on viewer direction
						//phong =  std::powf(phong,currentMat.power);

						///*SPECULAR TERM AFFECT IS MEASURED BY THE ALPHA POWER OF COSINE OF ANGLE 
						//BETWEEN VIEWER DIRECTION & REFLECTION VECTOR */

						//output.red += phong * currentMat.specular.red * currentLight.intensity.red;
						//output.green += phong * currentMat.specular.green * currentLight.intensity.green;
						//output.blue +=phong * currentMat.specular.blue * currentLight.intensity.blue;
								
						//IMPLEMENTATION OF BLINN PHONG SHADING MODEL//

						float fLightRayProjection = lightRay.direction*n ; //light Projection = lightRay*normal direction
						float fViewRayProjection = viewRay.direction*n ;
						/*Instead of taking reflected ray direction as phongDirection, in Blinn-Phong
						the direction of the Blinn Vector H is half way between Viewer V & Light Source L*/
						vecteur blinnDir = lightRay.direction - viewRay.direction;
						float temp = blinnDir * blinnDir ;

						//Normalize Blinn Vector and then find Blinn Coefficient to find
						//Contribution of the Specular component of light

						if (temp != 0.0f )
						{
							//Get blinCoefficient as blinnDir*n/mod(blinn)and then we do the same thing
							float blinn = std::powf(std::sqrtf(temp),-1) * std::max(fLightRayProjection - fViewRayProjection , 0.0f);
							blinn = coef * powf(blinn, currentMat.power);
							output += blinn *currentMat.specular  * currentLight.intensity;
						}
					}
				}
				coef *= currentMat.reflectance;
				//Run the doWhile loop for subsequent reflections
				float reflet = 2.0f * (viewRay.direction*n);
				viewRay.startPosition = newStart; //intersection point
				viewRay.direction = viewRay.direction - reflet*n; //reflected ray direction
				level++;
			}
			while ((coef > 0.0f) && (level < 10));  
			//Creating Image
			image->SetScalarComponentFromFloat(x,y,0,0,std::min(output.red * 255.0, 255.0));
			image->SetScalarComponentFromFloat(x,y,0,1,std::min(output.green * 255.0, 255.0));
			image->SetScalarComponentFromFloat(x,y,0,2,std::min(output.blue * 255.0, 255.0));
			image->Update();

		}
	}
	//Writing to Image
	vtkNew<vtkStructuredPointsWriter> writer;
	writer->SetFileName(inp);
	writer->SetInput(image);
	writer->Update();
	writer->Write();
    return true;
}
}






#include "stdafx.h"
#include<iostream>
#include "Trace.h"
#include<fstream>
#include <string>
#include "Parameters.h"
#include "Def.h"
#include <boost/property_tree/ptree.hpp>
using namespace std;
using  namespace RayTracing;

int _cdecl main(int argc, char* argv[])
{
	if(argc<3)
	{
		cout<<"Insufficient arguments: Provide- Scene.txt Ouput.tga Trace.exe"<<endl;
		return 0;
	}
	scene sceneFile;	
	Tracer trace;
	if(argc==3)
	{
		std::ifstream t(argv[1]);
		t.seekg(0, std::ios::end);
		size_t size = t.tellg();
		std::string buffer(size, ' ');
		t.seekg(0);
		t.read(&buffer[0], size);
		
		boost::property_tree::ptree pt;
		std::string str = argv[1];
		fromXml(buffer,pt);
		print_ptree(pt);
		if(!trace.ParseInput(pt, sceneFile))
		{
			cout<<"Failure while reading the Scene file"<<endl;
			return 0;
		}
		if(!trace.CreateImage(argv[2], sceneFile))
		{
			cout<<"Failure while creating the Image file"<<endl;
			return 0;
		}
	}
	return 1;
}
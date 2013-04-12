#include "stdafx.h"
#include "Parameters.h"
#include <boost/property_tree/json_parser.hpp>
#include "rapidxml.hpp"
#include "rapidxml_print.hpp"
#include <iostream>

using boost::property_tree::ptree;
using namespace std;
using namespace rapidxml;

void print_ptree(const ptree & ptree)
{
	cout << ptree.data() << endl;
	if (ptree.size() == 0)
	{
		return;
	}
	for (ptree::const_iterator it = ptree.begin(); it != ptree.end(); ++it)
	{
		cout << it->first << ": ";
		print_ptree(it->second);
	}
}

rapidxml::xml_node<> * toXmlInternal(const ptree & pt, rapidxml::xml_document<> * doc, rapidxml::xml_node<> * parent = 0)
{
	// iterate over all children.
	// if child is not empty, it's a Section.
	// if child is empty, it's a Param.
	if (!parent)
	{
		parent = doc->allocate_node(node_element, "Section");
		parent->append_attribute(doc->allocate_attribute("Name", "Parameters"));;
	}
	for (ptree::const_iterator it = pt.begin(); it != pt.end(); ++it)
	{
		xml_node<> * node;
		if (it->second.empty())
		{
			node = doc->allocate_node(node_element, "Param", 0);
			xml_attribute<> * nameAttr = doc->allocate_attribute("Name", it->first.c_str());
			xml_attribute<> * valueAttr = doc->allocate_attribute("Value", it->second.data().c_str());
			node->append_attribute(nameAttr);
			node->append_attribute(valueAttr);
		}
		else
		{
			node = doc->allocate_node(node_element, "Section", 0);
			xml_attribute<> * nameAttr = doc->allocate_attribute("Name", it->first.c_str());
			node->append_attribute(nameAttr);
			toXmlInternal(it->second, doc, node);
		}
		parent->append_node(node);

	}
	return parent;
}

string toXml(const ptree & pt)
{
	xml_document<> doc;
	xml_node<> * node = doc.allocate_node(node_element, "Parameters", 0);
	doc.append_node(node);

	xml_node<> * n = toXmlInternal(pt, &doc);
	node->append_node(n);

	string s;
	print(std::back_inserter(s), doc, 0);
	return s;
}

void fromXmlInternal(rapidxml::xml_node<> * node, ptree & parent)
{
	char * nodeName = node->name();
	xml_attribute<> * nameAttr = node->first_attribute("Name");
	char * name = 0;
	if (nameAttr)
	{
		name = nameAttr->value();
	}
	if ((strcmp(nodeName, "Section") == 0) ||(strcmp(nodeName, "Parameters") == 0))
	{
		ptree section;
		xml_node<> * subnode = node->first_node(); 
		do
		{
			fromXmlInternal(subnode, section);
			subnode = subnode->next_sibling();
		}while(subnode != 0);
		if (strcmp(name, "Parameters") == 0)
		{
			parent = section;
		}
		else
			parent.put_child(name, section);
	}
	else if (strcmp(nodeName, "Param") == 0)
	{
		xml_attribute<> * valueAttr = node->first_attribute("Value");
		parent.put(name, valueAttr->value());
	}

}

void fromXml(const string xmlString, ptree & pt)
{
	xml_document<> doc;
	memory_pool<> mp;
	char * pXmlString = mp.allocate_string(xmlString.c_str());
	try
	{
		doc.parse<0>(pXmlString);
		if (strcmp(doc.first_node()->name(), "Parameters") == 0)
			fromXmlInternal(doc.first_node()->first_node(), pt);
		else
			fromXmlInternal(doc.first_node(), pt);
	}
	catch(parse_error pe)
	{
		cerr << "Error parsing. What: " << pe.what() << " where:" << (char *)(pe.where<char>()) << endl;
		throw pe;
	}
	catch(exception e)
	{
		cerr << e.what() << endl;
		throw e;
	}
}

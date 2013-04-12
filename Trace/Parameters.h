#pragma once


#include <boost/property_tree/ptree.hpp>
#include <string>

using boost::property_tree::ptree;

template class boost::property_tree::basic_ptree<std::string, std::string>;

void print_ptree(const ptree & ptree);
std::string toXml(const ptree & pt);
void fromXml(const std::string xmlString, ptree & pt);
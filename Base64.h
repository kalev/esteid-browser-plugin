#include <string>
#include <vector>

#pragma once
std::string base64_encode(const std::string&);
std::string base64_encode(const std::vector<unsigned char>& in);
std::string base64_decode(std::string in);

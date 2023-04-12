#pragma once
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <mstch/mstch.hpp>
#include "Player.h"

using namespace std;

class HtmlOutputGenerator
{
public:
	HtmlOutputGenerator(const string& template_file_path, const string& output_file_path) :
		template_file_path(template_file_path), output_file_path(output_file_path) {}
	void generate_html(const string& top_player_tag);

private:
	string template_file_path;
	string output_file_path;
};


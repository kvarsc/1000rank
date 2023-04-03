#pragma once
#include <cpr/cpr.h>
#include "json.hpp"
#include "miniz.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <functional>
using json = nlohmann::json;
using namespace std;
class DatabaseDownloader
{
	public:
		DatabaseDownloader();
		~DatabaseDownloader();

		bool check_and_download_database(const string& repo_owner, const string& repo_name, const string& asset_name, const string& local_file_path, const string& input_db_file_path, const string& output_db_file_path, const bool& reextract_db);
		bool extract_database(const string& zip_file_path, const string& input_file_path, const string& output_file_path);
};


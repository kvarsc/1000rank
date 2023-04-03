// 1000rank.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include "json.hpp"
#include "DatabaseDownloader.h"

using json = nlohmann::json;
using namespace std;

int main()
{
    // Open and read the config file
    if (!filesystem::exists("config.json"))
    {
        cout << "config.json not found." << endl;
        exit(1);
    }
    ifstream config_file("config.json");

    if (!config_file.is_open())
    {
		cout << "Failed to open config.json." << endl;
		exit(1);
	}

    json config;
    try
    {
        config_file >> config;
    }
    catch (json::parse_error& e)
    {
		cout << "Failed to parse config.json: " << e.what() << endl;
		exit(1);
	}

    // Load the config fields
    string repo_owner = config["repo_owner"];
    string repo_name = config["repo_name"];
    string asset_name = config["asset_name"];
    string local_file_path = config["local_file_path"];
    string input_db_file_path = config["input_db_file_path"];
    string output_db_file_path = config["output_db_file_path"];
    bool reextract_db = config["reextract_db"];

    // Create a DatabaseDownloader instance
    DatabaseDownloader db_downloader;

    // Check if the database needs to be downloaded
    if (db_downloader.check_and_download_database(repo_owner, repo_name, asset_name, local_file_path, input_db_file_path, output_db_file_path, reextract_db))
    {
		cout << "Database is downloaded/up-to-date." << endl;
	}
    else
    {
		cout << "Error downloading database." << endl;
	}

    cout << "Program complete!" << endl;
    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file

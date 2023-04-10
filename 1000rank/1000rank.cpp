// 1000rank.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <vector>
#include "json.hpp"
#include "DatabaseDownloader.h"
#include "DatabaseManager.h"
#include "RankingSystem.h"

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

    // Load the database fields
    string repo_owner = config["database"]["repo_owner"];
    string repo_name = config["database"]["repo_name"];
    string asset_name = config["database"]["asset_name"];
    string local_file_path = config["database"]["local_file_path"];
    string input_db_file_path = config["database"]["input_db_file_path"];
    string output_db_file_path = config["database"]["output_db_file_path"];
    string filtered_db_file_path = config["database"]["filtered_db_file_path"];
    string pre_season_date = config["database"]["pre_season_date"];
    string post_season_date = config["database"]["post_season_date"];
    int minimum_entrants = config["database"]["minimum_entrants"];

    // Load the special tournaments
    vector<string> special_tournament_keys;
    if (config.contains("special_tournaments"))
    {
        for (const auto& tournament : config["special_tournaments"])
        {
            if (tournament.contains("id"))
            {
				special_tournament_keys.push_back(tournament["id"].get<string>());
            }
		}
	}

    // Load the parameters of the FIRE algorithm
    double sig = config["algorithm_parameters"]["sig"];
    double force_threshold = config["algorithm_parameters"]["force_threshold"];
    int nmin = config["algorithm_parameters"]["nmin"];
    double finc = config["algorithm_parameters"]["finc"];
    double fdec = config["algorithm_parameters"]["fdec"];
    double astart = config["algorithm_parameters"]["astart"];
    double fa = config["algorithm_parameters"]["fa"];
    double dtmax = config["algorithm_parameters"]["dtmax"];
    double dtmax_inc = config["algorithm_parameters"]["dtmax_inc"];
    int dtmax_freq = config["algorithm_parameters"]["dtmax_freq"];

    // Load the fields for which actions to take
    bool reextract_db = config["actions"]["reextract_db"];
    bool refilter_db = config["actions"]["refilter_db"];
    bool recompute_rankings = config["actions"]["recompute_rankings"];

    // Create a DatabaseDownloader instance
    DatabaseDownloader db_downloader;

    // Check if the database needs to be downloaded
    int db_downloaded;
    if (db_downloaded = db_downloader.check_and_download_database(repo_owner, repo_name, asset_name, local_file_path, input_db_file_path, output_db_file_path, reextract_db))
    {
		cout << "Database is downloaded/up-to-date." << endl;
	}
    else
    {
		cout << "Error downloading database." << endl;
	}

    bool filter_db = (refilter_db || (db_downloaded == 2));

    // If the database needs to be filtered, create a DatabaseManager instance and filter the database
    if (filter_db)
    {
        DatabaseManager db_manager(output_db_file_path);
        db_manager.create_filtered_sets_database(filtered_db_file_path, pre_season_date, post_season_date, minimum_entrants, special_tournament_keys);
    }

    // Create a db_manager for the filtered database and print all counts
    DatabaseManager db_manager(filtered_db_file_path);
    db_manager.print_all_counts();

    // If the database needed to be filtered, do endpoint filtering and add the ranking columns
    if (filter_db)
    {
        // TODO do endpoint filtering
		db_manager.add_ranking_columns();
	}

    // Create the ranking system
    RankingSystem ranking_system(sig, force_threshold, nmin, finc, fdec, astart, fa, dtmax, dtmax_inc, dtmax_freq);

    // Load all the players and matches into the ranking system
    ranking_system.load_all(db_manager);

    // If the rankings need to be computed, compute them
    // And then write the rankings to the database
    if (recompute_rankings || filter_db)
    {
		ranking_system.compute_rankings();
        //ranking_system.write_rankings(db_manager);
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

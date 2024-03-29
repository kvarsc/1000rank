// 1000rank.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <vector>
#include "json.hpp"
#include "DatabaseDownloader.h"
#include "DatabaseManager.h"
#include "RankingSystem.h"
#include "HtmlOutputGenerator.h"

using json = nlohmann::json;
using namespace std;

int main()
{
    // Open and read the meta-config file
    if (!filesystem::exists("config/meta-config.json"))
    {
		cout << "meta-config.json not found." << endl;
		exit(1);
	}
    ifstream meta_config_file("config/meta-config.json");
    if (!meta_config_file.is_open())
    {
		cout << "Failed to open meta-config.json." << endl;
		exit(1);
	}
    json meta_config;
    try
    {
		meta_config_file >> meta_config;
	}
    catch (json::parse_error& e)
    {
		cout << "Failed to parse meta-config.json: " << e.what() << endl;
		exit(1);
	}

    string config_file_name = meta_config["config_file"];
    config_file_name = "config/" + config_file_name;

    // Open and read the config file
    if (!filesystem::exists(config_file_name))
    {
        cout << "Config file " << config_file_name << " not found." << endl;
        exit(1);
    }
    ifstream config_file(config_file_name);
    if (!config_file.is_open())
    {
        cout << "Failed to open config file " << config_file_name << "." << endl;
        exit(1);
    }
    json config;
    try
    {
        config_file >> config;
    }
    catch (json::parse_error& e)
    {
        cout << "Failed to parse config file " << config_file_name << ": " << e.what() << endl;
        exit(1);
    }

    // Load the database fields
    string repo_owner = config["database"]["repo_owner"];
    string repo_name = config["database"]["repo_name"];
    string asset_name = config["database"]["asset_name"];
    string db_local_directory = config["database"]["local_directory"];
    string zip_prefix = config["database"]["zip_prefix"];
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

    // Load the excluded tournaments
    vector<string> excluded_tournament_keys;
    if (config.contains("excluded_tournaments"))
    {
        for (const auto& tournament : config["excluded_tournaments"])
        {
            if (tournament.contains("id"))
            {
				excluded_tournament_keys.push_back(tournament["id"].get<string>());
			}
		}
	}

    // Load the excluded sets
    vector<tourney_set> excluded_sets;
    if (config.contains("excluded_sets"))
    {
        for (const auto& set : config["excluded_sets"])
        {
            if (set.contains("p1_id") && set.contains("p2_id") && set.contains("tournament_key") && set.contains("round_name"))
            {
				tourney_set next_set;
				next_set.p1_id = set["p1_id"].get<string>();
				next_set.p2_id = set["p2_id"].get<string>();
				next_set.tournament_key = set["tournament_key"].get<string>();
				next_set.round_name = set["round_name"].get<string>();
				excluded_sets.push_back(next_set);
			}
		}
	}

    // Load the excluded rounds
    vector<tourney_round> excluded_rounds;
    if (config.contains("excluded_rounds"))
    {
        for (const auto& round : config["excluded_rounds"])
        {
            if (round.contains("tournament_key") && round.contains("round_name"))
            {
				tourney_round next_round;
				next_round.tournament_key = round["tournament_key"].get<string>();
				next_round.round_name = round["round_name"].get<string>();
				excluded_rounds.push_back(next_round);
			}
		}
	}

    // Load player clones
    unordered_map<string, string> player_clones;
    if (config.contains("player_clones"))
    {
        for (const auto& player : config["player_clones"])
        {
            if (player.contains("id") && player.contains("clone"))
            {
                player_clones[player["id"].get<string>()] = player["clone"].get<string>();
            }
        }
    }

    // Load the endpoint filtering parameters
    bool do_endpoint_filtering = config["endpoint_db_filtering"]["do_endpoint_filtering"];
    int regional_minimum_entrants = config["endpoint_db_filtering"]["regional_minimum_entrants"];
    int major_minimum_entrants = config["endpoint_db_filtering"]["major_minimum_entrants"];
    int regional_attendance_threshold = config["endpoint_db_filtering"]["regional_attendance_threshold"];
    int major_attendance_threshold = config["endpoint_db_filtering"]["major_attendance_threshold"];
    int minimum_losses = config["endpoint_db_filtering"]["minimum_losses"];
    int minimum_sets = config["endpoint_db_filtering"]["minimum_sets"];

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

    // Load the uncertainty parameters
    bool subtract_uncertainties = config["uncertainty_parameters"]["subtract_uncertainties"];
    double uncertainty_factor = config["uncertainty_parameters"]["uncertainty_factor"];

    // Load html output fields
    string html_local_directory = config["html_output"]["local_directory"];
    string html_template_file_path = config["html_output"]["html_template_file_path"];
    string html_output_file_path = config["html_output"]["html_output_file_path"];
    string title = config["html_output"]["title"];
    bool apply_scaling = config["html_output"]["apply_scaling"];
    double scaling_factor = config["html_output"]["scaling_factor"];
    double volatility_bin_width = config["html_output"]["volatility_bin_width"];
    bool use_bins_for_volatility = config["html_output"]["use_bins_for_volatility"];

    // Load the volatility bins
    vector<volatility_bin> volatility_bins;
    if (config["html_output"].contains("volatility_bins"))
    {
        for (const auto& bin : config["html_output"]["volatility_bins"])
        {
            volatility_bin next_bin;
            next_bin.max_bin_width_factor = bin["max_bin_width_factor"];
            next_bin.bin_name = bin["bin_name"];
            volatility_bins.push_back(next_bin);
        }
    }

    size_t num_players_to_write = config["html_output"]["num_players_to_write"];
    bool hide_unranked_ranks = config["html_output"]["hide_unranked_ranks"];

    // Load the fields related to rank change from previous year (delta fields)
    bool include_delta = config["delta"]["include_delta"];
    string previous_ranking_period_html = "";
    vector<string> earlier_ranking_period_htmls;
    if (include_delta)
    {
        previous_ranking_period_html = config["delta"]["previous_ranking_period_html"];
        // Load the earlier ranking period htmls into an array of strings
        if (config["delta"].contains("earlier_ranking_period_htmls") && config["delta"]["earlier_ranking_period_htmls"].is_array())
        {
            for (const auto& html : config["delta"]["earlier_ranking_period_htmls"])
            {
                earlier_ranking_period_htmls.push_back(html);
            }
        }
        else
        {
            cerr << "Invalid JSON format or 'earlier_ranking_period_htmls' key not found." << endl;
        }
    }


    // Load the fields for which actions to take
    bool reextract_db = config["actions"]["reextract_db"];
    bool refilter_db = config["actions"]["refilter_db"];
    bool recompute_rankings = config["actions"]["recompute_rankings"];
    bool regenerate_html = config["actions"]["regenerate_html"];

    // Create a DatabaseDownloader instance
    DatabaseDownloader db_downloader;

    // Check if the database needs to be downloaded
    int db_downloaded;
    if (db_downloaded = db_downloader.check_and_download_database(repo_owner, repo_name, asset_name, db_local_directory, zip_prefix, input_db_file_path, output_db_file_path, reextract_db))
    {
		cout << "Database is downloaded/up-to-date." << endl;
	}
    else
    {
		cout << "Error downloading database." << endl;
	}

    // Create booleans for which actions to take
    bool filter_db = (refilter_db || (db_downloaded == 2));
    bool compute_rankings = (recompute_rankings || filter_db);
    bool generate_html = (regenerate_html || compute_rankings);

    // If local directory is not empty, prepend it to both db file paths
    if (db_local_directory != "")
    {
		output_db_file_path = db_local_directory + "/" + output_db_file_path;
        filtered_db_file_path = db_local_directory + "/" + filtered_db_file_path;
	}

    // If the database needs to be filtered, create a DatabaseManager instance and filter the database
    if (filter_db)
    {
        DatabaseManager db_manager(output_db_file_path);
        db_manager.create_filtered_sets_database(filtered_db_file_path, pre_season_date, post_season_date, minimum_entrants, special_tournament_keys, excluded_tournament_keys, excluded_sets, excluded_rounds);
    }

    // Create a db_manager for the filtered database and print all counts
    DatabaseManager db_manager(filtered_db_file_path);
    db_manager.print_all_counts();

    // If the database needed to be filtered, fuse player clones, do endpoint filtering and add the ranking columns
    if (filter_db)
    {
        db_manager.add_indices();
        if (player_clones.size() > 0)
            db_manager.fuse_player_clones(player_clones);
        if (do_endpoint_filtering)
            db_manager.endpoint_filtering(regional_minimum_entrants, major_minimum_entrants, regional_attendance_threshold, major_attendance_threshold, minimum_losses, minimum_sets, special_tournament_keys);
        db_manager.add_ranking_columns();
	}

    // Create the ranking system
    RankingSystem ranking_system(sig, force_threshold, nmin, finc, fdec, astart, fa, dtmax, dtmax_inc, dtmax_freq, scaling_factor);

    // Load all the players and matches into the ranking system
    ranking_system.load_all(db_manager);

    // If the rankings need to be computed, compute them
    if (compute_rankings)
    {
		ranking_system.compute_rankings();
    }

    // Sort players by ranking and store the max ranking score
    ranking_system.sort_players_by_ranking();

    // If the rankings needed to be computed, compute uncertanties too
    // And store ranking and uncertainties in the database
    if (compute_rankings)
    {
        ranking_system.compute_uncertainties();
        // If config is set to subtract uncertainty, do so
        if (subtract_uncertainties)
        {
            ranking_system.subtract_uncertainties(uncertainty_factor);
            // Sort players by ranking again
            ranking_system.sort_players_by_ranking();
        }
        ranking_system.compute_volatilities();
        ranking_system.store_rankings(db_manager);
    }

    // Print the top 10 players
    ranking_system.print_top_players(10);

    // Compute the changes in ranking from last ranking period (deltas)
    if (generate_html && include_delta)
    {
		ranking_system.compute_ranking_deltas(html_local_directory, previous_ranking_period_html, earlier_ranking_period_htmls);
    }

    // Get players, match history, and sorted players
    unordered_map<string, Player> players = ranking_system.get_players();
    unordered_map<string, unordered_map<string, MatchRecord>> match_history = ranking_system.get_match_history();
    vector<reference_wrapper<Player>> sorted_players = ranking_system.get_sorted_players();

    // If the html needs to be generated, generate it
    if (generate_html)
    {
        HtmlOutputGenerator html_generator(html_local_directory, html_template_file_path, html_output_file_path, title, apply_scaling, scaling_factor, sorted_players[0].get().get_ranking_score(), volatility_bin_width, volatility_bins, include_delta);
        html_generator.generate_html(players, match_history, sorted_players, num_players_to_write, use_bins_for_volatility, hide_unranked_ranks);
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

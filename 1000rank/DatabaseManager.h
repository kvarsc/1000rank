#pragma once
#include <iostream>
#include <format>
#include <string>
#include <vector>
#include <tuple>
#include <filesystem>
#include "hdr/sqlite3.h"
#include "hdr/sqlite_modern_cpp.h"
#include "Player.h"

using namespace std;

struct tourney_set {
    string p1_id;
    string p2_id;
    string tournament_key;
    string round_name;
};

struct tourney_round {
    string tournament_key;
    string round_name;
};

class DatabaseManager {
public:
    DatabaseManager(const string& db_path);
    ~DatabaseManager();

    int get_sets_count();
    int get_players_count();
    int get_tournaments_count();
    void print_all_counts();

    void create_filtered_sets_database(const string& new_db_path, string pre_season_date, string post_season_date, int minimum_entrants, const vector<string>& special_tournament_keys, const vector<string>& excluded_tournament_keys, const vector<tourney_set>& excluded_sets, const vector<tourney_round>& excluded_rounds);
    void fuse_player_clones(const unordered_map<string, string>& player_clones);
    void add_indices();
    void endpoint_filtering(int regional_minimum_entrants, int major_minimum_entrants, int regional_attendance_threshold, int major_attendance_threshold, int minimum_losses, int minimum_sets, const vector<string>& special_tournament_keys);
    void add_ranking_columns();

    vector<tuple<string, string, double, double, double>> fetch_all_players();
    vector<tuple<string, string, string>> fetch_all_sets();

    void update_player_ranking_values(unordered_map<string, Player>& players);

private:
    sqlite::database db;
    string special_tournament_keys_string(const vector<string>& special_tournament_keys);
    string excluded_tournament_sets_string(const vector<tourney_set>& excluded_sets);
    string excluded_tournament_rounds_string(const vector<tourney_round>& excluded_rounds);
    void delete_low_attendance_sets(int regional_minimum_entrants, int major_minimum_entrants, int regional_attendance_threshold, int major_attendance_threshold, const string& special_keys_string);
    void delete_low_loss_sets(int minimum_losses);
    void delete_low_set_sets(int minimum_sets);
    void update_players_table();
};
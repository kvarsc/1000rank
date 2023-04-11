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

class DatabaseManager {
public:
    DatabaseManager(const string& db_path);
    ~DatabaseManager();

    int get_sets_count();
    int get_players_count();
    int get_tournaments_count();
    void print_all_counts();

    void create_filtered_sets_database(const string& new_db_path, string pre_season_date, string post_season_date, int minimum_entrants, const vector<string>& special_tournament_keys);
    void add_ranking_columns();

    vector<tuple<string, string, double, double, double>> fetch_all_players();
    vector<tuple<string, string, string>> fetch_all_sets();

    void update_player_ranking_values(unordered_map<string, Player>& players);

private:
    sqlite::database db;
    string special_tournament_keys_string(const vector<string>& special_tournament_keys);
};
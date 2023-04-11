#include "DatabaseManager.h"
#include "time_utils.h"

DatabaseManager::DatabaseManager(const string& db_path) : db(db_path) {
    // You can add any initialization code here if needed
}

DatabaseManager::~DatabaseManager() {
    // Destructor will be called automatically, closing the connection to the database
}

int DatabaseManager::get_sets_count() {
	int count = 0;
	db << "SELECT COUNT(*) FROM sets" >> count;
	return count;
}

int DatabaseManager::get_players_count() {
	int count = 0;
	db << "SELECT COUNT(*) FROM players" >> count;
	return count;
}

int DatabaseManager::get_tournaments_count() {
	int count = 0;
	db << "SELECT COUNT(*) FROM tournament_info" >> count;
	return count;
}

void DatabaseManager::print_all_counts() {
	cout << "Sets: " << get_sets_count() << endl;
	cout << "Players: " << get_players_count() << endl;
	cout << "Tournaments: " << get_tournaments_count() << endl;
}

void DatabaseManager::create_filtered_sets_database(const string& new_db_path, string pre_season_date, string post_season_date, int minimum_entrants, const vector<string>& special_tournament_keys) {
    // Announce the start of filtering and print this database's counts
    cout << "Filtering database..." << endl;
    print_all_counts();

    // Delete the existing file at new_db_path if it exists
    filesystem::remove(new_db_path);

    // Convert the pre_season_date and post_season_date to UNIX time
    int pre_season_unix_time = date_string_to_unix_time(pre_season_date);
    int post_season_unix_time = date_string_to_unix_time(post_season_date);

    // Create the string of special tournament keys
    string special_keys_string = special_tournament_keys_string(special_tournament_keys);

    // Create a new database and attach it
    sqlite::database new_db(new_db_path);
    db << "ATTACH ? AS new_db;" << new_db_path;

    cout << "Creating new tables in new database..." << endl;

    // Create the new 'sets' and 'tournament_info' tables in the new database
    db << "CREATE TABLE new_db.sets AS SELECT * FROM sets WHERE 0;";
    db << "CREATE TABLE new_db.players AS SELECT * FROM players WHERE 0;";
    db << "CREATE TABLE new_db.tournament_info AS SELECT * FROM tournament_info WHERE 0;";

    cout << "Inserting filtered sets..." << endl;

    // Create and execute the query to insert filtered sets
    db <<
        "INSERT INTO new_db.sets "
        "SELECT sets.* "
        "FROM sets "
        "JOIN tournament_info ON sets.tournament_key = tournament_info.key "
        "WHERE tournament_info.end >= ? "
        "AND tournament_info.end < ? "
        "AND NOT (sets.tournament_key LIKE '%sp__1on1' AND sets.location_names LIKE '%\"Round %') " // removes best-of-1 sets from JP tourneys
        "AND (tournament_info.entrants >= ? OR tournament_info.key IN (" + special_keys_string + ")) " // invitationals
        "AND NOT (sets.p1_score = -1 OR sets.p2_score = -1) " // removes DQs
        "AND NOT (tournament_info.online = 1);" // we want offline only!!!
        << pre_season_unix_time
        << post_season_unix_time
        << minimum_entrants;

    cout << "Inserting filtered players..." << endl;

    // Create and execute the query to insert filtered players
    db <<
        "INSERT INTO new_db.players "
        "SELECT players.* "
        "FROM players "
        "JOIN ("
        "SELECT DISTINCT p1_id AS player_id FROM new_db.sets "
        "UNION "
        "SELECT DISTINCT p2_id AS player_id FROM new_db.sets "
        ") AS relevant_player_ids ON players.player_id = relevant_player_ids.player_id;";

    cout << "Inserting filtered tournament_info..." << endl;

    // Create and execute the query to insert filtered tournament information
    db <<
        "INSERT INTO new_db.tournament_info "
        "SELECT DISTINCT tournament_info.* "
        "FROM tournament_info "
        "JOIN sets ON sets.tournament_key = tournament_info.key "
        "WHERE tournament_info.end >= ? "
        "AND tournament_info.end < ? "
        "AND NOT (sets.tournament_key LIKE '%sp__1on1' AND sets.location_names LIKE '%\"Round %') " // removes best-of-1 sets from JP tourneys
        "AND (tournament_info.entrants >= ? OR tournament_info.key IN (" + special_keys_string + ")) " // invitationals
        "AND NOT (sets.p1_score = -1 OR sets.p2_score = -1) " // removes DQs
        "AND NOT (tournament_info.online = 1);" // we want offline only!!!
        << pre_season_unix_time
        << post_season_unix_time
        << minimum_entrants;

    // Detach the new database
    db << "DETACH DATABASE new_db;";

    cout << "Done filtering database." << endl;
}

void DatabaseManager::add_ranking_columns()
{
    cout << "Adding ranking columns (and index) to database..." << endl;

    // Add the ranking columns to the 'players' table
    db << "ALTER TABLE players ADD COLUMN ranking_score REAL DEFAULT 0.0;";
    db << "ALTER TABLE players ADD COLUMN uncertainty REAL DEFAULT 0.0;";
    db << "ALTER TABLE players ADD COLUMN volatility REAL DEFAULT 0.0;";

    // Create an index on the 'player_id' column of the 'players' table
    db << "CREATE INDEX player_id_index ON players (player_id);";
}

vector<tuple<string, string, double, double, double>> DatabaseManager::fetch_all_players()
{
	vector<tuple<string, string, double, double, double>> players;

    db << "SELECT player_id, tag, ranking_score, uncertainty, volatility FROM players" >> [&](string id, string tag, double ranking_score, double uncertainty, double volatility)
    {
        players.emplace_back(id, tag, ranking_score, uncertainty, volatility);
    };

	return players;
}

vector<tuple<string, string, string>> DatabaseManager::fetch_all_sets()
{
	vector<tuple<string, string, string>> sets;
    db << "SELECT p1_id, p2_id, winner_id FROM sets" >> [&](string p1_id, string p2_id, string winner_id)
    {
		sets.emplace_back(p1_id, p2_id, winner_id);
	};
	return sets;
}

void DatabaseManager::update_player_ranking_values(unordered_map<string, Player>& players)
{
    string query = "UPDATE players SET ranking_score = CASE player_id ";
    for (const auto &[id, player] : players) {
		query += "WHEN '" + id + "' THEN " + format("{:9f}", player.get_ranking_score()) + " ";
	}
    query += "ELSE ranking_score END, uncertainty = CASE player_id ";
    for (const auto& [id, player] : players) {
        query += "WHEN '" + id + "' THEN " + format("{:9f}", player.get_uncertainty()) + " ";
    }
    query += "ELSE uncertainty END;";
    db << query;
}

string DatabaseManager::special_tournament_keys_string(const vector<string>& special_tournament_keys) {
    string result;
    for (const auto& key : special_tournament_keys) {
        result += "'" + key + "',";
    }
    if (!result.empty()) {
        result.pop_back();  // Remove the trailing comma
    }
    return result;
}

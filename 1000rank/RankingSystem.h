#pragma once
#include <iostream>
#include <cmath>
#include <algorithm>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <functional>
#include "DatabaseManager.h"
#include "Player.h"
#include "MatchRecord.h"
#include "RankingHtmlParser.h"

using namespace std;

class RankingSystem
{
public:
	RankingSystem(double sig, double force_threshold, int nmin, double finc, double fdec, double astart, double fa, double dtmax, double dtmax_inc, int dtmax_freq, double scaling_factor) :
		sig(sig), force_threshold(force_threshold), nmin(nmin), finc(finc), fdec(fdec), astart(astart), fa(fa), dtmax(dtmax), dtmax_inc(dtmax_inc), dtmax_freq(dtmax_freq), scaling_factor(scaling_factor) {}

	// Getters
	unordered_map<string, Player> get_players() const { return players; }
	unordered_map<string, unordered_map<string, MatchRecord>> get_match_history() const { return match_history; }
	vector<reference_wrapper<Player>> get_sorted_players() const { return sorted_players; }

	void load_players(DatabaseManager& db_manager);
	void load_match_history(DatabaseManager& db_manager);
	void add_match_record(const string& p1_id, const string& p2_id, const string& winner_id);
	void calc_forces_for_all_match_records();
	void load_all(DatabaseManager& db_manager);

	pair<int, int> get_match_history_size();

	// Ranking algorithm force method
	// IMPORTANT: this function computes forces as a function of wins and losses
	// You can adjust this function to change the way forces are calculated!
	void calc_forces_on_all_players();
	double calc_net_force(double force, double score1, double score2);

	// Other ranking algorithm methods
	// Main method minimizes the rankings' energy function using FIRE
	void compute_rankings();
	double deldel();
	double delvel();
	double velvel();

	// Sort players by ranking after ranking algorithm is done
	// And store the max ranking score
	// And store ranks in player objects
	void sort_players_by_ranking();

	// Compute the uncertainties and volatilities
	void compute_uncertainties();
	void compute_volatilities();
	bool is_upset(const Player& winner, const Player& loser)
	{ return winner.get_ranking_score() < loser.get_ranking_score(); }

	// Store ranking scores in database after ranking algorithm is done
	void store_rankings(DatabaseManager& db_manager);

	// Print the top players
	void print_top_players(int n);

	// Compute changes in rank from previous ranking period
	void compute_ranking_deltas(string previous_ranking_period_html, vector<string> earlier_ranking_period_htmls);
private:
	unordered_map<string, Player> players;
	unordered_map<string, unordered_map<string, MatchRecord>> match_history;
	vector<reference_wrapper<Player>> sorted_players;

	// FIRE algorithm parameters
	double sig;
	double force_threshold;
	int nmin;
	double finc;
	double fdec;
	double astart;
	double fa;
	double dtmax;
	double dtmax_inc;
	int dtmax_freq;

	// Other ranking system parameters
	double max_ranking_score = 1.0;
	double scaling_factor;
};
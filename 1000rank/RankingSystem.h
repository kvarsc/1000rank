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

using namespace std;

class RankingSystem
{
public:
	RankingSystem(double sig, double force_threshold, int nmin, double finc, double fdec, double astart, double fa, double dtmax, double dtmax_inc, int dtmax_freq, double scaling_factor) :
		sig(sig), force_threshold(force_threshold), nmin(nmin), finc(finc), fdec(fdec), astart(astart), fa(fa), dtmax(dtmax), dtmax_inc(dtmax_inc), dtmax_freq(dtmax_freq), scaling_factor(scaling_factor) {}

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
	void sort_players_by_ranking();

	// Store the max ranking score after you've sorted the players
	void store_max_ranking_score();

	// Compute the uncertainties
	void compute_uncertainties();

	// Store ranking scores in database after ranking algorithm is done
	void store_rankings(DatabaseManager& db_manager);

	// Print the top players
	void print_top_players(int n);
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
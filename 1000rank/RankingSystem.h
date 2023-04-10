#pragma once
#include <iostream>
#include <algorithm>
#include <string>
#include <unordered_map>
#include <utility>
#include "DatabaseManager.h"
#include "Player.h"
#include "MatchRecord.h"

using namespace std;

class RankingSystem
{
public:
	RankingSystem(double sig, double force_threshold, int nmin, double finc, double fdec, double astart, double fa, double dtmax, int dtmax_freq) :
		sig(sig), force_threshold(force_threshold), nmin(nmin), finc(finc), fdec(fdec), astart(astart), fa(fa), dtmax(dtmax), dtmax_freq(dtmax_freq) {}

	void load_players(DatabaseManager& db_manager);
	void load_match_history(DatabaseManager& db_manager);
	void add_match_record(const string& p1_id, const string& p2_id, const string& winner_id);
	void calc_forces_for_all_match_records();
	void load_all(DatabaseManager& db_manager);

	pair<int, int> get_match_history_size();

	// Ranking algorithm methods
	// IMPORTANT: this function computes forces as a function of wins and losses
	// You can adjust this function to change the way forces are calculated!
	void calc_forces_on_all_players();
	double calc_net_force(double force, double score1, double score2);

private:
	unordered_map<string, Player> players;
	unordered_map<string, unordered_map<string, MatchRecord>> match_history;

	// FIRE algorithm parameters
	double sig;
	double force_threshold;
	int nmin;
	double finc;
	double fdec;
	double astart;
	double fa;
	double dtmax;
	int dtmax_freq;
};


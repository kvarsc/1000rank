#pragma once
#include <iostream>
#include <string>
#include <unordered_map>
#include "Player.h"
#include "MatchRecord.h"

using namespace std;

class RankingSystem
{
public:
	RankingSystem(double force_threshold, int nmin, double finc, double fdec, double astart, double fa, double dtmax, int dtmax_freq) :
		force_threshold(force_threshold), nmin(nmin), finc(finc), fdec(fdec), astart(astart), fa(fa), dtmax(dtmax), dtmax_freq(dtmax_freq) {}

private:
	unordered_map<string, Player> players;
	unordered_map<string, unordered_map<string, MatchRecord>> match_history;

	// FIRE algorithm parameters
	double force_threshold;
	int nmin;
	double finc;
	double fdec;
	double astart;
	double fa;
	double dtmax;
	int dtmax_freq;
};


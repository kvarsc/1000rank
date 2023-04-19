#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <format>
#include <mstch/mstch.hpp>
#include "RankingSystem.h"

using namespace std;

struct volatility_bin {
	double max_bin_width_factor;
	string bin_name;
};

class HtmlOutputGenerator
{
public:
	HtmlOutputGenerator(const string& local_directory, const string& template_file_path, const string& output_file_path, const string& title, const bool& apply_scaling, const double& scaling_factor, const double& max_ranking_score, const double& volatility_bin_width, const vector<volatility_bin>& volatility_bins, const bool include_delta) :
		local_directory(local_directory), template_file_path(template_file_path), output_file_path(output_file_path), title(title), apply_scaling(apply_scaling), scaling_factor(scaling_factor), max_ranking_score(max_ranking_score), volatility_bin_width(volatility_bin_width), volatility_bins(volatility_bins), include_delta(include_delta) {}
	void generate_html(const unordered_map<string, Player>& players, const unordered_map<string, unordered_map<string, MatchRecord>>& match_history, const vector<reference_wrapper<Player>>& sorted_players, size_t num_players_to_write, bool use_bins_for_volatility, bool hide_unranked_ranks);

private:
	string local_directory;
	string template_file_path;
	string output_file_path;
	string title;

	bool apply_scaling;
	double scaling_factor;
	double max_ranking_score;

	double volatility_bin_width;
	vector<volatility_bin> volatility_bins;

	bool include_delta;
};


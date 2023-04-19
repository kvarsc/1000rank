#include "HtmlOutputGenerator.h"

void HtmlOutputGenerator::generate_html(const unordered_map<string, Player>& players, const unordered_map<string, unordered_map<string, MatchRecord>>& match_history, const vector<reference_wrapper<Player>>& sorted_players, size_t num_players_to_write, bool use_bins_for_volatility, bool hide_unranked_ranks)
{
	cout << "Generating HTML..." << endl;

	// Create full template file path
	string full_template_file_path = template_file_path;
	if (local_directory != "")
	{
		full_template_file_path = local_directory + "/" + template_file_path;
	}

	// Load the template file
	ifstream template_file(full_template_file_path);
	stringstream template_stream;
	template_stream << template_file.rdbuf();
	string template_string = template_stream.str();

	// Convert the list of players to a format compatible with mstch
	mstch::array players_data;
	for (size_t i = 0; i < num_players_to_write; ++i)
	{
		// Get the player id and tag
		string player_id = sorted_players[i].get().get_id();
		string player_tag = sorted_players[i].get().get_tag();

		// Compute the ranking score and uncertainty
		double raw_ranking_score = sorted_players[i].get().get_ranking_score(); // keep this for opponent comparisons
		double ranking_score = raw_ranking_score;
		double uncertainty = sorted_players[i].get().get_uncertainty();
		uncertainty = (1.0 / uncertainty);
		if (apply_scaling)
		{
			ranking_score *= (scaling_factor / max_ranking_score);
			uncertainty *= (scaling_factor / max_ranking_score);
		}
		double volatility = sorted_players[i].get().get_volatility();
		string volatility_string = format("{:.9f}", volatility);
		if (use_bins_for_volatility)
		{
			for (const auto& bin : volatility_bins)
			{
				if (volatility <= bin.max_bin_width_factor * volatility_bin_width)
				{
					volatility_string = bin.bin_name;
					break;
				}
			}
		}

		// Get the delta (change in rank from prev. season)
		string delta = sorted_players[i].get().get_delta();

		// Sort the player's opponents by ranking score
		vector<Player> opponents;
		for (const auto& [opponent_id, match_record] : match_history.at(player_id))
		{
			opponents.push_back(players.at(opponent_id));
		}
		sort(opponents.begin(), opponents.end(), [](const Player& a, const Player& b) { 
			return a.get_ranking_score() > b.get_ranking_score(); 
		});

		// Create a mstch array of the player's match history with each opponent
		mstch::array match_history_data;
		int index = 0;
		for (const auto& opponent : opponents)
		{
			// Load info about opponent
			string opponent_id = opponent.get_id();
			string opponent_tag = opponent.get_tag();
			int opponent_rank = opponent.get_rank();
			double opp_ranking_score = opponent.get_ranking_score();
			MatchRecord match_record = match_history.at(player_id).at(opponent_id);
			int wins = match_record.get_wins();
			int losses = match_record.get_losses();

			// Determine whether opponent's match history should be included
			bool include_match_history = false;
			if (wins > 0 && ((raw_ranking_score - opp_ranking_score) < 1.0))
			{
				include_match_history = true;
			}
			else if (losses > 0 && ((opp_ranking_score - raw_ranking_score) < 1.0))
			{
				include_match_history = true;
			}

			// Add the opponent's match history to the array
			if (include_match_history)
			{
				// Write opponent's rank as a string
				string opponent_rank_string = to_string(opponent_rank);
				if (hide_unranked_ranks && opponent_rank > num_players_to_write)
				{
					opponent_rank_string = "unranked";
				}

				match_history_data.push_back(mstch::map{
					{"add_break", index > 0},
					{"wins", wins},
					{"losses", losses},
					{"opponent", opponent_tag},
					{"opp_rank", opponent_rank_string}
					});
				++index;
			}
		}

		players_data.push_back(mstch::map{
			{"tag", player_tag},
			{"id", player_id},
			{"rank", to_string(i+1)},
			{"ranking_score", format("{:.2f}", ranking_score)},
			{"uncertainty", format("{:.2f}", uncertainty)},
			{"volatility", volatility_string},
			{"include_delta_body", include_delta},
			{"delta", delta},
			{"collapse_id", to_string(i)},
			{"match_history", match_history_data}
		});
	}

	// Render the template
	string rendered_html = mstch::render(template_string, mstch::map{
		{"title", title},
		{"include_delta_header", include_delta},
		{"players", players_data}
	});

	// Create full output file path
	string full_output_file_path = output_file_path;
	if (local_directory != "")
	{
		full_output_file_path = local_directory + "/" + output_file_path;
	}

	// Write the rendered template to the output file
	ofstream output_file(full_output_file_path);
	output_file << rendered_html;
	output_file.close();
}
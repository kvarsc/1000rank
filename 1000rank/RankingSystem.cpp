#include "RankingSystem.h"

void RankingSystem::load_players(DatabaseManager& db_manager)
{
	auto player_data = db_manager.fetch_all_players();
	for (const auto& [id, name, ranking_score, uncertainty, volatility] : player_data)
	{
		players.emplace(id, Player(id, name, ranking_score, uncertainty, volatility));
	}
}

void RankingSystem::load_match_history(DatabaseManager& db_manager)
{
	auto set_data = db_manager.fetch_all_sets();
	for (const auto &[p1_id, p2_id, winner_id] : set_data)
	{
		add_match_record(p1_id, p2_id, winner_id);
	}
}

void RankingSystem::add_match_record(const string& p1_id, const string& p2_id, const string& winner_id)
{
	string loser_id = (p1_id == winner_id) ? p2_id : p1_id;

	// Update the match record for the winner
	match_history[winner_id][loser_id].add_win();

	// Update the match record for the loser
	match_history[loser_id][winner_id].add_loss();
}

void RankingSystem::calc_forces_for_all_match_records()
{
	for (auto& player_match_history : match_history)
	{
		for (auto& opponent_match_record : player_match_history.second)
		{
			opponent_match_record.second.calc_forces();
		}
	}
}

void RankingSystem::load_all(DatabaseManager& db_manager)
{
	cout << "Loading players..." << endl;
	load_players(db_manager);
	cout << "Loading match history..." << endl;
	load_match_history(db_manager);
	cout << "Calculating match record forces..." << endl;
	calc_forces_for_all_match_records();

	pair<int,int> match_history_size = get_match_history_size();

	cout << "Loaded " << players.size() << " players, " << match_history_size.first << " unique match records, and " << match_history_size.second << " total sets." << endl;
}

pair<int, int> RankingSystem::get_match_history_size()
{
	int total_match_records = 0;
	int total_sets = 0;

	for (const auto& player_match_history : match_history)
	{
		total_match_records += player_match_history.second.size();
		for (const auto& opponent_match_record : player_match_history.second)
		{
			total_sets += opponent_match_record.second.get_wins();
			total_sets += opponent_match_record.second.get_losses();
		}
	}

	total_match_records /= 2;
	total_sets /= 2;

	return make_pair(total_match_records, total_sets);
}

void RankingSystem::calc_forces_on_all_players()
{
	// First find the maximum ranking_score and zero out all forces
	double max_ranking_score = 1.0;
	for (auto& player : players)
	{
		player.second.zero_del();
		if (player.second.get_ranking_score() > max_ranking_score)
		{
			max_ranking_score = player.second.get_ranking_score();
		}
	}

	// Now iterate through all match records and calculate forces on each player
	for (const auto& player_match_history : match_history)
	{
		string player_id = player_match_history.first;
		double player_ranking_score = players[player_id].get_ranking_score();
		players[player_id].add_to_del(-player_ranking_score / max_ranking_score); // minor attendance bonus

		// Iterate through all opponents and calculate forces on the player
		for (const auto& opponent_match_record : player_match_history.second)
		{
			string opponent_id = opponent_match_record.first;
			double opponent_ranking_score = players[opponent_id].get_ranking_score();
			double win_force = opponent_match_record.second.get_win_force();

			// Calculate the win force on the player from this opponent
			// Add it to the player's del and subtract it from the opponent's del
			double net_force_wins = calc_net_force(win_force, opponent_ranking_score, player_ranking_score);
			players[player_id].add_to_del(net_force_wins);
			players[opponent_id].add_to_del(-net_force_wins);
		}
	}
}

double RankingSystem::calc_net_force(double force, double score1, double score2)
{
	if (force <= 0) return 0;

	return score1 > score2 ? force : std::max(0.0, force * (score1 - score2 + 1));
}

void RankingSystem::compute_rankings()
{
	cout << "Computing rankings..." << endl;

	// Zero out ranking scores, uncertainties, and volatilities
	for (auto& player : players)
	{
		player.second.zero_ranking_score();
		player.second.zero_uncertainty();
		player.second.zero_volatility();
	}

	// Declare initial variables for the ranking loop
	int steps = 0;
	int steps_since_nonpos_pe = 0;
	double a = astart;

	// Calculate the initial forces on all players
	calc_forces_on_all_players();

	// Main ranking loop
	while (deldel() > force_threshold)
	{
		// Increase dtmax every dtmax_freq steps
		// And print deldel and dtmax in scientific notation
		if (steps % dtmax_freq == 0)
		{
			dtmax *= dtmax_inc;
			cout << "deldel: " << scientific << deldel() << " dtmax: " << dtmax << fixed << endl;
		}

		// Move ranking scores in the direction of del and vel
		for (auto& player : players)
			player.second.move_ranking_score(sig);

		// Store del into delold for each player
		for (auto& player : players)
			player.second.store_del();

		// Calculate the new forces on all players
		calc_forces_on_all_players();

		// Calculate the new velocities for each player
		for (auto& player : players)
			player.second.calc_vel(sig);

		// Calculate pe, the dot product of del and vel
		double pe = delvel();

		// Calculate the norms of del and vel
		double delnorm = sqrt(deldel());
		double velnorm = sqrt(velvel());

		// Set vel in accordance with FIRE
		for (auto& player : players)
			player.second.fire_vel(a, delnorm, velnorm);

		// Change a and sig depending on pe's sign
		if (pe > 0)
		{
			steps_since_nonpos_pe++;
			if (steps_since_nonpos_pe > nmin)
			{
				a = a * fa;
				sig = std::min(sig * finc, dtmax);
			}
		}
		else
		{
			steps_since_nonpos_pe = 0;
			a = astart;
			sig = sig * fdec;
			// Also set velocities to zero
			for (auto& player : players)
				player.second.zero_vel();
		}

		// Increase the step counter
		steps++;
	}
}

double RankingSystem::deldel()
{
	double sum = 0.0;
	for (const auto& player : players)
	{
		sum += player.second.get_del() * player.second.get_del();
	}
	return sum;
}

double RankingSystem::delvel()
{
	double sum = 0.0;
	for (const auto& player : players)
	{
		sum += player.second.get_del() * player.second.get_vel();
	}
	return sum;
}

double RankingSystem::velvel()
{
	double sum = 0.0;
	for (const auto& player : players)
	{
		sum += player.second.get_vel() * player.second.get_vel();
	}
	return sum;
}

void RankingSystem::sort_players_by_ranking()
{
	cout << "Sorting players by ranking..." << endl;

	sorted_players.clear();

	for (auto &[id, player] : players)
		sorted_players.push_back(ref(player));

	sort(sorted_players.begin(), sorted_players.end(), [](const auto& a, const auto& b) {
		return a.get().get_ranking_score() > b.get().get_ranking_score();
	});

	// Store the maximum ranking score
	max_ranking_score = sorted_players[0].get().get_ranking_score();

	// Store rankings in player objects
	for (int i = 0; i < sorted_players.size(); i++)
		sorted_players[i].get().set_rank(i + 1);
}

void RankingSystem::compute_uncertainties()
{
	cout << "Computing uncertainties..." << endl;
	// Calculate the uncertainty for each player
	for (const auto& player_match_history : match_history)
	{
		string player_id = player_match_history.first;
		double player_ranking_score = players[player_id].get_ranking_score();
		players[player_id].add_to_uncertainty( 0.5 / (2 * max_ranking_score) ); // minor attendance bonus

		// Iterate through all opponents and calculate uncertainties on the player
		for (const auto& opponent_match_record : player_match_history.second)
		{
			string opponent_id = opponent_match_record.first;
			double opponent_ranking_score = players[opponent_id].get_ranking_score();
			double win_force = opponent_match_record.second.get_win_force();

			// Calculate the win force's contribution to the uncertainty
			if ( ( player_ranking_score > opponent_ranking_score ) && ( ( player_ranking_score - opponent_ranking_score ) < 1.0 ) )
			{
				players[player_id].add_to_uncertainty(win_force);
				players[opponent_id].add_to_uncertainty(win_force);
			}
		}
	}
}

void RankingSystem::subtract_uncertainties(double uncertainty_factor)
{
	cout << "Subtracting uncertainties..." << endl;

	// Subtract the uncertainty from each player's ranking score
	for (auto& player : players)
	{
		double ranking_score = player.second.get_ranking_score();
		double uncertainty = player.second.get_uncertainty();
		player.second.set_ranking_score(ranking_score - uncertainty_factor / uncertainty);
	}
}

void RankingSystem::compute_volatilities()
{
	cout << "Computing volatilities..." << endl;

	// Step 1: Store upset scores and downset scores for each player
	unordered_map<string, unordered_map<string, double>> upset_scores;
	unordered_map<string, unordered_map<string, double>> downset_scores;

	for (const auto& match_history_entry : match_history)
	{
		string player_id = match_history_entry.first;
		const Player& player = players[player_id];
		const auto& opponents = match_history_entry.second;

		for (const auto& opponent_match_record : opponents)
		{
			string opponent_id = opponent_match_record.first;
			const Player& opponent = players[opponent_id];
			const MatchRecord& match_record = opponent_match_record.second;

			double win_force = match_record.get_win_force();
			double loss_force = match_record.get_loss_force();

			if (win_force > 0)
			{
				if (is_upset(player, opponent))
					upset_scores[player_id][opponent_id] = win_force;
				else
					downset_scores[player_id][opponent_id] = win_force;
			}
			if (loss_force > 0)
			{
				if (is_upset(opponent, player))
					upset_scores[player_id][opponent_id] = loss_force;
				else
					downset_scores[player_id][opponent_id] = loss_force;
			}
		}
	}

	// Step 2: Calculate the volatility for each player
	for (const auto& player_entry : players)
	{
		string player_id = player_entry.first;
		double player_score = player_entry.second.get_ranking_score();
		double volatility = 0.0;

		// Iterate through all opponents and calculate volatilities on the player
		for (const auto& upset_entry : upset_scores[player_id])
		{
			string opponent_id = upset_entry.first;
			double upset_score = upset_entry.second;
			double opponent_score = players[opponent_id].get_ranking_score();
			double score_difference = std::abs(opponent_score - player_score);
			double sum_downset_scores = 0.0;

			for (const auto& downset_entry : downset_scores[player_id])
			{
				string other_oppoenent_id = downset_entry.first;
				double downset_score = downset_entry.second;
				double other_opponent_score = players[other_oppoenent_id].get_ranking_score();
				double other_score_difference = std::abs(opponent_score - other_opponent_score);

				if (other_score_difference < 1.0)
					sum_downset_scores += downset_score * (1.0 - other_score_difference);
			}

			volatility += score_difference * upset_score / (upset_score + sum_downset_scores);
		}

		players[player_id].set_volatility(volatility);
	}
}

void RankingSystem::store_rankings(DatabaseManager& db_manager)
{
	cout << "Storing rankings..." << endl;
	db_manager.update_player_ranking_values(players);
}

void RankingSystem::print_top_players(int n)
{
	cout << "Printing top " << n << " players..." << endl;
	for (int i = 0; i < n; i++)
	{
		double ranking_score = sorted_players[i].get().get_ranking_score();
		double uncertainty = sorted_players[i].get().get_uncertainty();
		ranking_score = ranking_score * ( scaling_factor / max_ranking_score );
		uncertainty = ( 1.0 / uncertainty ) * ( scaling_factor / max_ranking_score );
		cout << i + 1 << ". " << sorted_players[i].get().get_tag() << " (" << ranking_score << "; " << uncertainty << ")" << endl;
	}
}

void RankingSystem::compute_ranking_deltas(string local_directory, string previous_ranking_period_html, vector<string> earlier_ranking_period_htmls)
{
	cout << "Computing ranking deltas..." << endl;

	// Parse the previous ranking period's HTML
	RankingHtmlParser previous_ranking_results(local_directory, previous_ranking_period_html);

	// Parse the earlier ranking period HTMLs
	vector<RankingHtmlParser> earlier_ranking_results;
	for (const auto& earlier_ranking_period_html : earlier_ranking_period_htmls)
		earlier_ranking_results.push_back(RankingHtmlParser(local_directory, earlier_ranking_period_html));

	// Get the ID to rank mapping from the previous ranking period
	unordered_map<string, int> previous_ranking_id_to_rank = previous_ranking_results.get_id_to_rank_map();

	// Iterate through all players and compute their ranking deltas
	for (const auto& player_entry : players)
	{
		string player_id = player_entry.first;
		int player_rank = player_entry.second.get_rank();
		if (previous_ranking_id_to_rank.count(player_id))
		{
			int previous_rank = previous_ranking_id_to_rank[player_id];
			int ranking_delta = previous_rank - player_rank;
			string ranking_delta_string = to_string(ranking_delta);
			if(ranking_delta > 0)
				ranking_delta_string = "+" + ranking_delta_string;
			players[player_id].set_delta(ranking_delta_string);
		}
		else
		{
			players[player_id].set_delta("NEW");
			// Check if the player was in an earlier ranking period
			for (const auto& earlier_ranking_result: earlier_ranking_results)
			{
				unordered_map<string, int> earlier_ranking_id_to_rank = earlier_ranking_result.get_id_to_rank_map();
				if (earlier_ranking_id_to_rank.count(player_id))
				{
					players[player_id].set_delta("BACK");
					break;
				}
			}
		}
	}
}
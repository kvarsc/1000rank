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
	}/*
		double player_del = -player_ranking_score / max_ranking_score; // minor attendance bonus

		// Iterate through all opponents and calculate forces on the player
		for (const auto& opponent_match_record : player_match_history.second)
		{
			string opponent_id = opponent_match_record.first;
			double opponent_ranking_score = players[opponent_id].get_ranking_score();
			double win_force = opponent_match_record.second.get_win_force();
			double loss_force = opponent_match_record.second.get_loss_force();

			// Calculate the force on the player from this opponent
			double net_force_wins = calc_net_force(win_force, opponent_ranking_score, player_ranking_score);
			double net_force_losses = calc_net_force(loss_force, player_ranking_score, opponent_ranking_score);
			player_del += net_force_wins - net_force_losses;
		}

		players[player_id].set_del(player_del);
	}
	*/
}

double calc_net_force(double force, double score1, double score2)
{
	if (force <= 0) return 0;

	return score1 > score2 ? force : std::max(0.0, force * (score1 - score2 + 1));
}
#pragma once
#include <string>

using namespace std;

class Player
{
public:
	Player (string id, string tag, double ranking_score, double uncertainty, double volatility) : 
		id(id), tag(tag), ranking_score(ranking_score), uncertainty(uncertainty), volatility(volatility), del(0.0), delold(0.0), vel(0.0) {}

	// Getters
	string get_id() const { return id; }
	string get_tag() const { return tag; }
	double get_ranking_score() const { return ranking_score; }
	double get_uncertainty() const { return uncertainty; }
	double get_volatility() const { return volatility; }
	double get_del() const { return del; }
	double get_delold() const { return delold; }
	double get_vel() const { return vel; }

	// Setters
	void set_ranking_score(double new_ranking_score) { ranking_score = new_ranking_score; }
	void set_uncertainty(double new_uncertainty) { uncertainty = new_uncertainty; }
	void set_volatility(double new_volatility) { volatility = new_volatility; }
	void set_del(double new_del) { del = new_del; }
	void set_delold(double new_delold) { delold = new_delold; }
	void set_vel(double new_vel) { vel = new_vel; }
private:
	// ID info
	string id;
	string tag;

	// Ranking info
	double ranking_score;
	double uncertainty;
	double volatility;

	// Algorithm info
	double del; // force (delta) of the current time step
	double delold; // force (delta) of the previous time step
	double vel; // velocity
};


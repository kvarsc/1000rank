#pragma once

class MatchRecord
{
public:
	MatchRecord() : wins(0), losses(0), win_force(0), loss_force(0) {}

	int get_wins() const { return wins; }
	int get_losses() const { return losses; }
	double get_win_force() const { return win_force; }
	double get_loss_force() const { return loss_force; }

	void add_win() { ++wins; }
	void add_loss() { ++losses; }

	// IMPORTANT: this function converts wins and losses to force values
	// You can adjust this function to change the way forces are calculated!
	void calc_forces();

private:
	int wins;
	int losses;

	double win_force;
	double loss_force;
};


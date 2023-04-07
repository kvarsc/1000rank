#include "MatchRecord.h"

void MatchRecord::calc_forces()
{
	int max_wl = (wins > losses) ? wins : losses;
	double sum = 0.0;
	for (int i = 1; i <= max_wl; ++i) sum += 1.0 / i;
	win_force = (wins / static_cast<double>(max_wl)) * sum;
	loss_force = (losses / static_cast<double>(max_wl)) * sum;
}
#pragma once
#include <vector>


class Simulation {
public:
	const double commission_persent;
	Simulation(const double _commission_persent) : commission_persent(_commission_persent) {}

	// конструктор копирования
	Simulation(const Simulation& ref_Point)
		: window(ref_Point.window), current_point(ref_Point.current_point), commission_persent(ref_Point.commission_persent)
	{}

	void step(double new_cost) {
		if (!window.size()) throw;
		window.push_back(new_cost);
		window.erase(window.begin());
		if (!window.size()) throw;
		++current_point;
	}

	int window_size() const {
		return window.size();
	}
	void set_window(const std::vector<double>& inV) {
		window = inV;
	}
	std::vector<double> get_window() const {
		return window;
	}

	std::pair<double, double> calculate_stats_distribution() {
		auto& sample = window;
		//statistics 
		double mean = 0;
		double variance = 0;
		int n = sample.size();
		if (n < 2) throw;
		for (int i = 1; i < n; ++i)
		{
			double diff = sample[i] - sample[i - 1];
			mean += diff;
		}
		mean /= n;
		for (int i = 1; i < n; ++i)
		{
			double diff = sample[i] - sample[i - 1];
			variance += (diff - mean) * (diff - mean);
		}
		variance /= n - 1;
		return { mean, variance };
	}

	double current_cost() const {
		return window.end()[-1];
	}

	int get_current_point() const {
		return current_point;
	}

private:
	std::vector<double> window;
	int current_point = 0;

};

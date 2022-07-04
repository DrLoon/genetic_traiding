#pragma once
#include <vector>
#include <string>



class Simulation {
	using vectorD = std::vector<double>;
	using vectorI = std::vector<int>;

public:
	const double commission_persent;
	const int timestep;

	Simulation(const vectorD& _dataset, const double _commission_persent, const int _timestep)
		: dataset(_dataset), commission_persent(_commission_persent), timestep(_timestep)
	{
	}

	void step() {
		current_point++;
	}

	int dataset_size() const {
		return dataset.size();
	}

	vectorD last_n_costs(const int n) const {
		auto end = dataset.begin() + current_point;
		return vectorD(std::max(end - n, dataset.begin()), end);
	}

	double current_cost() const {
		return dataset[current_point];
	}

	int get_current_point() const {
		return current_point;
	}

	bool end() const {
		return current_point == (dataset.size() - 1);
	}

	void waste_points(const int n) {
		for (int i = 0; i < n; ++i)
			step();
	}

private:
	const vectorD& const dataset;

	int current_point = 0;
};

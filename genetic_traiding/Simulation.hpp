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
		replace_data.reserve(100);
	}

	// конструктор копирования
	Simulation(const Simulation& ref_Point)
		: dataset(ref_Point.dataset), commission_persent(ref_Point.commission_persent), timestep(ref_Point.timestep), 
		replace(ref_Point.replace), replace_data(ref_Point.replace_data), replace_start(ref_Point.replace_start), replace_end(ref_Point.replace_end),
		current_point(ref_Point.current_point)
	{

	}

	void step() {
		current_point++;
	}

	int dataset_size() const {
		return dataset.size();
	}

	vectorD last_n_costs(const int n) const {
		/*std::cout << "LAST N COSTs: " << "\n";
		std::cout << "replace: " << replace << "\n";
		std::cout << "current_point: " << current_point << "\n";
		std::cout << "replace_start: " << replace_start << '\n';
		std::cout << "----------------------------------------------- " << "\n";*/
		if (!replace) {
			//std::cout << "1" << "\n";
			auto end = dataset.begin() + current_point;
			return vectorD(std::max(end - n, dataset.begin()), end);
		}
		if (current_point - n < replace_start && current_point >= replace_start) {
			//std::cout << "2" << "\n";
			vectorD copy;
			copy.reserve(n);
			for (int i = current_point - n; i < replace_start; ++i) {
				copy.push_back(dataset[i]);
			}
			for (int i = 0; i < current_point - replace_start; ++i) {
				copy.push_back(replace_data[i]);
			}
			return copy;
		}
		if (current_point - n >= replace_start) {
			//std::cout << "3" << "\n";
			int start = current_point - replace_start;
			auto end = replace_data.begin() + start;
			return vectorD(std::max(end - n, replace_data.begin()), end);
		}
	}

	double current_cost() const {
		/*std::cout << "CURRENT COST: " << "\n";
		std::cout << "replace: " << replace << "\n";
		std::cout << "current_point: " << current_point << "\n";
		std::cout << "replace_start: " << replace_start << '\n';
		std::cout << "----------------------------------------------------" << "\n";*/
		if (replace && current_point >= replace_start && current_point < replace_end) {
			return replace_data[current_point - replace_start];
		}
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

	void replace_cost(double x) {
		if (!replace) {
			replace = true;
			replace_start = current_point;
			replace_end = replace_start;
		}
		++replace_end;
		replace_data.push_back(x);
	}
	std::vector<double> replace_data;
private:
	const vectorD& const dataset;

	int current_point = 0;

	bool replace = false;
	int replace_start = -1;
	int replace_end = -1;
	
};

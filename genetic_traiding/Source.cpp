//#define PYPLT
#include<iostream>
#include<fstream>
#include<vector>
#include<algorithm>
#include<cmath>
#include<random>
#include<float.h>
#define NOMINMAX
#include<Windows.h>
#include"TradeAgent.hpp"
#include"Simulation.hpp"
#include"DataReader.hpp"
#include"MCTS_copy.hpp"


const int timestep = 1; // here 9 (every hour) or 1 (every day)

const double commission_persent = 0.0004;
const double train_persent = 1;

std::vector<double> cost_train;
std::vector<double> cost_test;





int get_max_action(std::vector<std::tuple<int, double, int>>& res) {
	double max_v = -99999999999;
	int max_ind = -1;
	std::cout << "MC res ";
	for (int i = 0; i < res.size(); ++i) {
		std::cout << std::get<1>(res[i]) << " | " << std::get<2>(res[i]) << "    ";
		if (max_v < std::get<1>(res[i])) {
			max_v = std::get<1>(res[i]);
			max_ind = i;
		}
	}
	std::cout << '\n';
	return std::get<0>(res[max_ind]);
}
void solveMC() {
	std::vector<double> start_window;
	int WINDOW_SIZE = 7 * timestep;
	for (int i = 0; i < WINDOW_SIZE; ++i) start_window.push_back(cost_train[i]);
	Simulation sim(commission_persent);
	sim.set_window(start_window);
	TradeAgent agent(sim, timestep, true);

	for (int t = WINDOW_SIZE; t < cost_train.size(); ++t) {
		agent.update_cost(cost_train[t]);
		MCTS mcts(1000, agent);
		auto res = mcts.run();
		int action = get_max_action(res);
		agent.step(action, true);
		std::cout << t << ": \t" << action << "\n";

		agent.print_results();
	}
	agent.post_print(true);
	agent.print_results();
}

int main() {
	clock_t read = clock();
	std::string timestep_str;

	if (timestep == 1)
		timestep_str = "day";
	else if (timestep == 9)
		timestep_str = "hour";
	else
		throw "not such thing";

	std::string file_name = "datasets/" + timestep_str + "/SBER.txt";

	const DataReader dr(file_name);
	std::tie(cost_train, cost_test) = dr.split_train_test(train_persent);

	clock_t start = clock();
	
	//training(6000);
	solveMC();

	clock_t now = clock();
	std::cout << (double)(now - start) / CLOCKS_PER_SEC << " sec\n";
	return 0;
}
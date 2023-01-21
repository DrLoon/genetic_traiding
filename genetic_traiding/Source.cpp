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
#include"LGenetic.hpp"
#include"NeuralN.hpp"
#include"TradeAgent.hpp"
#include"Simulation.hpp"
#include"DataReader.hpp"
#include"MCTS_copy.hpp"


const int timestep = 1; // here 9 (every hour) or 1 (every day)

bool show = false;
const int input_size = 7 * timestep;
const int output_size = 3;
const NeuralN MyNet_static = NeuralN(
	{ input_size, 25, 15, 5, output_size }, 
	{ NeuralN::RELU, NeuralN::RELU, NeuralN::RELU, NeuralN::SIGMOID }
);


const double commission_persent = 0.0004;
const double train_persent = 0.7;

std::vector<double> cost_train;
std::vector<double> cost_test;


const NeuralN new_NN_by_vec(const std::vector<double>& x) {
	NeuralN MyNet = MyNet_static;
	MyNet.read_weitghs(x);
	return MyNet;
}


double loss(std::vector<double>& x) {
	const NeuralN MyNet = new_NN_by_vec(x);
	Simulation sim(cost_test, commission_persent, timestep);

	TradeAgent agent(input_size, sim);

	agent.do_actions_sim();

	agent.print_results();
	if (show)
		agent.post_print(true);

	return agent.get_money();
}
//double trade_action(const std::vector<double>& x) {
//	const NeuralN MyNet = new_NN_by_vec(x);
//	Simulation sim(cost_train, commission_persent, timestep);
//
//	TradeAgent agent(input_size, MyNet, sim);
//
//	agent.do_actions_sim();
//
//	if (show) {
//		agent.post_print(true);
//		//agent.print_results(sim.current_cost());
//	}
//
//	return agent.fitness();
//}


//void training(int times) {
//	LGenetic model(128, MyNet_static.paramsNumber(), trade_action);
//	model.rand_population_uniform();
//	model.set_crossover(LGenetic::SPBX);
//	model.set_mutation(LGenetic::AM);
//	model.enable_multiprocessing(10);
//	//model.enable_avarage_fitness(10);
//	model.set_loss(loss);
//	model.learn(times);
//#ifdef PYPLT
//	model.show_plt_avarage();
//#endif
//
//	auto best = model.best_gene();
//
//	show = true;
//	trade_action(best);
//	loss(best);
//	show = false;
//}


int get_max_action(std::vector<std::tuple<int, double, int>>& res) {
	double max_v = -99999999999;
	int max_ind = -1;
	std::cout << "MC res ";
	for (int i = 0; i < res.size(); ++i) {
		std::cout << std::get<1>(res[i]) << " ";
		if (max_v < std::get<1>(res[i])) {
			max_v = std::get<1>(res[i]);
			max_ind = i;
		}
	}
	std::cout << '\n';
	return std::get<0>(res[max_ind]);
}
void solveMC() {
	Simulation sim(cost_train, commission_persent, timestep);
	TradeAgent agent(input_size, sim);
	int iter = 0;
	while (!agent.isDone()) {
		MCTS mcts(100, agent);
		auto res = mcts.run();
		int action = get_max_action(res);
		agent.step(action, true);
		std::cout << ++iter << ": \t" << action << "\n";
	}
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
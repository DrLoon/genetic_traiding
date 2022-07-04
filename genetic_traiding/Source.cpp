#include<iostream>
#include<fstream>
#include<vector>
#include<algorithm>
#include<cmath>
#include<random>
#include<float.h>
#include<Windows.h>
#include"LGenetic.h"
#include"NeuralN.hpp"
#include"TradeAgent.hpp"
#include"Simulation.hpp"
#include"DataReader.hpp"


const int timestep = 9; // here 9 (every hour) or 1 (every day)

bool show = false;
const int input_size = 7 * timestep;
const int output_size = 3;
const NeuralN MyNet_static = NeuralN(
	{ input_size, 40, 20, 10, output_size }, 
	{ NeuralN::RELU, NeuralN::RELU, NeuralN::RELU, NeuralN::RELU }
);


const double commission_persent = 0.0004;
const double train_persent = 0.7;

std::vector<double> cost_train;
std::vector<double> cost_test;


const NeuralN new_NN_by_vec(std::vector<double>& x) {
	NeuralN MyNet = MyNet_static;
	MyNet.read_weitghs(x);
	return MyNet;
}


double loss(std::vector<double>& x, std::string file_validation) {
	const NeuralN MyNet = new_NN_by_vec(x);
	Simulation sim(cost_test, commission_persent, timestep);

	TradeAgent agent(input_size, MyNet, sim);

	agent.do_actions_sim();

	agent.print_results();
	if (show)
		agent.post_print(true);

	return agent.get_money();
}
double trade_action(std::vector<double>& x) {
	const NeuralN MyNet = new_NN_by_vec(x);
	Simulation sim(cost_train, commission_persent, timestep);

	TradeAgent agent(input_size, MyNet, sim);

	agent.do_actions_sim();

	if (show) {
		agent.post_print(true);
		//agent.print_results(sim.current_cost());
	}

	return agent.fitness();
}

void do_it() {
	int gene_length = MyNet_static.paramsNumber();
	int pop_size = 128;
	LGenetic Model
	(
		pop_size,
		gene_length,
		trade_action
	);

	Model.rand_population_normal();
	Model.set_crossover(LGenetic::SPBX);
	Model.set_mutation(LGenetic::AM);
	Model.set_loss(loss);
	Model.learn(10);

	auto best = Model.best_gene();
	show = true;
	trade_action(best);
	loss(best, "heh");
	show = false;
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

	std::string file_name = "datasets/" + timestep_str + "/MGNT.txt";

	const DataReader dr(file_name);
	std::tie(cost_train, cost_test) = dr.split_train_test(train_persent);

	clock_t start = clock();
	
	do_it();

	clock_t now = clock();
	std::cout << (double)(now - start) / CLOCKS_PER_SEC << " sec\n";
	return 0;
}
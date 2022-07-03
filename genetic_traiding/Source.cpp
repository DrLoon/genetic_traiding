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

const int timestep = 9; //тут 9(каждый час) или 1(каждый день)

bool show = false;
const int input_size = 7 * timestep;
const int output_size = 3;
const NeuralN MyNet_static = NeuralN(
	{ input_size, 25, 15, 5, output_size }, 
	{ NeuralN::RELU, NeuralN::RELU, NeuralN::RELU, NeuralN::SIGMOID }
);


double commission_persent = 0.0004;
double train_persent = 0.7;

int dataset_size;
int train_size;
int test_size;

std::vector<double> cost_train;
std::vector<double> cost_test;

bool TEST = false;

const NeuralN new_NN_by_vec(std::vector<double>& x) {
	NeuralN MyNet = MyNet_static;
	MyNet.read_weitghs(x);
	return MyNet;
}


double loss(std::vector<double>& x, std::string file_validation) {
	const NeuralN MyNet = new_NN_by_vec(x);

	Simulation sim(cost_test);
	TradeAgent agent(input_size, MyNet);

	sim.waste_points(input_size);
	while (!sim.end()) {
		sim.step();
		auto sample = sim.last_n_costs(input_size);
		int action = agent.get_action(sample);
		switch (action)
		{
		case 0:
			agent.nothing();
			break;
		case 1:
			agent.buy(sim.current_cost(), sim.get_current_point(), commission_persent);
			break;
		case 2:
			agent.sell(sim.current_cost(), sim.get_current_point(), commission_persent);
			break;
		default:
			throw "bad action";
		}
		if (sim.get_current_point() % (30 * timestep))
			agent.update_month();

	}
	agent.post_stuff(test_size);
	agent.print_results(sim.current_cost());
	if (show)
		agent.post_print(sim.current_cost(), test_size, timestep, true);

	return agent.get_money();
}
double trade_action(std::vector<double>& x) {
	const NeuralN MyNet = new_NN_by_vec(x);

	Simulation sim(cost_train);
	TradeAgent agent(input_size, MyNet);

	sim.waste_points(input_size);
	while (!sim.end()) {
		sim.step();
		auto sample = sim.last_n_costs(input_size);
		int action = agent.get_action(sample);
		switch (action)
		{
		case 0:
			agent.nothing();
			break;
		case 1:
			agent.buy(sim.current_cost(), sim.get_current_point(), commission_persent);
			break;
		case 2:
			agent.sell(sim.current_cost(), sim.get_current_point(), commission_persent);
			break;
		default:
			throw "bad action";
		}
		if (sim.get_current_point() % (30 * timestep))
			agent.update_month();
	}
	agent.post_stuff(train_size);
	if (show)
		agent.post_print(sim.current_cost(), train_size, timestep, false);
		//agent.print_results(sim.current_cost());

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
	switch (timestep)
	{
	case 1:
		timestep_str = "day";
		break;
	case 9:
		timestep_str = "hour";
		break;
	default:
		timestep_str = "day";
		break;
	}
	std::string file_name = "datasets/" + timestep_str + "/SBER.txt";
	std::fstream dataset_file(file_name);

	//dataset size culculating
	std::fstream count_file_copy(file_name);
	std::istreambuf_iterator<char> begin(count_file_copy), end;
	dataset_size = (int)std::count(begin, end, char('\n')) - 1;
	train_size = (int)(dataset_size * train_persent);
	test_size = dataset_size - train_size;
	cost_train.resize(train_size);
	cost_test.resize(test_size);

	//dataset reading
	std::string first_str;
	dataset_file >> first_str;
	std::cout << "start reading, first word is [ " << first_str << " ], dataset size is " << dataset_size << " [train " << train_size << " and test "<< test_size << "]\n";
	for (int i = 0; i < train_size; i++)
	{
		dataset_file >> cost_train[i];
	}
	for (int i = 0; i < test_size; i++)
	{
		dataset_file >> cost_test[i];
	}
	std::cout << "reading is done, time is " << (double)(clock() - read) / CLOCKS_PER_SEC << " sec\n";

	clock_t start = clock();
	
	do_it();

	clock_t now = clock();
	std::cout << (double)(now - start) / CLOCKS_PER_SEC << " sec\n";
	return 0;
}
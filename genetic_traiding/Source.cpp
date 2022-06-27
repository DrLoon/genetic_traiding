#include<iostream>
#include<fstream>
#include<vector>
#include<algorithm>
#include<cmath>
#include<random>
#include"LGenetic.h"
#include "NeuralN.hpp"


bool show = false;
const int input_size = 7*24;
const int output_size = 3;
const NeuralN MyNet_static = NeuralN({ input_size, 25, 15, 5, output_size }, { NeuralN::RELU, NeuralN::RELU, NeuralN::RELU, NeuralN::SIGMOID });

int dataset_size = 8300;
std::vector<double> cost_history(dataset_size);
int test_size = 20000;
std::vector<double> test_history(test_size);

bool TEST = false;

const NeuralN new_NN_by_vec(std::vector<double>& x) {
	NeuralN MyNet = MyNet_static;
	MyNet.read_weitghs_from_vector(x);
	return MyNet;
}

void normolize(const int start, const int end, std::vector<double>& x) {
	double Min = 99999999;
	double Max = 0;
	for (auto& i : x) {
		Min = std::min(Min, i);
		Max = std::max(Max, i);
	}
	/*Min = 0;
	Max = 300;*/
	for (auto& i : x) {
		i = (i - Min) / (Max - Min);
	}
}
int get_action(std::vector<double>& ans) {
	int action = 0;
	for (int j = 0; j < ans.size(); ++j)
		if (ans[j] > ans[action]) action = j;
	return action;
}
void nothing(int ignor_days) {
	ignor_days++;
}
void buy(double& money, const double& cost, std::vector<int>& buy_days, const int& end, int& amount_stocks) {
	if (money > cost) {
		money -= cost;
		amount_stocks += 1;
		buy_days.push_back(end);
	}
}
void sell(int& amount_stocks, double& money, const double& cost, const double& active_money, const int window, std::vector<int>& sell_days, int& hangry_days, const int end, double& storage, std::vector<int>& storage_days) {
	if (amount_stocks >= 1) {
		money += cost;
		if (money > active_money) {
			storage += money - active_money;
			money = active_money;

			//storage_days.push_back(end);
			//if (storage_days.size() > 1)
			//	hangry_days = std::max(hangry_days, storage_days[storage_days.size() - 1] - storage_days[storage_days.size() - 2]);
			//else
			//	hangry_days = storage_days[0];
		}
		amount_stocks -= 1;
		sell_days.push_back(end);
		if (sell_days.size() > 1)
			hangry_days = std::max(hangry_days, sell_days[sell_days.size() - 1] - sell_days[sell_days.size() - 2]);
		else
			hangry_days = sell_days[0] - window;
	}
}



double loss(std::vector<double>& x, std::string file_validation) {
	const NeuralN MyNet = new_NN_by_vec(x);

	int ignor_days = 0;
	int hangry_days = -1;
	std::vector<int> buy_days;
	std::vector<int> sell_days;
	std::vector<int> storage_days;
	double money = 10000;
	double active_money = money;
	double storage = 0;
	int amount_stocks = 0;
	const int window = input_size;


	for (int i = window; i < test_history.size(); ++i) {
		int start = i - window;
		int end = i - 1;
		std::vector<double> sample(test_history.begin() + start, test_history.begin() + end + 1);
		normolize(start, end, sample);
		auto ans = MyNet.forward(sample);
		double cost = test_history[end];
		int action = get_action(ans);
		switch (action)
		{
		case 0:
			nothing(ignor_days);
			break;
		case 1:
			buy(money, cost, buy_days, end, amount_stocks);
			break;
		case 2:
			sell(amount_stocks, money, cost, active_money, window, sell_days, hangry_days, end, storage, sell_days);
			break;
		default:
			break;
		}
	}
	if (hangry_days == -1) hangry_days = test_size;
	else hangry_days = std::max(hangry_days, test_size - sell_days[sell_days.size() - 1]);

	if (show) {
		std::cout << "[";
		for (auto i : buy_days) {
			std::cout << i << ", ";
		}
		std::cout << "]\n";

		std::cout << "\n[";
		for (auto i : sell_days) {
			std::cout << i << ", ";
		}
		std::cout << "]\n";

		std::cout << "hangry_days " << hangry_days << "\n";
		std::cout << "amount_stocks " << amount_stocks << "\n";
		std::cout << "money " << money << "\n";
		std::cout << "storage " << storage << "\n";
		
	}

	int money_in_stockes = amount_stocks * (*(test_history.end() - 1));
	std::cout << "\tstocks: " << money_in_stockes << " ";
	std::cout << "storage: " << storage << " ";
	std::cout << "summ: " << money_in_stockes + storage + money << " ";
	std::cout << "ff: " << -storage / (hangry_days) << " ";
	return money;
}

double trade_action(std::vector<double>& x) {
	const NeuralN MyNet = new_NN_by_vec(x);

	int ignor_days = 0;
	int hangry_days = -1;
	std::vector<int> buy_days;
	std::vector<int> sell_days;
	std::vector<int> storage_days;
	double money = 10000;
	double active_money = money;
	double storage = 0;
	int amount_stocks = 0;
	const int window = input_size;
	

	for (int i = window; i < cost_history.size(); ++i) {
		int start = i - window;
		int end = i - 1;
		std::vector<double> sample(cost_history.begin() + start, cost_history.begin() + end + 1);
		normolize(start, end, sample);
		auto ans = MyNet.forward(sample);
		double cost = cost_history[end];
		int action = get_action(ans);
		switch (action)
		{
		case 0:
			nothing(ignor_days);
			break;
		case 1:
			buy(money, cost, buy_days, end, amount_stocks);
			break;
		case 2:
			sell(amount_stocks, money, cost, active_money, window, sell_days, hangry_days, end, storage, sell_days);
			break;
		default:
			break;
		}
	}
	if (hangry_days == -1) hangry_days = dataset_size;
	//else hangry_days = std::max(hangry_days, dataset_size - storage_days[storage_days.size() - 1]);
	else hangry_days = std::max(hangry_days, dataset_size - sell_days[sell_days.size() - 1]);

	if (show) {
		std::cout << "buy = [";
		for (auto i : buy_days) {
			std::cout << i << ", ";
		}
		std::cout << "]\n";

		std::cout << "\n sell = [";
		for (auto i : sell_days) {
			std::cout << i << ", ";
		}
		std::cout << "]\n";

		std::cout << "hangry_days " << hangry_days << "\n";
		std::cout << "amount_stocks " << amount_stocks << "\n";
		std::cout << "money " << money << "\n";
		std::cout << "storage " << storage << "\n";
	}
	return -storage*10 - money + hangry_days*100 + amount_stocks*10;
}

void do_it() {
	int gene_length = MyNet_static.getParamsNumber();
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
	Model.file_validation = "testSBER.txt";
	Model.set_loss(loss);
	Model.learn(1000);

	auto best = Model.best_gene();
	show = true;
	trade_action(best);
	loss(best, "heh");
	show = false;
}

int main() {
	std::fstream sin("trainhourSBER.txt");
	for (int i = 0; i < dataset_size; i++)
	{
		sin >> cost_history[i];
	}

	std::fstream test("hourSBER.txt");
	for (int i = 0; i < test_size; i++)
	{
		test >> test_history[i];
	}
	clock_t start = clock();
	
	do_it();
	clock_t now = clock();
	std::cout << (double)(now - start) / CLOCKS_PER_SEC << " sec\n";
	return 0;
}
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

int timestep = 9; //тут 24(каждый час) или 1(каждый день)

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

void normolize(std::vector<double>& x) {
	double Min = DBL_MAX;
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
		money -= cost * commission_persent; //commission
		amount_stocks += 1;
		buy_days.push_back(end);
	}
}
void sell(int& amount_stocks, double& money, const double& cost, const double& active_money, const int window, std::vector<int>& sell_days, int& hangry_days, const int end, double& storage, std::vector<int>& storage_days, int& days_without_storage) {
	if (amount_stocks >= 1) {
		money += cost;
		money -= cost * commission_persent; //commission
		if (money > active_money) {
			storage += money - active_money;
			money = active_money;

			storage_days.push_back(end);
			if (storage_days.size() > 1)
				days_without_storage = std::max(days_without_storage, storage_days[storage_days.size() - 1] - storage_days[storage_days.size() - 2]);
			else
				days_without_storage = storage_days[0] - window;
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
	int days_without_storage = -1;
	std::vector<int> buy_days;
	std::vector<int> sell_days;
	std::vector<int> storage_days;
	double money = 10000;
	double active_money = money;
	double storage = 0;
	int amount_stocks = 0;
	const int window = input_size;


	for (int i = window; i < cost_test.size(); ++i) {
		int start = i - window;
		int end = i - 1;
		std::vector<double> sample(cost_test.begin() + start, cost_test.begin() + end + 1);
		normolize(sample);
		auto ans = MyNet.forward(sample);
		double cost = cost_test[end];
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
			sell(amount_stocks, money, cost, active_money, window, sell_days, hangry_days, end, storage, storage_days, days_without_storage);

			break;
		default:
			break;
		}
	}
	if (hangry_days == -1) hangry_days = test_size;
	else hangry_days = std::max(hangry_days, test_size - sell_days[sell_days.size() - 1]);

	if (days_without_storage == -1) days_without_storage = test_size;
	else days_without_storage = std::max(days_without_storage, test_size - storage_days[storage_days.size() - 1]);

	if (show) {
		/*std::cout << "[";
		for (auto i : buy_days) {
			std::cout << i << ", ";
		}
		std::cout << "]\n";

		std::cout << "\n[";
		for (auto i : sell_days) {
			std::cout << i << ", ";
		}
		std::cout << "]\n";*/


		std::cout << "\n\nTest dataset with amount of days " << test_size / timestep <<  " (" << test_size / timestep / 365 <<" years)\n";
		std::cout << "\thangry_days " << hangry_days / timestep << "\n";
		std::cout << "\tdays_without_storage " << days_without_storage / timestep << "\n";
		std::cout << "\tamount_stocks " << amount_stocks << " [price " << *(cost_test.end() - 1) << "] => " << *(cost_test.end() - 1) * amount_stocks << "\n";
		std::cout << "\tmoney " << money << "\n";
		std::cout << "\tstorage " << storage << "\n";
		std::cout << "\tsum: " << *(cost_test.end() - 1) * amount_stocks + storage + money << "\n\n";
		
	}
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	double money_in_stockes = amount_stocks * (*(cost_test.end() - 1));
	std::cout << "stocks: " << money_in_stockes << " ";
	std::cout << "storage: " << storage << " ";
	std::cout << "money: " << money << " ";
	
	double sum = money_in_stockes + storage + money;
	std::cout << "summ: ";
	if (sum > active_money)
		SetConsoleTextAttribute(hConsole, 2);
	else 
		SetConsoleTextAttribute(hConsole, 12);
	std::cout << sum << " ";

	SetConsoleTextAttribute(hConsole, 15);
	std::cout << "ff: " << storage / (hangry_days) << " ";
	return money;
}

double trade_action(std::vector<double>& x) {
	const NeuralN MyNet = new_NN_by_vec(x);

	int ignor_days = 0;
	int hangry_days = -1;
	int days_without_storage = -1;
	std::vector<int> buy_days;
	std::vector<int> sell_days;
	std::vector<int> storage_days;
	double money = 10000;
	double active_money = money;
	double storage = 0;
	int amount_stocks = 0;
	const int window = input_size;
	

	for (int i = window; i < cost_train.size(); ++i) {
		int start = i - window;
		int end = i - 1;
		std::vector<double> sample(cost_train.begin() + start, cost_train.begin() + end + 1);
		normolize(sample);
		auto ans = MyNet.forward(sample);
		double cost = cost_train[end];
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
			sell(amount_stocks, money, cost, active_money, window, sell_days, hangry_days, end, storage, storage_days, days_without_storage);
			break;
		default:
			break;
		}
	}
	if (hangry_days == -1) hangry_days = train_size;
	else hangry_days = std::max(hangry_days, train_size - sell_days[sell_days.size() - 1]);

	if (days_without_storage == -1) days_without_storage = train_size;
	else days_without_storage = std::max(days_without_storage, train_size - storage_days[storage_days.size() - 1]);
	

	if (show) {
		/*std::cout << "buy = [";
		for (auto i : buy_days) {
			std::cout << i << ", ";
		}
		std::cout << "]\n";

		std::cout << "\n sell = [";
		for (auto i : sell_days) {
			std::cout << i << ", ";
		}
		std::cout << "]\n";*/
		std::cout << "\n\nTrain dataset with amount of days " << train_size / timestep << " (" << train_size / timestep / 365 << " years)\n";
		std::cout << "\thangry_days " << hangry_days / timestep << "\n";
		std::cout << "\tdays_without_storage " << days_without_storage / timestep << "\n";
		std::cout << "\tamount_stocks " << amount_stocks << " [price " << *(cost_train.end() - 1) << "] => " << *(cost_train.end() - 1) * amount_stocks << "\n";
		std::cout << "\tmoney " << money << "\n";
		std::cout << "\tstorage " << storage << "\n";
		std::cout << "\tsum: " << *(cost_train.end() - 1) * amount_stocks + storage + money << "\n";
	}
	return -storage*10 - money + hangry_days*100 + amount_stocks*10;
	//return -storage;
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
	Model.learn(1000);

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
	case 24:
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
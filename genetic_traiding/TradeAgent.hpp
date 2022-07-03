#pragma once
#include <vector>
#include <string>

#include "NeuralN.hpp"

struct TradeCounters {
	using vectorD = std::vector<double>;
	using vectorI = std::vector<int>;

	int ignor_days = 0;
	int hungry_days = -1;
	int days_without_storage = -1;
	vectorI buy_days;
	vectorI sell_days;
	vectorI storage_days;
};


class TradeAgent {
	using vectorD = std::vector<double>;
	using vectorI = std::vector<int>;
public:
	const int id = 0;

	TradeAgent(int _window, const NeuralN& _nn, const int _id = 0) : WINDOW(_window), NN(_nn), id(_id) 
	{
	}

	void nothing() {
		tr_cntrs.ignor_days++;
	}
	bool buy(const double& cost, const int& end, const double commission_persent) {
		// TODO: add any count to buy
		// return success or not

		if (cost <= 0 || end < 0)
			throw "something wrong";
			
		if (money > cost) {
			money -= cost;
			money -= cost * commission_persent; //commission
			amount_stocks += 1;
			tr_cntrs.buy_days.push_back(end);

			return true;
		}
		else
			return false;
	}
	bool sell(const double& cost, const int end, const double commission_persent) {
		// TODO: add any count to sell
		// return success or not

		if (cost <= 0 || end < 0)
			throw "something wrong";

		if (amount_stocks > 0) {
			money += cost;
			money -= cost * commission_persent; //commission
			if (money > active_money_limit) {
				storage += money - active_money_limit;
				money = active_money_limit;

				tr_cntrs.storage_days.push_back(end);
				if (tr_cntrs.storage_days.size() > 1) {
					size_t st_size = tr_cntrs.storage_days.size();
					tr_cntrs.days_without_storage = std::max(tr_cntrs.days_without_storage, tr_cntrs.storage_days[st_size - 1] - tr_cntrs.storage_days[st_size - 2]);
				}
				else
					tr_cntrs.days_without_storage = tr_cntrs.storage_days[0] - WINDOW;
			}
			amount_stocks -= 1;
			tr_cntrs.sell_days.push_back(end);
			if (tr_cntrs.sell_days.size() > 1) {
				size_t st_size = tr_cntrs.sell_days.size();
				tr_cntrs.hungry_days = std::max(tr_cntrs.hungry_days, tr_cntrs.sell_days[st_size - 1] - tr_cntrs.sell_days[st_size - 2]);
			}
			else
				tr_cntrs.hungry_days = tr_cntrs.sell_days[0] - WINDOW;

			return true;
		}
		else
			return false;
	}


	double get_money() const {
		return money;
	}
	double get_storage() const {
		return storage;
	}

	int get_action(vectorD& input) const {
		normolize(input);
		auto a = NN.forward(input);
		return ans_to_action(a);
	}

	void post_stuff(int size) {
		if (tr_cntrs.hungry_days == -1) tr_cntrs.hungry_days = size;
		else tr_cntrs.hungry_days = std::max(tr_cntrs.hungry_days, size - tr_cntrs.sell_days[tr_cntrs.sell_days.size() - 1]);

		if (tr_cntrs.days_without_storage == -1) tr_cntrs.days_without_storage = size;
		else tr_cntrs.days_without_storage = std::max(tr_cntrs.days_without_storage, size - tr_cntrs.storage_days[tr_cntrs.storage_days.size() - 1]);
	}

	void print_results(double last_cost) {
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		double money_in_stockes = amount_stocks * last_cost;
		std::cout << "stocks: " << money_in_stockes << " ";
		std::cout << "storage: " << storage << " ";
		std::cout << "money: " << money << " ";

		double sum = money_in_stockes + storage + money;
		std::cout << "summ: ";
		if (sum > active_money_limit)
			SetConsoleTextAttribute(hConsole, 2);
		else
			SetConsoleTextAttribute(hConsole, 12);
		std::cout << sum << " ";

		SetConsoleTextAttribute(hConsole, 15);
		std::cout << "ff: " << storage / (tr_cntrs.hungry_days) << " ";
	}

	double fitness() {
		return -storage * 10 - money + tr_cntrs.hungry_days * 100 + amount_stocks * 10;
	}
	
private:
	const int WINDOW;
	const NeuralN NN;

	TradeCounters tr_cntrs;

	double money = 10000;
	const double active_money_limit = money;
	double storage = 0;

	int amount_stocks = 0;

	void normolize(vectorD& x) const {
		double Min = DBL_MAX;
		double Max = DBL_MIN;
		for (auto& i : x) {
			Min = std::min(Min, i);
			Max = std::max(Max, i);
		}
		/*Min = 0;
		Max = 300;*/
		double diff = Max - Min;
		for (auto& i : x) {
			i = (i - Min) / diff;
		}
	}
	int ans_to_action(const vectorD& ans) const {
		int action = 0;
		for (int j = 0; j < ans.size(); ++j)
			if (ans[j] > ans[action]) action = j;
		return action;
	}
};


class Simulation {
	using vectorD = std::vector<double>;
	using vectorI = std::vector<int>;

public:
	Simulation(const vectorD& _dataset) : dataset(_dataset) {

	}

	void step() {
		current_point++;
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
	const vectorD dataset;

	int current_point = 0;
};
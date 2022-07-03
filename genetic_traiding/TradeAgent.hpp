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
	int get_action(const vectorD& ans) const {
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

	vectorD last_n_points(int n) {
		auto m = dataset.begin() + current_point;
		return vectorD(m, std::min(m + n, dataset.end() - 1));
	}

	int get_current_point() {
		return current_point;
	}

private:
	const vectorD dataset;

	int current_point = 0;
};
#pragma once
#include <vector>
#include <string>

#include "NeuralN.hpp"
#include "Simulation.hpp"

struct TradeCounters {
	using vectorD = std::vector<double>;
	using vectorI = std::vector<int>;

	int ignor_days = 0;
	int hungry_days = -1;
	int days_without_storage = -1;
	vectorI buy_days;
	vectorI sell_days;
	vectorI storage_days;
	vectorI storage_every_month;
	vectorI storage_per_month;
	int trade_days = 0;
};


class TradeAgent {
	using vectorD = std::vector<double>;
	using vectorI = std::vector<int>;
public:
	const int id = 0;

	TradeAgent(int _window, const NeuralN& _nn, Simulation& _sim, const int _id = 0) : WINDOW(_window), NN(_nn), id(_id), sim(_sim)
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
			
		if (money > cost * (1 + commission_persent)) {
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

		if (amount_stocks > 0 && money > cost * commission_persent) {
			money += cost;
			money -= cost * commission_persent; //commission
			if (money > active_money_limit) {
				storage += money - active_money_limit;
				money = active_money_limit;

				tr_cntrs.storage_days.push_back(end);
				if (tr_cntrs.storage_days.size() > 1) {
					tr_cntrs.days_without_storage = std::max(tr_cntrs.days_without_storage, tr_cntrs.storage_days.end()[-1] - tr_cntrs.storage_days.end()[-2]);
				}
				else
					tr_cntrs.days_without_storage = tr_cntrs.storage_days[0] - WINDOW;
			}
			amount_stocks -= 1;
			tr_cntrs.sell_days.push_back(end);
			if (tr_cntrs.sell_days.size() > 1) {
				tr_cntrs.hungry_days = std::max(tr_cntrs.hungry_days, tr_cntrs.sell_days.end()[-1] - tr_cntrs.sell_days.end()[-2]);
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

	void update_month() {
		if (tr_cntrs.storage_every_month.size())
			tr_cntrs.storage_per_month.push_back(storage - tr_cntrs.storage_every_month.back());
		else
			tr_cntrs.storage_per_month.push_back(storage);

		tr_cntrs.storage_every_month.push_back(storage);
	}

	void post_stuff() {
		int size = sim.dataset_size();
		if (tr_cntrs.hungry_days == -1) tr_cntrs.hungry_days = size;
		else tr_cntrs.hungry_days = std::max(tr_cntrs.hungry_days, size - tr_cntrs.sell_days.back());

		if (tr_cntrs.days_without_storage == -1) tr_cntrs.days_without_storage = size;
		else tr_cntrs.days_without_storage = std::max(tr_cntrs.days_without_storage, size - tr_cntrs.storage_days.back());
	}

	void print_results() const {
		double last_cost = sim.current_cost();
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
		std::cout << "ff: " << fitness() << " ";
	}
	void post_print(bool month_storage) const {
		double last_cost = sim.current_cost();
		int size = sim.dataset_size();
		int timestep = sim.timestep;

		std::cout << "\n\nDataset with amount of days " << size / timestep << " (" << size / timestep / 365 << " years)\n";
		std::cout << "\thangry_days " << tr_cntrs.hungry_days / timestep << "\n";
		std::cout << "\tdays_without_storage " << tr_cntrs.days_without_storage / timestep << "\n";
		std::cout << "\tamount_stocks " << amount_stocks << " [price " << last_cost << "] => " << last_cost * amount_stocks << "\n";
		std::cout << "\tmoney " << money << "\n";
		std::cout << "\tstorage " << storage << "\n";
		std::cout << "\tsum: " << last_cost * amount_stocks + storage + money << "\n";
		if (month_storage) {
			std::cout << "\tstorage_per_month: ";
			for (auto& i : tr_cntrs.storage_per_month)
				std::cout << i << " ";
		}
		std::cout << "\n\n";
		//*(cost_test.end() - 1)
	}

	double fitness() const {
		vectorI a = tr_cntrs.storage_per_month;
		////const double uni = uniformity(a) * 100000000;
		const double uni = uniformity2(a) * 10000000;
		//return -storage * 10  + uniformity3(a);
		return -storage * 10 - money + (double)tr_cntrs.hungry_days * 100 + (double)amount_stocks * 10 + uni;
		double mean = 0;
		for (auto& i : tr_cntrs.storage_per_month)
			mean += i;
		mean /= tr_cntrs.storage_per_month.size();
		return mean;
	}
	
	void do_actions_sim() {
		sim.waste_points(WINDOW);
		while (!sim.end()) {
			auto sample = sim.last_n_costs(WINDOW);

			switch (get_action(sample))
			{
			case 0:
				nothing();
				break;
			case 1:
				buy(sim.current_cost(), sim.get_current_point(), sim.commission_persent);
				break;
			case 2:
				sell(sim.current_cost(), sim.get_current_point(), sim.commission_persent);
				break;
			default:
				throw "bad action";
				break;
			}

			if (sim.get_current_point() % (30 * sim.timestep) == 0)
				update_month();

			sim.step();
			tr_cntrs.trade_days++;
		}
		post_stuff();
	}

private:
	const int WINDOW;
	const NeuralN NN;
	Simulation& const sim;

	TradeCounters tr_cntrs;

	double money = 10000;
	const double active_money_limit = money;
	double storage = 0;

	int amount_stocks = 0;

	void normolize(vectorD& x) const {
		double Min = DBL_MAX;
		double Max = -DBL_MAX;
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
	vectorD normolize(const vectorI& x) const {
		int Min = INT_MAX;
		int Max = -INT_MAX;
		for (auto& i : x) {
			Min = std::min(Min, i);
			Max = std::max(Max, i);
		}
		/*Min = 0;
		Max = 300;*/
		int diff = Max - Min;
		if (diff == 0)
			return vectorD(x.size(), std::min(x[0], 1));
		vectorD res(x.size());
		for (int i = 0; i < x.size(); ++i) {
			res[i] = static_cast<double>(x[i] - Min) / diff;
		}
		return res;
	}
	int ans_to_action(const vectorD& ans) const {
		int action = 0;
		for (int j = 0; j < ans.size(); ++j)
			if (ans[j] > ans[action]) action = j;
		return action;
	}
	
	double uniformity(const vectorI x) const {
		const vectorD norm = normolize(x);
		double Min = DBL_MAX;
		double Max = -DBL_MAX;
		for (auto& i : norm) {
			Min = std::min(Min, i);
			Max = std::max(Max, i);
		}

		int not_null = 0;
		for (auto& i : x)
			if (i != 0)
				++not_null;
		return static_cast<double>(Max - Min + 1) / (not_null*10 + 1);
	}
	double uniformity2(const vectorI x) const {
		const vectorD norm = normolize(x);
		double mean = 0;
		for (auto& i : norm) {
			mean += i;
		}
		mean /= norm.size();
		double s_diff = 0;
		for (auto& i : norm) {
			s_diff += abs(i - mean);
		}

		int not_null = 0;
		for (auto& i : x)
			if (i != 0)
				++not_null;
		return static_cast<double>(s_diff + 1) / (not_null * 10 + 1);
	}
	double uniformity3(const vectorI x) const {
		const vectorD norm = normolize(x);
		double mean = 0;
		int not_null = 0;
		for (auto& i : norm) {
			mean += i;
			if (i) ++not_null;
		}
		if (mean == 0) return norm.size();
		mean /= not_null;

		double s_diff = 0;
		for (auto& i : norm) {
			s_diff += i ? abs(i - mean) : mean;
		}
		return s_diff;
	}
};

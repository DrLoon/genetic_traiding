#pragma once
#include <vector>
#include <string>

#include "NeuralN.hpp"
#include "Simulation.hpp"
#include "MCTS_copy.hpp"

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


class TradeAgent : public IEnviroment {
	using vectorD = std::vector<double>;
	using vectorI = std::vector<int>;
public:

	TradeAgent(Simulation _sim, const int _discretization, bool _save_cntrs = false) 
		: sim(_sim), save_cntrs(_save_cntrs), discretization(_discretization)
	{}
	// конструктор копирования
	TradeAgent(const TradeAgent& ref_Point) 
		: sim(ref_Point.sim), 
		mean_store(ref_Point.mean_store), variance_store(ref_Point.variance_store),
		last_storage(ref_Point.last_storage), month_done(ref_Point.month_done), last_cost(ref_Point.last_cost),
		money(ref_Point.money), storage(ref_Point.storage), amount_stocks(ref_Point.amount_stocks), discretization(ref_Point.discretization)
	{
		save_cntrs = false;
	}

	void nothing() {
		if (save_cntrs)
			tr_cntrs.ignor_days++;
	}
	bool buy(const double cost, const int end, const double commission_persent) {
		// TODO: add any count to buy
		// return success or not

		if (cost <= 0 || end < 0)
			throw "something wrong";
			
		if (money > cost * (1 + commission_persent)) {
			money -= cost;
			money -= cost * commission_persent; //commission
			amount_stocks += 1;

			if (save_cntrs)
				tr_cntrs.buy_days.push_back(end);

			return true;
		}
		else
			return false;
	}
	bool sell(const double cost, const int end, const double commission_persent) {
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

				if (save_cntrs) {
					tr_cntrs.storage_days.push_back(end);
					if (tr_cntrs.storage_days.size() > 1) {
						tr_cntrs.days_without_storage = std::max(tr_cntrs.days_without_storage, tr_cntrs.storage_days.end()[-1] - tr_cntrs.storage_days.end()[-2]);
					}
					else
						tr_cntrs.days_without_storage = tr_cntrs.storage_days[0] - 7 * discretization;
				}
			}
			amount_stocks -= 1;
			if (save_cntrs) {
				tr_cntrs.sell_days.push_back(end);
				if (tr_cntrs.sell_days.size() > 1) {
					tr_cntrs.hungry_days = std::max(tr_cntrs.hungry_days, tr_cntrs.sell_days.end()[-1] - tr_cntrs.sell_days.end()[-2]);
				}
				else
					tr_cntrs.hungry_days = tr_cntrs.sell_days[0] - 7 * discretization;
			}

			return true;
		}
		else
			return false;
	}


	void update_month() {
		if (save_cntrs) {
			if (tr_cntrs.storage_every_month.size())
				tr_cntrs.storage_per_month.push_back(storage - tr_cntrs.storage_every_month.back());
			else
				tr_cntrs.storage_per_month.push_back(storage);

			tr_cntrs.storage_every_month.push_back(storage);
		}

		++month_done;
		double diff = storage - last_storage;
		mean_store += diff;
		variance_store += diff * diff;
		last_storage = storage;
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
		//std::cout << "ff: " << fitness() << " ";
	}
	void post_print(bool month_storage) const {
		double last_cost = sim.current_cost();
		double size = sim.get_current_point();

		std::cout << "\n\nDataset with amount of days " << size / discretization << " (" << size / discretization / 365 << " years)\n";
		if (save_cntrs) {
			std::cout << "\thangry_days " << tr_cntrs.hungry_days / discretization << "\n";
			std::cout << "\tdays_without_storage " << tr_cntrs.days_without_storage / discretization << "\n";
		}
		std::cout << "\tamount_stocks " << amount_stocks << " [price " << last_cost << "] => " << last_cost * amount_stocks << "\n";
		std::cout << "\tmoney " << money << "\n";
		std::cout << "\tstorage " << storage << "\n";
		std::cout << "\tsum: " << last_cost * amount_stocks + storage + money << "\n";
		if (save_cntrs) {
			if (month_storage) {
				std::cout << "\tstorage_per_month: ";
				for (auto& i : tr_cntrs.storage_per_month)
					std::cout << i << " ";
				}
		}
		std::cout << "\n\n";
		//*(cost_test.end() - 1)
	}
	
	void update_cost(const double _new_cost) {
		sim.step(_new_cost);
	}



	virtual void step(const int action, const bool isGreen) override {
		if (isGreen) {
			last_cost = sim.current_cost();
			switch (action)
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

			if (sim.get_current_point() % (30 * discretization) == 0)
				update_month();
		}
		else {
			auto [mean, var] = sim.calculate_stats_distribution();
			var = sqrt(var);
			double s1 = mean - 1.5 * var;
			double s2 = mean - 3 * var;
			double s3 = mean + 1.5 * var;
			double s4 = mean + 3 * var;
	
			switch (action)
			{
			case 0:
				sim.step(std::max(last_cost + s1, 0.001));
				break;
			case 1:
				sim.step(std::max(last_cost + s2, 0.001));
				break;
			case 2:
				sim.step(std::max(last_cost + s3, 0.001));
				break;
			case 3:
				sim.step(std::max(last_cost + s4, 0.001));
				break;
			default:
				break;
			}
		}
	}
	virtual int actions_number() const override {
		throw "";
		return 0;
	}
	virtual std::pair<double, std::vector<int>> evaluate(const int action, const bool isGreen) const override {
		double eval = 0; 
		double mean = 0;
		double variance = 0;
		if (month_done) {
			mean = mean_store / month_done;
			variance = (variance_store - mean * mean) / month_done;
		}

		eval = mean * 1000 + storage * 10 + amount_stocks * sim.current_cost() + money;
		if (isGreen) {
			return {  eval, {0, 1, 2, 3} };
		}
		else {
			return {  eval, {0, 1, 2} };
		}
	}
	virtual std::shared_ptr<IEnviroment> clone() const override {
		return std::shared_ptr<IEnviroment>(new TradeAgent(*this));
	}

private:
	Simulation sim;

	//statistics 
	double mean_store = 0;
	double variance_store = 0;
	double last_storage = 0;
	int month_done = 0;
	double last_cost = -1;

	TradeCounters tr_cntrs;
	bool save_cntrs;

	double money = 500;
	const double active_money_limit = money;
	double storage = 0;

	int amount_stocks = 0;

	int discretization = 1;
};

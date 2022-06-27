#pragma once

#include <vector>
#include <functional>

class LGenetic {
public:
	int mutation_rate = 50;
	std::string file_validation;

	LGenetic(int popSize, int genSize, std::function<double(std::vector<double>&)> _fitness_function);

	//доступные кроссоверы
	enum Crossover { TPINVX, SPBX, SBX, DBX, OX, HX, AX };

	//доступные мутациири
	enum Mutation { DRM, CM, HM, PM, AM };

	//доступные операторы селекции
	enum Selection { FPS, TS };

	//функци€ обучени€
	void learn(int generationsNum);

	//запускает функцию обучени€ несколько раз
	std::vector<double> learn_a_lot(int generationsNum, int trysNum);

	//возвращает лучшую особь
	std::vector<double> best_gene();

	//заполн€ет гены случайными числами 
	void rand_population();
	void rand_population_normal();

	//заполне€т гены случаными перестановками чисел от 1 до n
	void rand_population_int_order();

	//задание кроссовера
	void set_crossover(Crossover);
	//задание мутации
	void set_mutation(Mutation);
	//задание селекции
	void set_selection(Selection);

	void set_loss(std::function<double(std::vector<double>&, std::string)> _loss);

	void read_gene(std::vector<double> in_gene);

	//задание начальной попул€ции из вне
	void set_population(std::vector<std::vector<double>> in_pop);

	void doPart(int from, int to, std::vector<double>& output);
	
	//
	void make_test(std::string file_name, int itarations, int repeat_times, std::vector<std::vector<std::vector<double>>>& init_pop);
private:
	std::vector<std::vector<double>> pop;
	std::vector<double> b_gene;

	int current_iteration;
	int max_iteration;
	//двух-точечна€ инверис€ 
	void TPINV();
	void SPB();
	void SB();
	void DB();
	void H();
	void O();
	void A();

	void mutationG(std::vector<double>& x);
	void mutationP(std::vector<double>& x);
	void mutationA(std::vector<double>& x);
	void mutationAA(std::vector<double>& x, int genNum);
	void DR(std::vector<double>& x);
	void C(std::vector<double>& x);
	void D(std::vector<double>& x);
	void P(std::vector<double>& x);

	void sort();
	void fitness_proportionate_selection();
	void qsort(std::vector<double>& F, int n);

	std::function<double(std::vector<double>&)> fitness_function;
	std::function<void()> crossover = [this]() { this->SB(); };
	std::function<void()> selection = [this]() { this->sort(); };
	std::function<void(std::vector<double>&)> mutation = [this](std::vector<double>& x) { this->mutationA(x); };


	std::function<double(std::vector<double>&, std::string)> loss;
};
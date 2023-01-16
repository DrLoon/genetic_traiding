#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>
#include <functional>
#include <set>
#include <thread>
#include <string>
#include <limits>
#include <conio.h>

#ifdef PYPLT
#include "matplotlibcpp.h"
#endif

class LGenetic {
public:
	int mutation_rate = 50;

	LGenetic(const int popSize, const int genSize,
		std::function<double(std::vector<double>&)> _fitness_function
	) : fitness_function(_fitness_function)
	{
		if (popSize <= 1 || genSize <= 0) throw;

		pop.resize(popSize);
		for (auto& i : pop)
			i.resize(genSize);
		rand_population_uniform();
		b_gene = pop[0];
	}

	//доступные кроссоверы
	enum Crossover { TPINVX, SPBX, SBX, DBX, OX, HX, AX };

	//доступные мутациири
	enum Mutation { DRM, CM, HM, PM, AM, GM};

	//доступные операторы селекции
	enum Selection { FPS, TS };

	//функция обучения
	void learn(int generationsNum) {
		double start_time = clock();
		max_iteration = generationsNum;
		for (int t = 0; t < generationsNum; ++t)
		{
			#ifdef PYPLT
				avg_fitness.push_back(abs(fitness_function(b_gene)));
				if (is_avarage_fitness) {
					double min_f = std::numeric_limits<double>::max(); 
					double max_f = 0;
					double sum = 0;
					for (int i = 0; i < repeat_times; ++i) {
						double res = abs(single_fitness(b_gene));
						sum += res;
						min_f = std::min(res, min_f);
						max_f = std::max(res, max_f);
					}
					min_fitness.push_back(min_f);
					max_fitness.push_back(max_f);
					avg_fitness.end()[-1] = sum / repeat_times;
				}
				if (_kbhit()) {
					switch (_getch())
					{
					case 's':
						show_plt_avarage();
						break;
					default:
						break;
					}
				}
			#endif
			
			current_iteration = t;
			sort();

			std::cout << t << " - " << fitness_function(b_gene);
			if (enable_loss) std::cout << " \t| loss: " << loss(b_gene);
			std::cout << " \t| " << show_time(start_time);
			std::cout << "\n";
			if (t % 10 == 0) {
				save_best_gene("best_latest_.txt");
			}

			crossover();
			//(rand() % 100 > 50) ? crossover() : this->SB();
			for (int i = 1; i < pop.size(); ++i)
				if (rand() % 100 < mutation_rate) mutation(pop[i]);
		}
	}

	//читаемый вывод прошедшего времени 
	std::string show_time(double start_time) {
		int seconds = static_cast<int>(clock() - start_time) / CLOCKS_PER_SEC;
		int minutes = seconds / 60;
		if (minutes > 0)
			seconds -= minutes * 60;
		else 
			return std::to_string(seconds) + " sec ";
		int hours = minutes / 60;
		if (hours > 0)
			minutes -= hours * 60;
		else
			return std::to_string(minutes) + " min " + std::to_string(seconds) + " sec ";
		return std::to_string(hours) + " hour " + std::to_string(minutes) + " min " + std::to_string(seconds) + " sec ";
	}

	//сохраняет лучшую особь в файл
	void save_best_gene(std::string file_name) {
		std::ofstream f(file_name);
		for (auto& i : this->b_gene)
			f << i << ' ';
	}

	//возвращает лучшую особь
	std::vector<double> best_gene() {
		return b_gene;
	}

	//обрезает значение, если оно выходит за границы
	double clip(double value) {
		value = std::min(value, upper_bound);
		value = std::max(value, lower_bound);
		return value;
	}

	//заполняет гены случайными числами 
	void rand_population() {
		srand(time(0));
		std::default_random_engine generator;
		std::uniform_int_distribution<int> distribution(0, 100);
		for (auto& i : pop)
			for (auto& j : i)
			{
				j = (double)(distribution(generator)) / 100 - 0.5;
				if (enable_bounds) j = clip(j);
			}
	}
	void rand_population_uniform() {
		std::default_random_engine generator;
		std::uniform_real_distribution<double> distribution(-1, 1);
		for (auto& i : pop)
			for (auto& j : i)
			{
				j = distribution(generator);
				if (enable_bounds) j = clip(j);
			}
	}
	void rand_population_normal() {
		srand(time(0));
		std::default_random_engine generator;
		std::normal_distribution<double> distribution(0, 1);
		for (auto& i : pop)
			for (auto& j : i)
			{
				j = distribution(generator);
				if (enable_bounds) j = clip(j);
			}
	}

	//заполнеят гены случаными перестановками чисел от 1 до n
	void rand_population_int_order() {
		int m = pop[0].size();
		std::vector<double> numbers(m);
		for (auto& chrom : pop) {
			for (int i = 0; i < m; ++i) numbers[i] = i;
			unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
			auto rng = std::default_random_engine{ seed };
			shuffle(numbers.begin(), numbers.end(), rng);
			chrom = numbers;
		}
	}

	//задание кроссовера
	void set_crossover(Crossover _crossover) {
		switch (_crossover)
		{
		case TPINVX:
			crossover = [this]() { this->TPINV(); };
			break;
		case SPBX:
			crossover = [this]() { this->SPB(); };
			break;
		case SBX:
			crossover = [this]() { this->SB(); };
			break;
		case DBX:
			crossover = [this]() { this->DB(); };
			break;
		case OX:
			crossover = [this]() { this->O(); };
			break;
		case HX:
			crossover = [this]() { this->H(); };
			break;
		case AX:
			crossover = [this]() { this->A(); };
			break;
		default:
			crossover = [this]() { this->SB(); };
			break;
		}
	}
	//задание селекции
	void set_selection(Selection _selection) {
		switch (_selection)
		{
		case FPS:
			selection = [this]() { this->fitness_proportionate_selection(); };
			break;
		case TS:
			selection = [this]() { this->sort(); };
			break;
		default:
			selection = [this]() { this->sort(); };
			break;
		}
	}
	//задание мутации
	void set_mutation(Mutation _mutation) {
		switch (_mutation)
		{
		case DRM:
			mutation = [this](std::vector<double>& x) { this->DR(x); };
			break;
		case CM:
			mutation = [this](std::vector<double>& x) { this->C(x); };
			break;
		case PM:
			mutation = [this](std::vector<double>& x) { this->P(x); };
			break;
		case AM:
			mutation = [this](std::vector<double>& x) { this->mutationA(x); };
			break;
		case GM:
			mutation = [this](std::vector<double>& x) { this->mutationG(x); };
			break;
		default:
			mutation = [this](std::vector<double>& x) { this->mutationA(x); };
			break;
		}
	}

	void set_loss(std::function<double(std::vector<double>&)> _loss) {
		loss = [_loss](std::vector<double>& x) { return _loss(x); };
		enable_loss = true;
	}

	//запись одного гена из вне
	void read_gene(std::vector<double>& in_gene) {
		if (pop[1].size() != in_gene.size()) throw;
		pop[1] = in_gene;
	}

	//задание начальной популяции из вне
	void set_population(std::vector<std::vector<double>> _pop) {
		if (pop.size() != _pop.size()) throw;
		for(int i = 0; i < pop.size(); ++i)
			if (pop[i].size() != _pop[i].size()) throw;
		this->pop = _pop;
	}

	//функция для обработки потока
	void doPart(int from, int to, std::vector<double>& output) {
		for (int i = from; i < to; ++i) {
			output[i] = fitness_function(pop[i]);
		}
	}

	//активация использования границ и задание их значений
	void set_bounds(double _upper_bound, double _lower_bound) {
		if (_lower_bound >= _upper_bound) throw;
		upper_bound = _upper_bound;
		lower_bound = _lower_bound;
		enable_bounds = true;
	}

	//активация использования потоков и задание их количества
	void enable_multiprocessing(const int _threads_number) {
		if (_threads_number <= 0) throw;
		unsigned int avalible_threads = std::thread::hardware_concurrency();
		if (avalible_threads < _threads_number) {
			std::cout << "Attempt to use " << _threads_number << " threads, when only " << avalible_threads << " are avalible\n";
		}
		enable_threads = true;
		threads_number = _threads_number;
	}

	void enable_avarage_fitness(const int _repeat_times) {
		if (_repeat_times <= 0) throw;
		if (!is_avarage_fitness)
			single_fitness = fitness_function;
			//single_fitness = [this](std::vector<double>& x) { return fitness_function(x); };
		is_avarage_fitness = true;
		repeat_times = _repeat_times;
		fitness_function = [this](std::vector<double>& x) { return avarage_fitness(x); };
	} 

#ifdef PYPLT
	//show learning results
	void show_plt_avarage() {
		std::vector<double> x;
		for (int i = 0; i < avg_fitness.size(); ++i) x.push_back(static_cast<double>(i));
		// Clear previous plot
		matplotlibcpp::clf();

		if (is_avarage_fitness) {
			std::cout << "plotting between\n";
			/*std::map<std::string, std::string> keywords;
			keywords["alpha"] = "0.4";
			keywords["color"] = "grey";
			keywords["hatch"] = "-";
			matplotlibcpp::fill_between(x, max_fitness, min_fitness, keywords);*/
			matplotlibcpp::scatter(x, max_fitness, { {"label", "max fitness"}, {"color", "green"} });
			matplotlibcpp::scatter(x, min_fitness, { {"label", "min fitness"}, {"color", "orange"} });
		}
		// Plot a line whose name will show up as "log(x)" in the legend.
		matplotlibcpp::plot(x, avg_fitness, { {"label", "average fitness"}, {"color", "blue"}});
		// Add graph title
		matplotlibcpp::title("Fitness");
		// Enable legend.
		matplotlibcpp::legend();
		matplotlibcpp::show();
		std::cout << "average fitness plot was shown\n";
	}
#endif;

private:
	std::vector<std::vector<double>> pop;
	std::vector<double> b_gene;

	//work with bounds 
	bool enable_bounds = false;
	double upper_bound;
	double lower_bound;

	//work with threads 
	bool enable_threads = false;
	int threads_number = 1;


	int current_iteration;
	int max_iteration;

	//two-point inverse crossover 
	void TPINV() {
		int n = pop.size() / 2;
		for (int i = 0; i < n; i++)
		{
			pop[n + i] = pop[i];
			int m = pop[i].size();
			int r = rand() % m;
			int l = rand() % m;
			while (r == l) l = rand() % m;
			if (l < r) std::swap(l, r);
			for (int j = 0; j < (l - r) / 2; j++)
				std::swap(pop[i + n][r + j], pop[i + n][l - j]);
		}
	}
	//single point crossover (b - for binary)
	void SPB() {
		int n = pop.size() / 2;
		for (int i = 0; i < n; i += 2) {
			pop[n + i] = pop[i];
			pop[n + i + 1] = pop[i + 1];
			int m = rand() % pop[i].size();

			for (int j = 0; j < m; j++)
				std::swap(pop[n + i][j], pop[n + i + 1][j]);
		}
	}
	//simulated binary crossover
	void SB() {
		int n = pop.size() / 2;
		double eta = 100;
		for (int i = 0; i < n - 1; i += 2) {
			pop[n + i] = pop[i];
			pop[n + i + 1] = pop[i + 1];
			double u = static_cast<double>(rand() % 100) / 100;
			double gamma;
			if (u <= 0.5) {
				gamma = pow(2 * u, 1 / (1 + eta));
			}
			else {
				gamma = pow(1 / (2 * (1 - u)), 1 / (1 + eta));
			}
			for (int j = 0; j < pop[i].size(); j++) {
				pop[n + i][j] = 0.5 * ((1 + gamma) * pop[i][j] + (1 - gamma) * pop[i + 1][j]);
				pop[n + i + 1][j] = 0.5 * ((1 - gamma) * pop[i][j] + (1 + gamma) * pop[i + 1][j]);

				if (enable_bounds) pop[n + i][j] = clip(pop[n + i][j]);
				if (enable_bounds) pop[n + i + 1][j] = clip(pop[n + i + 1][j]);
			}
		}
	}
	//diraction based crossover
	void DB() {
		int n = pop.size() / 2;
		int m = n / 2;
		int divser = abs(fitness_function(pop[0]) - fitness_function(pop[n]));
		if (divser <= 1e-6) divser = 1;
		for (int i = 0; i < m; ++i) {
			pop[n + i] = pop[i];
			pop[n + m + i] = pop[m + i];
			double s_i = abs(fitness_function(pop[i]) - fitness_function(pop[m + i])) / divser;
			bool change = false;
			int j_mem = 0;
			for (int j = 0; j < pop[i].size(); j++) {
				if (abs(pop[i][j] - pop[m + i][j]) != 0) j_mem = j;
				double r_j = rand() % 100 > 50 ? abs(pop[i][j] - pop[m + i][j]) : 0.0;
				if (r_j != 0) change = true;
				pop[n + i][j] = pop[i][j] + s_i * r_j;
				pop[n + m + i][j] = pop[m + i][j] + s_i * r_j;

				if (enable_bounds) pop[n + i][j] = clip(pop[n + i][j]);
				if (enable_bounds) pop[n + m + i][j] = clip(pop[n + m + i][j]);

			}
			if (!change) {
				//throw;
				if (abs(pop[i][j_mem] - pop[m + i][j_mem]) != 0) {
					pop[n + i][j_mem] += abs(pop[i][j_mem] - pop[m + i][j_mem]);
					pop[n + m + i][j_mem] += abs(pop[i][j_mem] - pop[m + i][j_mem]);
				}
				else {
					j_mem = rand() % pop[i].size();
					pop[n + i][j_mem] += (double)(rand() % 100) / 100.0;
					pop[n + m + i][j_mem] += (double)(rand() % 100) / 100.0;
				}
			}
		}
	}
	//heuristic crossover
	void H() {
		int n = pop.size() / 2;
		srand(time(0));
		std::default_random_engine generator;
		std::uniform_real_distribution<double> distribution(-1, 1);
		for (int i = 0; i < n; i += 2) {
			double u = distribution(generator);
			for (int j = 0; j < pop[i].size(); j++) {
				pop[n + i][j] = u * (pop[i][j] - pop[i + 1][j]) + pop[i][j];
			}
		}
	}
	//ordered crossover 
	void O() {
		int n = pop.size() / 2;
		for (int i = 0; i < n; i += 2) {
			pop[n + i] = pop[i];
			pop[n + i + 1] = pop[i + 1];
			int r_1 = rand() % pop[i].size();
			int r_2 = rand() % pop[i].size();
			if (r_1 > r_2) std::swap(r_1, r_2);
			std::set<double> perent1, perent2;
			for (int j = r_1; j < r_2; j++) {
				std::swap(pop[n + i][j], pop[n + i + 1][j]);
				perent1.insert(pop[n + i + 1][j]);
				perent2.insert(pop[n + i][j]);
			}

			int iter = 0;
			for (int j = 0; j < pop[i].size(); ++j) {
				if (perent1.contains(pop[i][j])) {
					pop[n + i][iter] = pop[i][j];
					++iter;
					if (iter >= r_1 && iter < r_2) iter = r_2;
					if (iter >= pop[i].size()) break;
				}
			}

			iter = 0;
			for (int j = 0; j < pop[i].size(); ++j) {
				bool exist = false;
				for (int k = r_1; k < r_2; k++)
					if (pop[i + 1][j] == pop[n + i + 1][k]) exist = true;
				if (perent2.contains(pop[i + 1][j])) {
					pop[n + i + 1][iter] = pop[i + 1][j];
					++iter;
					if (iter >= r_1 && iter < r_2) iter = r_2;
					if (iter >= pop[i].size()) break;
				}
			}

		}
	}
	//alex crossover
	void A() {
		int n = pop.size() / 2;
		for (int i = 0; i < n; i += 2) {
			pop[n + i] = pop[i];
			pop[n + i + 1] = pop[i + 1];

			int m = rand() % 5 + 1;
			for (int j = 0; j < m; ++j) {
				int a = rand() % pop[i].size();
				int b = rand() % pop[i].size();
				while (b == a) b = rand() % pop[i].size();
				if (b < a) std::swap(a, b);

				for (int k = a; k < b + 1; ++k) {
					std::swap(pop[n + i][k], pop[n + i + 1][k]);
				}
			}
		}
	}

	//меняет два гена местами 
	void mutationG(std::vector<double>& x) {
		int m = x.size();
		int k = rand() % (m - 1);
		std::swap(x[k], x[k + 1]);
	}
	void mutationP(std::vector<double>& x) {
		int num = 5;
		int mut = (double)(rand() % 20) / 100;
		for (int i = 0; i < num; i++) {
			int j = rand() % pop[i].size();
			x[j] += mut;
			if (enable_bounds) x[j] = clip(x[j]);
		}
	}
	void mutationA(std::vector<double>& x) {
		std::random_device rd;
		std::mt19937 e2(rd());
		std::normal_distribution<> dist(0, 1);
		//double mut = dist(e2);

		int num = rand() % (pop[0].size() / 5);
		double mut = (double)(rand() % 1500) / 1000;
		if (rand() % 2)
			mut *= -1;
		for (int i = 0; i < num; i++) {
			int j = rand() % pop[0].size();
			x[j] += dist(e2) * 0.2;
			if (enable_bounds) x[j] = clip(x[j]);
		}
	}
	
	//dynumic random mutation
	void DR(std::vector<double>& x) {
		double b = 0.05;
		std::default_random_engine generator;
		std::uniform_real_distribution<double> distribution(-1, 1);
		double s = pow(1 - (double)current_iteration / (double)max_iteration, b);
		for (auto& i : x) {
			i += s * distribution(generator);
			if (enable_bounds) i = clip(i);
		}
	}
	//применяет оператор кроссовера к мутирующему гену и случайно сгенерированной особи 
	void C(std::vector<double>& x) {
		std::vector<double> y(x.size());
		std::default_random_engine generator;
		std::uniform_real_distribution<double> distribution(-1, 1);
		for (auto& j : y)
			j = distribution(generator);
		std::vector<std::vector<double>> im_pop(4);
		im_pop[0] = x;
		im_pop[1] = y;
		im_pop[2] = x;
		im_pop[3] = y;
		std::swap(pop, im_pop);
		crossover();
		std::swap(pop, im_pop);
		x = im_pop[2];
	}
	void P(std::vector<double>& x) {
		std::default_random_engine generator;
		std::uniform_real_distribution<double> distribution(-1, 1);
		std::exponential_distribution<double> exponent(1);
		double s = exponent(generator);
		for (auto& i : x) {
			double t = (i - 1) / 2;
			double r = distribution(generator);
			if (t < r) i -= s * (i + 1);
			else i += s * (1 - i);
			if (enable_bounds) i = clip(i);
		}
	}

	void sort() {
		std::vector<double> F;
		F.reserve(pop.size());
		if (enable_threads) {
			F.resize(pop.size());
			int n = pop.size();
			std::vector<std::thread> threads(threads_number);
			for (int i = 0; i < threads_number; ++i)
				threads[i] = std::thread(&LGenetic::doPart, this, i * n / threads_number, (i + 1) * n / threads_number, std::ref(F));

			for (auto& th : threads)
				th.join();
		}
		else {
			for (auto& i : pop) { F.push_back(fitness_function(i)); };
		}

		qsort(F, pop.size());
		b_gene = pop[0]; //TODO добавть проверку, что результат теперь лучше 
	}
	void qsort(std::vector<double>& F, int n) {
		int i = 0;
		int j = n - 1;
		double mid = F[n / 2];

		do {
			while (F[i] < mid) i++;
			while (F[j] > mid) j--;

			if (i <= j) {
				std::swap(pop[i], pop[j]);
				std::swap(F[i], F[j]);
				i++;
				j--;
			}
		} while (i <= j);

		if (j > 0) qsort(F, j + 1);
		if (i < n) qsort(F, n - i);
	}
	void fitness_proportionate_selection() {
		std::vector<double> F;
		for (auto& i : pop) F.push_back(fitness_function(i));
		qsort(F, pop.size());
		b_gene = pop[0];
		int p = pop.size() / 2;
		double sum = 0.0;
		for (int i = 0; i < p; ++i) sum += F[i];
		for (int i = 0; i < p; ++i) F[i] /= sum;
		auto pop_copy = pop;
		for (int i = 1; i < p; ++i) {
			double prob = (double)(rand() % 100) / 100.0;
			int number = 0;
			double prob_sum = 0;
			for (int j = 0; j < F.size(); ++j) {
				prob_sum += F[i];
				if (prob_sum >= prob) {
					number = j;
					break;
				}
			}
			pop[i] = pop_copy[number];
		}
	}

	std::function<double(std::vector<double>&)> fitness_function;
	std::function<void()> crossover = [this]() { this->SB(); };
	std::function<void()> selection = [this]() { this->sort(); };
	std::function<void(std::vector<double>&)> mutation = [this](std::vector<double>& x) { this->mutationA(x); };


	std::function<double(std::vector<double>&)> loss;
	bool enable_loss = false;

	#ifdef PYPLT
	//plotting staff
	std::vector<double> avg_fitness;
	std::vector<double> min_fitness;
	std::vector<double> max_fitness;
	#endif

	bool is_avarage_fitness = false;
	std::function<double(std::vector<double>&)> single_fitness;
	int repeat_times = 5;
	double avarage_fitness(std::vector<double>& x) {
		double res = 0;
		for (int i = 0; i < repeat_times; ++i) {
			res += single_fitness(x);
		}
		return res / repeat_times;
	}
};

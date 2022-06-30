#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>
#include <functional>
#include <set>
#include <thread>

#include"LGenetic.h"


LGenetic::LGenetic(int popSize, int genSize, 
							std::function<double(std::vector<double>&)> _fitness_function
						   ) : fitness_function(_fitness_function) 
{
	pop.resize(popSize);
	for (auto& i : pop)
		i.resize(genSize);

	int m = pop[0].size();
	std::vector<double> numbers(m);
	for (auto& chrom : pop) {
		for (int i = 0; i < m; i++) numbers[i] = i;

		unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
		auto rng = std::default_random_engine{ seed };
		shuffle(numbers.begin(), numbers.end(), rng);
		chrom = numbers;
	}
	b_gene = pop[0];
}

void LGenetic::learn(int generationsNum) {
	double minn = fitness_function(pop[0]);

	max_iteration = generationsNum;
	for (int t = 0; t < generationsNum; ++t)
	{
		current_iteration = t;
		sort();
		std::cout << t << " - " << abs(fitness_function(b_gene)) << " | " << loss(b_gene, file_validation) << "\n";

		crossover();
		//(rand() % 100 > 50) ? crossover() : this->TPINV();
		for (int i=1;i<pop.size();++i)
			if (rand() % 100 < mutation_rate) mutation(pop[i]);
	}
}

//запускает функцию обучения несколько раз
std::vector<double> LGenetic::learn_a_lot(int generationsNum, int trysNum) {
	rand_population();
	std::vector<double> best_gene = pop[0];
	for (int i = 0; i < trysNum; ++i) {
		rand_population();
		learn(generationsNum);
		if (fitness_function(best_gene) > fitness_function(pop[0]))
			best_gene = pop[0];
		std::cout << '\n';
	}
	
	return best_gene;
}

//возвращает лучшую особь
std::vector<double> LGenetic::best_gene() {
	return pop[0];
}

//задание кроссовера
void LGenetic::set_crossover(Crossover _crossover) {
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
void LGenetic::set_selection(Selection _selection) {
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
void LGenetic::set_mutation(Mutation _mutation) {
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
	default:
		mutation = [this](std::vector<double>& x) { this->mutationA(x); };
		break;
	}
}

void LGenetic::set_loss(std::function<double(std::vector<double>&, std::string)> _loss) {
	loss = [_loss](std::vector<double>& x, std::string file) { return _loss(x, file); };
}

//заполнеят гены случаными перестановками чисел от 1 до n
void LGenetic::rand_population_int_order() {
	int m = pop[0].size();
	std::vector<double> numbers(m);
	for (auto& chrom : pop) {
		for (int i = 0; i < m; i++) numbers[i] = i;
		unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
		auto rng = std::default_random_engine{ seed };
		shuffle(numbers.begin(), numbers.end(), rng);
		chrom = numbers;
	}
}
//заполняет гены случайными числами 
void LGenetic::rand_population() {
	srand(time(0));
	std::default_random_engine generator;
	std::uniform_int_distribution<int> distribution(0, 100);
	for(auto& i : pop)
		for (auto& j : i)
			j = (double)(distribution(generator)) / 100 - 0.5;
}
void LGenetic::rand_population_normal() {
	srand(time(0));
	std::default_random_engine generator;
	std::uniform_real_distribution<double> distribution(-1 , 1 );
	for (auto& i : pop)
		for (auto& j : i)
			j = distribution(generator);
}

void LGenetic::read_gene(std::vector<double> in_gene) {
	pop[1] = in_gene;
}

//двух-точечная инверися
void LGenetic::TPINV() {
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
void LGenetic::SPB(){
	int n = pop.size() / 2;
	for (int i = 0; i < n; i += 2) {
		pop[n + i] = pop[i];
		pop[n + i + 1] = pop[i + 1];
		int m = rand() % pop[i].size();

		for (int j = 0; j < m; j++)
			std::swap(pop[n + i][j], pop[n + i + 1][j]);
	}
}
void LGenetic::SB() {
	int n = pop.size() / 2;
	double eta = 1;
	for (int i = 0; i < n - 1; i += 2) {
		pop[n + i] = pop[i];
		pop[n + i + 1] = pop[i + 1];
		double u = (double)(rand() % 100) / 100;
		double gamma;
		if (u <= 0.5) {
			gamma = pow(2 * u, 1 / (1 + eta));
		}
		else {
			gamma = pow(1 / (2 * (1 - u)), 1 / (1 + eta));
		}
		for (int j = 0; j < pop[i].size(); j++){
				pop[n + i][j] = 0.5 * ((1 + gamma) * pop[i][j] + (1 - gamma) * pop[i + 1][j]);
				pop[n + i + 1][j] = 0.5 * ((1 - gamma) * pop[i][j] + (1 + gamma) * pop[i + 1][j]);
				if (pop[n + i][j] > 10) pop[n + i][j] = 10;
				if (pop[n + 1 + i][j] > 10) pop[n + 1 + i][j] = 10;
				if (pop[n + i][j] < -10) pop[n + i][j] = -10;
				if (pop[n + 1 + i][j] < -10) pop[n + 1 + i][j] = -10;
		}
	}
}
void LGenetic::DB() {
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
			double r_j =  rand() % 100 > 50 ? abs(pop[i][j] - pop[m + i][j]) : 0.0;
			if (r_j != 0) change = true;
			pop[n + i][j] = pop[i][j] + s_i * r_j;
			pop[n + m + i][j] = pop[m + i][j] + s_i * r_j;

			if (pop[n + i][j] > 10) pop[n + i][j] = 10;
			if (pop[n + m + i][j] > 10) pop[n + m + i][j] = 10;
			if (pop[n + i][j] < -10) pop[n + i][j] = -10;
			if (pop[n + m + i][j] < -10) pop[n + m + i][j] = -10;
			//if (pop[n + m + i][j] != pop[m + i][j]) std::cout << "lol";

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
void LGenetic::O() {
	int n = pop.size() / 2;
	for (int i = 0; i < n; i += 2) {
		pop[n + i] = pop[i];
		pop[n + i + 1] = pop[i + 1];
		int r_1 = rand() % pop[i].size();
		int r_2 = rand() % pop[i].size();
		if(r_1 > r_2) std::swap(r_1, r_2);
		std::set<double> perent1, perent2;
		for (int j = r_1; j < r_2; j++) {
			std::swap(pop[n + i][j], pop[n + i + 1][j]);
			perent1.insert(pop[n + i + 1][j]);
			perent2.insert(pop[n + i][j]);
		}

		int iter = 0;
		for (int j = 0; j < pop[i].size(); ++j) {
			/*bool exist = false;
			for (int k = r_1; k < r_2; k++)
				if (pop[i][j] == pop[n + i][k]) exist = true;*/
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
void LGenetic::H() {
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
void LGenetic::A() {
	/*int n = pop.size();
	int prts_num = 6;
	std::pair<int, int> prnts = { 0, 1 };
	for (int i = prts_num; i < n - 1; ++i) {
		if (prnts.first == prts_num - 1)
			throw;

		pop[i] = pop[prnts.first];
		pop[i + 1] = pop[prnts.second];

		int m = rand() % pop[i].size();
		for (int j = 0; j < m; ++j) {
			std::swap(pop[i][j], pop[i + 1][j]);
		}

		prnts.second++;
		if (prnts.second == prts_num) {
			prnts.first++;
			prnts.second = prnts.first + 1;
		}
	}*/
	/*int n = pop.size() / 2;
	for (int i = 0; i < n; i += 2) {
		pop[n + i] = pop[i];
		pop[n + i + 1] = pop[i + 1];
		int m = rand() % pop[i].size();

		for (int j = 0; j < m; ++j) {
			int a = rand() % pop[i].size();
			std::swap(pop[n + i][a], pop[n + i + 1][a]);
		}
	}*/
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
	/*int n = pop.size() / 2;
	for (int i = 0; i < n; i += 2) {
		pop[n + i] = pop[i];
		pop[n + i + 1] = pop[i + 1];
		int a = rand() % pop[i].size();
		int b = rand() % pop[i].size();
		if(a > b) std::swap(a, b);
		for (int j = a; j < b; ++j) {
			std::swap(pop[n + i][j], pop[n + i + 1][j]);
		}
	}*/
}


void LGenetic::mutationG(std::vector<double>& x) {
	int m = x.size();
	int k = rand() % (m - 1);
	std::swap(x[k], x[k + 1]);
}
void LGenetic::mutationP(std::vector<double>& x) {
	int num = 5;
	int mut = (double)(rand() % 20) / 100;
	for (int i = 0; i < num; i++) {
		int j = rand() % pop[i].size();
		x[j] += mut;
	}
}
void LGenetic::mutationA(std::vector<double>& x) {
	int num = rand() % 200;
	double mut = (double)(rand() % 5000) / 1000;
	if (rand() % 2)
		mut *= -1;
	for (int i = 0; i < num; i++) {
		int j = rand() % pop[0].size();
		x[j] += mut;
	}
}
void LGenetic::mutationAA(std::vector<double>& x, int genNum) {
	int num = rand() % pop[0].size();
	double mut = (double)(rand() % 30000) / 30000.0;
	if (rand() % 2)
		mut *= -1;
	for (int i = 0; i < num; i++) {
		int j = rand() % pop[0].size();
		x[j] += mut;
	}
}
void LGenetic::DR(std::vector<double>& x) {
	double b = 0.05;
	std::default_random_engine generator;
	std::uniform_real_distribution<double> distribution(-1, 1);
	double s = pow(1 - (double)current_iteration/(double)max_iteration, b);
	for (auto& i : x) {
		i += s * distribution(generator);
		if (i > 1) i = 1;
		if (i < -1) i = -1;
	}
}
void LGenetic::C(std::vector<double>& x) {
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
}
void LGenetic::D(std::vector<double>& x) {

}
void LGenetic::P(std::vector<double>& x) {
	std::default_random_engine generator;
	std::uniform_real_distribution<double> distribution(-1, 1);
	std::exponential_distribution<double> exponent(1);
	double s = exponent(generator);
	for (auto& i : x) {
		double t = (i - 1) / 2;
		double r = distribution(generator);
		if (t < r) i -= s * (i + 1);
		else i += s * (1 - i);
	}
}

//void LGenetic::sort() {
//	std::vector<double> F;
//	for (auto& i : pop) F.push_back(fitness_function(i));
//	qsort(F, pop.size());
//	b_gene = pop[0];
//}
void LGenetic::qsort(std::vector<double>& F, int n) {
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
void LGenetic::sort() {
	std::vector<double> F;
	F.reserve(pop.size());
	//for (auto& i : pop) { F.push_back(fitness_function(i)); std::cout << "1\n"; };
	F.resize(pop.size());
	int n = pop.size();
	int threadsN = 8;
	std::thread th1(&LGenetic::doPart, this, 0 * n / threadsN, 1 * n / threadsN, std::ref(F));
	std::thread th2(&LGenetic::doPart, this, 1 * n / threadsN, 2 * n / threadsN, std::ref(F));
	std::thread th3(&LGenetic::doPart, this, 2 * n / threadsN, 3 * n / threadsN, std::ref(F));
	std::thread th4(&LGenetic::doPart, this, 3 * n / threadsN, 4 * n / threadsN, std::ref(F));
	std::thread th5(&LGenetic::doPart, this, 4 * n / threadsN, 5 * n / threadsN, std::ref(F));
	std::thread th6(&LGenetic::doPart, this, 5 * n / threadsN, 6 * n / threadsN, std::ref(F));
	std::thread th7(&LGenetic::doPart, this, 6 * n / threadsN, 7 * n / threadsN, std::ref(F));
	std::thread th8(&LGenetic::doPart, this, 7 * n / threadsN, 8 * n / threadsN, std::ref(F));
	//std::thread th9(&LGenetic::doPart, this, 8 * n / threadsN, 9 * n / threadsN, std::ref(F));
	//std::thread th10(&LGenetic::doPart, this, 9 * n / threadsN, 10 * n / threadsN, std::ref(F));
	//std::thread th11(&LGenetic::doPart, this, 10 * n / threadsN, 11 * n / threadsN, std::ref(F));
	//std::thread th12(&LGenetic::doPart, this, 11 * n / threadsN, 12 * n / threadsN, std::ref(F));
	

	th1.join();
	th2.join();
	th3.join();
	th4.join();
	th5.join();
	th6.join();
	th7.join();
	th8.join();
	//th9.join();
	//th10.join();
	//th11.join();
	//th12.join();

	qsort(F, pop.size());
	b_gene = pop[0];
}

void LGenetic::fitness_proportionate_selection() {
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

void LGenetic::set_population(std::vector<std::vector<double>> in_pop) {
	this->pop = in_pop;
}


void LGenetic::make_test(std::string file_name, int itarations, int repeat_times, std::vector<std::vector<std::vector<double>>>& init_pop) {
	if (init_pop.size() != repeat_times) throw;
	std::vector<double> mean_fitness(itarations);
	std::vector<double> best_fitness(itarations);
	std::vector<double> worst_fitness(itarations);
	std::vector<double> temp(itarations);

	double best_score = 100000000;
	double worst_score = 0;
	clock_t start = clock();
	for (int rep = 0; rep < repeat_times; ++rep) {
		std::cout << rep << "\n";
		double res = -1;
		//rand_population_normal();
		pop = init_pop[rep];
		max_iteration = itarations;
		for (int t = 0; t < itarations; ++t)
		{
			current_iteration = t;
			//(rand() % 100 > 50) ? crossover() : this->TPINV();

			selection();
			crossover();
			//(rand() % 100 > 50) ? crossover() : this->TPINV();
			//(rand() % 100 > 50) ? this->SPB() : this->TPINV();
			for (int i = 1; i < pop.size(); ++i)
				if (rand() % 100 < mutation_rate) mutation(pop[i]);
			for (int i = 1; i < pop.size(); ++i)
				if (rand() % 100 < 5) mutationA(pop[i]);

			//res = fitness_function(b_gene);
			res = loss(b_gene, file_validation);
			if (!(t % 100))
				std::cout << t << " \t " << res << "\n";
			if (res != res) 
				std::cout << "lol";
			mean_fitness[t] += res;
			temp[t] = res;
		}
		if (best_score > res) {
			best_score = res;
			best_fitness = temp;
		}
		if (worst_score < res) {
			worst_score = res;
			worst_fitness = temp;
		}
	}
	clock_t now = clock();
	std::cout << (double)(now - start) / CLOCKS_PER_SEC << " sec\n";
	std::cout << "\n";
	for (auto& i : mean_fitness) i /= repeat_times;
	std::ofstream mean_file("test_mean_" + file_name);
	std::ofstream best_file("test_best_" + file_name);
	std::ofstream worst_file("test_worst_" + file_name);

	mean_file << itarations << "\n";
	for (int t = 0; t < itarations; ++t)
		mean_file << t << " " << mean_fitness[t] << "\n";
	mean_file << (double)(now - start) / CLOCKS_PER_SEC << "\n";

	best_file << itarations << "\n";
	for (int t = 0; t < itarations; ++t)
		best_file << t << " " << best_fitness[t] << "\n";
	best_file << (double)(now - start) / CLOCKS_PER_SEC << "\n";

	worst_file << itarations << "\n";
	for (int t = 0; t < itarations; ++t)
		worst_file << t << " " << worst_fitness[t] << "\n";
	worst_file << (double)(now - start) / CLOCKS_PER_SEC << "\n";
}

void LGenetic::doPart(int from, int to, std::vector<double>& output) {
	for (int i = from; i < to; ++i) {
		output[i] = fitness_function(pop[i]);
	}
}

#pragma once
#include <iostream>
#include <vector>
#include <unordered_map>




class IEnviroment
{
public:
	virtual void step(const int action, const bool isGreen) = 0;  // совершает шаг по номеру действия из actions_number
	virtual int actions_number() const = 0;  // возращает количество возможных действий в текущей ситуации
	virtual std::pair<double, std::vector<int>> evaluate(const int action, const bool isGreen) const = 0;  // возращает оценку текущей ситуации и ветвление
	virtual std::shared_ptr<IEnviroment> clone() const = 0;  // 
};


class Node {
public:
	const int id = -1;
	const int parent = -1;
	std::vector<int> succ;
	std::vector<int> actions_succ;

	double Q = 0;
	int visits = 0;
	double acc_Q = 0;


	std::shared_ptr<IEnviroment> env;
	const int action = -1;


	Node() {
		throw "you can't make an empty Node (may problem with map)";
	}
	Node(const int _id, const int _parent, const int _action, const IEnviroment& _env) : 
		id(_id), parent(_parent), action(_action), env(_env.clone())
	{
		
	}

	void reset() {}

	void update_metrics(const double Q_children_sum, const int new_visits) {
		acc_Q += Q_children_sum;
		visits += new_visits;
	}

	double UCB() const {
		return acc_Q / visits;
	}
	double UCB_pro(const int neibours_visits) const {
		return acc_Q / visits + C * (log(neibours_visits) / visits);
	}



private:
	const double C = 0;
};


class MCTree {
public:
	MCTree() {}

	bool addNode(const Node& node) {
		auto res = nodes.insert(std::make_pair(node.id, node)).second;
		if (res && node.parent != -1)
			nodes[node.parent].succ.push_back(node.id);
		return res;
	}

	Node& operator[](const int i) {
		return nodes.at(i);
	}

	int nodes_size() const {
		return nodes.size();
	}

private:
	std::unordered_map<int, Node> nodes;
};





class MCTS {
public:
	const int max_deep;
	MCTS(const int _max_deep, IEnviroment& _base_env, const int C = 0) : max_deep(_max_deep), base_env(_base_env) {
		TREE.addNode(Node(0, -1, -1, base_env));  // id 0 and -1 is parent
		//auto val = TREE[0].env->evaluate(TREE[0].action);
		TREE[0].actions_succ = { 0, 1, 2 };
		//TREE[0].Q = 1;
		isGreen = true;
	}

	std::vector<std::tuple<int, double, int>> run() {
		// возвращает оценку всех возможных действий на 1м слое
		for (int i = 0; i < max_deep; ++i) {
			step();
		}

		std::vector<std::tuple<int, double, int>> res;
		for (auto& i : TREE[0].succ) {
			res.push_back(std::make_tuple(TREE[i].action, TREE[i].UCB_pro(this->neibours_visits(TREE[i])), TREE[i].visits));
		}

		return res;
	}

	void print_binary_tree() {
		print_tree_level(&TREE[0], 0);
	}

private:
	MCTree TREE;
	const IEnviroment& base_env;
	bool isGreen = true;

	int gothrough() {
		Node* current_node = &TREE[0];
		isGreen = true;

		while (current_node->succ.size()) {

			std::vector<int>& children = current_node->succ;
			double max_UCB = -99999999;
			int max_ind = -1;
			for (int i = 0; i < (int)children.size(); ++i) {
				double UCB = TREE[children[i]].UCB_pro(this->neibours_visits(TREE[children[i]]));
				if (max_UCB < UCB) {
					max_UCB = UCB;
					max_ind = TREE[children[i]].id;
				}
			}

			current_node = &TREE[max_ind];
			isGreen = !isGreen;
		}
		return current_node->id;
	}

	void expand(const int node_id) {
		Node& cur_node = TREE[node_id];
		//auto val = cur_node.env->evaluate(TREE[0].action, isGreen);
		
		for (int i = 0; i < cur_node.actions_succ.size(); ++i) {
			int new_id = nodes_count(); // got new id (size is 1 more than max id)
			TREE.addNode(Node(new_id, node_id, cur_node.actions_succ[i], *cur_node.env)); // add Node and gave it parent action
			TREE[new_id].env->step(cur_node.actions_succ[i], isGreen);
			std::tie(TREE[new_id].Q, TREE[new_id].actions_succ) = TREE[new_id].env->evaluate(cur_node.actions_succ[i], isGreen);

			//auto val = TREE[new_id].env->evaluate(TREE[0].action);
			//TREE[new_id].actions_succ = { 0,1,2,3 };
			//TREE[new_id].Q = val.second[i];

			TREE[new_id].update_metrics(TREE[new_id].Q, 1);
		}
	}

	void simulate() {
		// not using
	}

	void backprop(const int node_id) {
		double Q_children_sum = 0;
		for (auto& i : TREE[node_id].succ) {
			Q_children_sum += TREE[i].Q;
		}
		TREE[node_id].update_metrics(Q_children_sum, TREE[node_id].succ.size());

		Node* current_node = &TREE[node_id];
		Node* parent_node = nullptr;
		while (current_node->parent != -1) {
			parent_node = &TREE[current_node->parent];

			parent_node->update_metrics(Q_children_sum, TREE[node_id].succ.size());

			current_node = parent_node;
		}
	}

	void step() {
		int node = gothrough();
		expand(node);
		//simulate();
		backprop(node);
	}

	int nodes_count() {
		return TREE.nodes_size();
	}

	void print_tree_level(Node* top, int level) {
		if (top) {
			if (top->succ.size())
				print_tree_level(&TREE[top->succ[0]], level + 1);
			for (int i = 0; i < level; i++)
				std::cout << "     ";
			std::cout << top->id << std::endl;
			if (top->succ.size())
				print_tree_level(&TREE[top->succ[1]], level + 1);
		}
	}

	int neibours_visits(const Node& node) {
		const Node& parent = TREE[node.parent];
		int visits = 0;
		for (auto& i : parent.succ)
			visits += TREE[i].visits;
		return visits;
	}
};



//namespace testMCTS {
//	class Env : public IEnviroment {
//	public:
//		Env() {
//
//		}
//		void step(const int action) override {
//		}
//		int actions_number() const override {
//			return 2;
//		}
//		double evaluate(const int action) const override {
//			return (double)rand() / RAND_MAX;
//			//return value;
//		}
//
//		std::shared_ptr<IEnviroment> clone() const override {
//			return std::shared_ptr<IEnviroment>(new Env(*this));
//		}
//
//		double reward() {
//			return 0;
//		}
//	};
//
//	class Env2 : public IEnviroment {
//	public:
//		double value = 0;
//		std::vector<double> values_vec;
//		Env2() {
//
//		}
//		void step(const int action) override {
//			values_vec.push_back(value);
//			if (action == 0)
//				value += 0.01;
//			else
//				value += (double)rand() / RAND_MAX / 5;
//		}
//		int actions_number() const override {
//			return 2;
//		}
//		double evaluate(const int action) const override {
//			//return (double)rand() / RAND_MAX;
//			if (values_vec.size() < 100)
//				return value;
//			return value - values_vec.end()[-100];
//		}
//
//		double reward() {
//			return value;
//		}
//
//		std::shared_ptr<IEnviroment> clone() const override {
//			return std::shared_ptr<IEnviroment>(new Env2(*this));
//		}
//	};
//};

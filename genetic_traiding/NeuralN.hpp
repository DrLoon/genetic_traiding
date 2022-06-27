#pragma once
#include <vector>
#include <initializer_list>

class NeuralN;
#include <armadillo>

using namespace arma;


void relu(mat& input) {
	for (auto& i : input) {
		i = std::max(0.0, i);
	}
}
void sigmoid(mat& input) {
	for (auto& i : input) {
		i = 1 / (exp(-i) + 1);
	}
}



class NeuralN {
public:
	enum activation_type { SIGMOID, RELU };

	NeuralN(std::initializer_list<int> _layers_size, std::initializer_list<activation_type> _layers_activation)
		: layers_size(_layers_size), 
		  layers_activation(_layers_activation)
	{
		if (layers_size.size() - 1 != layers_activation.size()) throw "bad input";

		layers.resize(layers_size.size() - 1);
		for (size_t i = 0; i < layers.size(); ++i) {
			layers[i] = randu<mat>(layers_size[i], layers_size[i + 1]);
		}

		biases.resize(layers_size.size() - 1);
		for (size_t i = 0; i < layers.size(); ++i) {
			biases[i] = randu<mat>(1, layers_size[i + 1]);
		}
	}

	std::vector<double> forward(std::vector<double> in_data) const {
		if (in_data.size() != layers_size[0]) throw "bad input";

		mat in_layer(1, layers_size[0]);
		for (int i = 0; i < layers_size[0]; ++i)
			in_layer(0, i) = in_data[i];

		for (int i = 0; i < layers.size(); ++i) {
			in_layer = in_layer * layers[i] + biases[i];
			switch (layers_activation[i]) {
			case SIGMOID:
				sigmoid(in_layer);
				//temp = 1 / (exp(-temp) + 1);
				break;
			case RELU:
				relu(in_layer);
				//temp = max(temp, mat(temp.n_rows, temp.n_cols, fill::zeros));
				break;
			}
		}
		return conv_to< std::vector<double> >::from(in_layer.row(0));
	}

	void read_weitghs(std::istream& gin) {
		for (auto& i : layers) 
			for (int j = 0; j < i.n_rows; ++j)
				for (int k = 0; k < i.n_cols; ++k)
					gin >> i(j, k);
		

		for (auto& i : biases)
			for (int j = 0; j < i.n_rows; ++j)
				for (int k = 0; k < i.n_cols; ++k)
					gin >> i(j, k);

	}

	void read_weitghs_from_vector(const std::vector<double>& in_data) {
		int counter = 0;
		for (auto& i : layers)
			for (int j = 0; j < i.n_rows; ++j)
				for (int k = 0; k < i.n_cols; ++k)
					i(j, k) = in_data[counter++];

		for (auto& i : biases)
			for (int j = 0; j < i.n_rows; ++j)
				for (int k = 0; k < i.n_cols; ++k)
					i(j, k) = in_data[counter++];
	}

	void write_weitghs(std::string filePath) const {
		std::ofstream file(filePath);
		for (auto& i : layers)
			for (int j = 0; j < i.n_rows; ++j)
				for (int k = 0; k < i.n_cols; ++k)
					file << i(j, k) << " ";

		for (auto& i : biases)
			for (int j = 0; j < i.n_rows; ++j)
				for (int k = 0; k < i.n_cols; ++k)
					file << i(j, k) << " ";

		file.close();
	}

	int getParamsNumber() const {
		int res = 0;
		for (int i = 0; i < layers_size.size() - 1; ++i) {
			res += layers_size[i] * layers_size[i + 1];
		}
		for (int i = 1; i < layers_size.size(); ++i) {
			res += layers_size[i];
		}
		return res;
	}
private:
	const std::vector<int> layers_size;
	const std::vector<activation_type> layers_activation;
	std::vector<mat> layers;

	std::vector<mat> biases;

};
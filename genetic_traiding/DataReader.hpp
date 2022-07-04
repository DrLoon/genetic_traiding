#pragma once
#include<iostream>
#include<fstream>
#include<vector>

class DataReader {
	using vectorD = std::vector<double>;
	using vectorI = std::vector<int>;
	using ifstream = std::ifstream;
	using string = std::string;
public:
	DataReader(const string _file_name) : file_name(_file_name)
	{
		if (!ifstream(file_name)) throw "Can't open input file!";
	}

	int data_len() const {
		ifstream count_file_copy(file_name);
		std::istreambuf_iterator<char> begin(count_file_copy), end;
		auto dataset_size = (int)std::count(begin, end, char('\n')) - 1;
		return dataset_size;
	}

	std::pair<vectorD, vectorD> split_train_test(double train_persent) const {
		clock_t read = clock();
		int dataset_size = data_len();
		int train_size = (int)(dataset_size * train_persent);
		int test_size = dataset_size - train_size;

		vectorD train(train_size);
		vectorD test(test_size);

		std::string first_str;
		ifstream file_stream(file_name);
		file_stream >> first_str;
		std::cout << "start reading, first word is [ " << first_str << " ], dataset size is " << dataset_size << " [train " << train_size << " and test " << test_size << "]\n";
		for (int i = 0; i < train_size; i++)
		{
			file_stream >> train[i];
		}
		for (int i = 0; i < test_size; i++)
		{
			file_stream >> test[i];
		}
		std::cout << "reading is done, time is " << (double)(clock() - read) / CLOCKS_PER_SEC << " sec\n";

		return std::make_pair(train, test);
	}
private:
	const string file_name;
};

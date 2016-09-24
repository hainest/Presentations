#include <fstream>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>
#include <iomanip>

int main(int argc, const char *argv[]) {
	if (argc != 3) {
		std::cerr << "Usage: " << argv[0] << " input_file alpha\n";
		return -1;
	}

	std::ifstream fin{argv[1], std::ios::binary};

	if (!fin) {
		std::cerr << "Unable to open '" << argv[1] << "'\n";
		return -1;
	}

	const float alpha = std::stof(argv[2]);
	size_t N = 0;
	fin.read(reinterpret_cast<char *>(&N), sizeof(decltype(N)));

	std::vector<float> v(N);
	fin.read(reinterpret_cast<char *>(v.data()), N * sizeof(decltype(v)::value_type));

	float sum{};
	for(size_t i=0; i<N; i++) {
		sum += alpha * v[i] + v[i];
	}
	std::cout << "sum = " << std::scientific << std::setprecision(8) << sum << "\n";
}

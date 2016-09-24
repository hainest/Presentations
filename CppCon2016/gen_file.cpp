#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <algorithm>
#include <vector>

int main(int argc, const char *argv[]) {
	if (argc != 3) {
		std::cerr << "Usage: " << argv[0] << " output_file n\n"
				  << "NOTE: n is the exponent such that N = 2**n numbers are generated\n";
		return -1;
	}

	std::ofstream fout{argv[1], std::ios::binary};

	if (!fout) {
		std::cerr << "Unable to open '" << argv[1] << "'\n";
		return -1;
	}

	const size_t N = 1UL << std::stoul(argv[2]);

	std::random_device device;
	std::mt19937 gen(device());
	std::uniform_real_distribution<float> dist(0.1, 0.9);

	std::vector<decltype(dist)::result_type> v(N);
	std::generate_n(v.begin(), N, [&dist, &gen]() { return dist(gen); });

	fout.write(reinterpret_cast<const char *>(&N), sizeof(decltype(N)));
	fout.write(reinterpret_cast<const char *>(v.data()), N * sizeof(decltype(v)::value_type));
}

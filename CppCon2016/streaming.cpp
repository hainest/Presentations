#include <memory>
#include <vector>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <cstdio>
#include "stopwatch.hpp"
#include <fstream>
#include <iostream>
#include <immintrin.h>
#include <string>
#include "aligned_allocator.hpp"

struct Vtr;

struct Sax {
	const float& a;
	const Vtr& x;
	Sax(const float& d, const Vtr &v) :
			a { d }, x { v } {
	}
};

inline Sax operator*(const float& d, const Vtr& v) {
	return Sax(d, v);
}

struct Saxpy {
	const float &a;
	const Vtr &x;
	const Vtr &y;
	Saxpy(const Sax& s, const Vtr& u) :
			a { s.a }, x { s.x }, y { u } {
	}
};

inline Saxpy operator+(const Sax& s, const Vtr& u) {
	return Saxpy(s, u);
}

struct Vtr {
	using value_type = float;
	using iterator = float*;
	using const_iterator = float const*;

#ifdef INTRINSICS
	using my_allocator = aligned_allocator<float, 32>;
#else
	using my_allocator = std::allocator<float>;
#endif

	value_type * __restrict__ data_;
	size_t size;
	my_allocator allocator;

	Vtr(size_t _size) :
			size { _size } {
		data_ = allocator.allocate(size);
	}
	Vtr(const Saxpy& sp) {
		*this = sp;
	}
	const_iterator begin() const {
		return data_;
	}
	iterator begin() {
		return data_;
	}
	const_iterator end() const {
		return data_ + size;
	}
	iterator end() {
		return data_ + size;
	}
	const_iterator data() const {
		return data_;
	}
	iterator data() {
		return data_;
	}
	value_type operator[](size_t i) const {
		return data_[i];
	}
	Vtr& operator=(Saxpy const& sp) {
		if (!data_) {
			size = sp.x.size;
			data_ = allocator.allocate(size);
		}
#ifdef INTRINSICS
		const __m256 a = _mm256_set1_ps(sp.a);

		const auto x = sp.x.data_;
		const auto y = sp.y.data_;
		auto c = data_;

		for (size_t i = 0; i < size; i+=16) {
			const __m256 x1 = _mm256_load_ps(x + i + 0);
			const __m256 x2 = _mm256_load_ps(x + i + 8);
			const __m256 y1 = _mm256_load_ps(y + i + 0);
			const __m256 y2 = _mm256_load_ps(y + i + 8);

			const __m256 r1 = _mm256_fmadd_ps(a, x1, y1);
			const __m256 r2 = _mm256_fmadd_ps(a, x2, y2);

			_mm256_stream_ps(c + i + 0, r1);
			_mm256_stream_ps(c + i + 8, r2);
		}
#elif defined(CSTYLE)
		for(size_t i = 0; i < size; ++i) {
			data_[i] = sp.a * sp.x[i] + sp.y[i];
		}
#else
		std::transform(sp.x.begin(), sp.x.end(), sp.y.begin(), begin(), [&sp](float x, float y) {return sp.a * x + y;});
#endif

		return *this;
	}
	~Vtr() {
		if (data_)
			allocator.deallocate(data_, size);
	}
};

int main(int argc, char const* argv[]) {
	if (argc < 3) {
		std::cerr << "Usage: " << argv[0] << " infile sum\n";
		return -1;
	}

	std::ifstream fin { argv[1], std::ios::binary };
	if (!fin) {
		std::cerr << "Unable to open " << argv[1] << '\n';
		return -1;
	}

	constexpr size_t num_iterations = 100;
	size_t N = 0;
	fin.read(reinterpret_cast<char*>(&N), sizeof(N));

	std::vector<float> timings;
	timings.reserve(num_iterations);

	stopwatch<std::milli, float> sw;

	Vtr a(N), b(N), c(N);

	fin.read(reinterpret_cast<char*>(a.data()), N * sizeof(float));
	std::copy_n(a.begin(), N, b.begin());

	// Run it once to "prime" the cache
	c = 3.0f * a + b;

	asm volatile("saxpy_loop_begin%=:" :);
	for (size_t i = 0; i < num_iterations; ++i) {
		sw.start();
		c = 3.0f * a + b;
		sw.stop();
		timings.push_back(sw.count());
	}
	asm volatile("saxpy_loop_end%=:" :);

	float comparison_sum = std::stof(argv[2]);
	auto avg = std::accumulate(timings.begin(), timings.end(), 0.0f) / num_iterations;
	auto var = std::accumulate(timings.begin(), timings.end(), 0.0f, [avg](float x, float y) {return x + (y-avg)*(y-avg);});
	auto pdiff = [](float a, float b) {return 100.0f * std::fabs(a-b) / ((a+b)/2.0f);};
	auto sum = std::accumulate(c.begin(), c.end(), 0.0f);
	std::printf("%.3f,%.3f,%E\n", avg, std::sqrt(var / (num_iterations - 1)), pdiff(sum, comparison_sum));
}

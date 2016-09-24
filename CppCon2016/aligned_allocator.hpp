#ifndef ALIGNED_ALLOCATOR_HPP_
#define ALIGNED_ALLOCATOR_HPP_

#include <malloc.h>
#include <new>
#include <stdexcept>
#include <sstream>

template<typename T, size_t Align = 16>
class aligned_allocator {
public:
	using value_type = T;

	aligned_allocator() noexcept = default;
	aligned_allocator(const aligned_allocator&) noexcept = default;
	~aligned_allocator() noexcept = default;

	template<typename U>
	struct rebind {
		typedef aligned_allocator other;
	};

	T* allocate(size_t n) {
		T *data = static_cast<T*>(aligned_alloc(Align, n * sizeof(T)));
		if (!data) {
			std::stringstream ss;
			ss << "Unable to allocate " << n * sizeof(T) / 1024.0f << " MB of memory.\n";
			throw std::runtime_error { ss.str() };
		}
		return data;
	}

	void deallocate(T *p, __attribute__((unused)) size_t n) {
		if (p)
			free(p);
	}
};

template<class T, class U>
bool operator==(const aligned_allocator<T>&, const aligned_allocator<U>&) noexcept {
	return false;
}
template<class T, class U>
bool operator !=(const aligned_allocator<T>&a, const aligned_allocator<U>&b) noexcept {
	return !(a == b);
}

#endif

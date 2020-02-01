#include "LinearProbingHashSet.h"

#include <iostream>
#include <chrono>
#include <array>
#include <unordered_set>
#include <random>

class Stopwatch {
public:
	Stopwatch() {
		m_Start = std::chrono::system_clock::now();
	}

	void stop(uint32_t count) {
		auto end = std::chrono::system_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - m_Start);
		std::cout << "\tBenchamrking of " << count << " elements finished in " << elapsed.count() << " microseconds" << std::endl;
	}

private:
	std::chrono::system_clock::time_point m_Start;
};

enum class SetType {
	Std,
	Hs
};

template<class SetT>
void benchInsertIntSet(uint32_t count) {
	std::cout << "--- Insert" << std::endl;
	Stopwatch sw{};

	SetT set;
	for (uint32_t i = 0; i < count; ++i) {
		set.insert(i);
	}

	sw.stop(count);
}

template<class SetT, SetType Type>
void benchContainsInserted(uint32_t count) {
	std::cout << "--- Contains Inserted" << std::endl;
	
	SetT set;
	for (uint32_t i = 0; i < count; ++i) {
		set.insert(i);
	}
	
	std::default_random_engine el(count);
	std::uniform_int_distribution<> dist(0, count - 1);

	Stopwatch sw{};

	for (uint32_t i = 0; i < count; ++i) {
		if constexpr (Type == SetType::Std) {
			if (set.find(dist(el)) == set.end())
				std::cout << "Fail" << std::endl;
		} else {
			if (!set.contains(dist(el)))
				std::cout << "Fail" << std::endl;
		}
	}

	sw.stop(count);

	#if defined(TESTING)
		if constexpr (Type != SetType::Std)
			std::cout << "Elements per query: " << 1.0f * set.ElementsTested / set.QueryCount << std::endl;
	#endif
}

template<class SetT, SetType Type>
void benchContainsNotInserted(uint32_t count) {
	std::cout << "--- Contains Not Inserted" << std::endl;

	SetT set;
	for (uint32_t i = 0; i < count; ++i) {
		set.insert(i);
	}

	std::default_random_engine el(count);
	std::uniform_int_distribution<uint32_t> dist(count, 2 * count - 1);

	Stopwatch sw{};

	for (uint32_t i = 0; i < count; ++i) {
		if constexpr (Type == SetType::Std) {
			if (set.find(dist(el)) != set.end())
				std::cout << "Fail" << std::endl;
		} else {
			if (set.contains(dist(el)))
				std::cout << "Fail" << std::endl;
		}
	}

	sw.stop(count);
}

template<class SetT, SetType Type>
void benchRandomUsage(uint32_t count) {
	std::cout << "--- Random Usage" << std::endl;

	std::default_random_engine el(count);
	std::uniform_int_distribution<uint32_t> dist(0, count - 1);

	uint64_t found = 0;

	Stopwatch sw{};

	SetT set;
	for (uint32_t i = 0; i < count; ++i) {
		auto num = dist(el);
		auto op = num % 4;
		if (op == 0) {
			set.insert(num);
		} else if (op == 1) {
			if constexpr (Type == SetType::Std) {
				set.erase(num);
			} else {
				set.remove(num);
			}
		} else if (op == 2) {
			set.insert(num);
			if constexpr (Type == SetType::Std) {
				set.erase(num);
			} else {
				set.remove(num);
			}
		} else {
			if constexpr (Type == SetType::Std) {
				if (set.find(dist(el)) != set.end()) {
					++found;
				}
			} else {
				if (set.contains(dist(el))) {
					++found;
				}
			}
		}
	}

	sw.stop(count);

	std::cout << "Checksum found: " << found << std::endl;
}

std::vector<uint32_t> sizes = {
	100,
	#if defined (NDEBUG)
		1000,
		10000,
		100000,
		1000000,
		10000000,
		//100000000
	#endif
};

int main() {
	
	#if 0
	hs::LPHashSet<uint32_t, hs::LPHashSetPolicy::SSE> set;

	for (uint32_t i = 0; i < 1000000; ++i) {
		set.insert(i);
	}

	std::default_random_engine el(42);
	std::uniform_int_distribution<uint32_t> dist(0, 1000000 - 1);

	std::cout << "Starting contains" << std::endl;

	for (uint32_t i = 0; i < 10000000; ++i) {
		for (uint32_t j = 0; j < 10000000; ++j) {
			if (!set.contains(dist(el)))
				std::cout << "Fail" << std::endl;
		}
	}

	return 0;
	#endif

	std::cout << "\nhs::LPHashset" << std::endl;
	for (const auto size : sizes) {
		//benchInsertIntSet<hs::LPHashSet<uint32_t, hs::LPHashSetPolicy::Simple>>(size);
		//benchContainsInserted<hs::LPHashSet<uint32_t, hs::LPHashSetPolicy::Simple>, SetType::Hs>(size);
		//benchContainsNotInserted<hs::LPHashSet<uint32_t, hs::LPHashSetPolicy::Simple>, SetType::Hs>(size);
		benchRandomUsage<hs::LPHashSet<uint32_t, hs::LPHashSetPolicy::Simple>, SetType::Hs>(size);
	}

	std::cout << "\nhs::LPHashset SSE" << std::endl;
	for (const auto size : sizes) {
		//benchInsertIntSet<hs::LPHashSet<uint32_t, hs::LPHashSetPolicy::Simple>>(size);
		//benchContainsInserted<hs::LPHashSet<uint32_t, hs::LPHashSetPolicy::SSE>, SetType::Hs>(size);
		//benchContainsNotInserted<hs::LPHashSet<uint32_t, hs::LPHashSetPolicy::SSE>, SetType::Hs>(size);
		benchRandomUsage<hs::LPHashSet<uint32_t, hs::LPHashSetPolicy::SSE>, SetType::Hs>(size);
	}

	std::cout << "\nhs::LPHashset AVX" << std::endl;
	for (const auto size : sizes) {
		//benchInsertIntSet<hs::LPHashSet<uint32_t, hs::LPHashSetPolicy::Simple>>(size);
		//benchContainsInserted<hs::LPHashSet<uint32_t, hs::LPHashSetPolicy::AVX>, SetType::Hs>(size);
		//benchContainsNotInserted<hs::LPHashSet<uint32_t, hs::LPHashSetPolicy::SSE>, SetType::Hs>(size);
		benchRandomUsage<hs::LPHashSet<uint32_t, hs::LPHashSetPolicy::SSE>, SetType::Hs>(size);
	}

	std::cout << "\nstd::unordered_set" << std::endl;
	for (const auto size : sizes) {
		//benchInsertIntSet<std::unordered_set<uint32_t, hs::DefaultHash>>(size);
		//benchContainsInserted<std::unordered_set<uint32_t, hs::DefaultHash>, SetType::Std>(size);
		//benchContainsNotInserted<std::unordered_set<uint32_t, hs::DefaultHash>, SetType::Std>(size);
		benchRandomUsage<std::unordered_set<uint32_t, hs::DefaultHash>, SetType::Std>(size);
	}

    return 0;
}

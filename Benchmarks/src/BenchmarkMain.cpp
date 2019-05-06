#include "LinearProbingHashSet.h"

#include <iostream>
#include <chrono>
#include <array>
#include <unordered_set>


template<class SetT>
void benchInsertIntSet(uint32_t count) {
	auto start = std::chrono::system_clock::now();
    
	SetT set;
    for (uint32_t i = 0; i < count; ++i) {
        set.insert(i);
    }

	auto end = std::chrono::system_clock::now();

	auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

	std::cout << "Benchamrking of " << count << " elements finished in " << elapsed.count() << " microseconds" << std::endl;
}

std::array<uint32_t, 7> sizes = {
	100,
	1000,
	10000,
	100000,
	1000000,
	10000000,
	100000000
};

int main() {
	
	benchInsertIntSet<hs::LPHashSet<uint32_t>>(4000000);
	//benchInsertIntSet<std::unordered_set<uint32_t>>(40000000);

	//return 0;

	std::cout << "hs::LPHashset" << std::endl;
	for (const auto size : sizes) {
		benchInsertIntSet<hs::LPHashSet<uint32_t>>(size);
	}

	std::cout << "std::unordered_set" << std::endl;
	for (const auto size : sizes) {
		benchInsertIntSet<std::unordered_set<uint32_t>>(size);
	}

    return 0;
}

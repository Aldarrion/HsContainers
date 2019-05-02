
#include "HashSet.h"
#include "LinearProbingHashSet.h"

#include <iostream>



int main() {
	hs::LPHashSet<int> set;
	set.insert(1);

	if (set.contains(1)) {
		std::cout << "set contains 1";
	}

	return 0;
}

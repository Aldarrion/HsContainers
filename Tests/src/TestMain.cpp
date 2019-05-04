
#include "HashSet.h"
#include "LinearProbingHashSet.h"

#include <iostream>
#include "gtest/gtest.h"

using TestedSet = hs::LPHashSet<int>;

//-----------------------------------------------------------------------------
TEST(HashSetBasic, Contains_OnInserted_ReturnsTrue) {
	TestedSet set;
	set.insert(1);
	const bool isFound = set.contains(1);

	EXPECT_TRUE(isFound);
}

//-----------------------------------------------------------------------------
TEST(HashSetBasic, Contains_OnEmptySet_ReturnsFalse) {
	TestedSet set;

	const bool isFound = set.contains(1);

	EXPECT_FALSE(isFound);
}

//-----------------------------------------------------------------------------
TEST(HashSetBasic, Contains_OnNotInserted_ReturnsFalse) {
	TestedSet set;
	set.insert(1);
	const bool isFound = set.contains(2);

	EXPECT_FALSE(isFound);
}

//-----------------------------------------------------------------------------
TEST(HashSetBasic, Insert_MoreThanCapacity_DoublesCapacity) {
	TestedSet set;

	size_t originalCapacity = set.capacity();
	for (int i = 0; i < originalCapacity; ++i) {
		set.insert(i);
	}

	EXPECT_EQ(set.capacity(), originalCapacity * 2);
}

//-----------------------------------------------------------------------------
TEST(HashSetBasic, ContainsInserted_AfterRehash_ReturnsTrue) {
	TestedSet set;

	size_t originalCapacity = set.capacity();
	for (int i = 0; i < originalCapacity; ++i) {
		set.insert(i);
	}

	for (int i = 0; i < originalCapacity; ++i) {
		bool contains = set.contains(i);
		EXPECT_TRUE(contains);
	}
}

//-----------------------------------------------------------------------------
TEST(HashSetBasic, ContainsNotInserted_AfterRehash_ReturnsFalse) {
	TestedSet set;

	size_t originalCapacity = set.capacity();
	for (int i = 0; i < originalCapacity; ++i) {
		set.insert(i);
	}

	EXPECT_FALSE(set.contains(static_cast<int>(originalCapacity) + 1));
}
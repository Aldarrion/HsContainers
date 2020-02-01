
#include "HashSet.h"
#include "LinearProbingHashSet.h"

#include <iostream>
#include "gtest/gtest.h"

#include <unordered_set>

using TestedSet = hs::LPHashSet<int, hs::LPHashSetPolicy::SSE>;

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
TEST(HashSetBasic, Contains_InsertMultiple_Works) {
	TestedSet set;
	set.insert(1);
	set.insert(2);
	set.insert(3);
	set.insert(4);
	set.insert(5);

	EXPECT_TRUE(set.contains(1));
	EXPECT_TRUE(set.contains(2));
	EXPECT_TRUE(set.contains(3));
	EXPECT_TRUE(set.contains(4));
	EXPECT_TRUE(set.contains(5));

	EXPECT_FALSE(set.contains(6));
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

//-----------------------------------------------------------------------------
TEST(HashSetBasic, Remove_ByDefault_RemovesElement) {
	TestedSet set;

	set.insert(1);
	EXPECT_TRUE(set.contains(1));
	set.remove(1);
	EXPECT_FALSE(set.contains(1));
}



#include <stdint.h>

namespace hs {

template<class TKey>
class HashSet {
public:
	using Hash_t = int32_t;

	void insert(const TKey& key) {
		// TODO
	}

	bool contains(const TKey& key) {
		// TODO
		return false;
	}

private:
	struct Entry {
		TKey key_;
		int32_t distance_;
	};


};

} // namespace hs

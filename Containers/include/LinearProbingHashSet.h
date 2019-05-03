
#include <stdint.h>

namespace hs {

//-----------------------------------------------------------------------------
using Hash_t = uint32_t;

//-----------------------------------------------------------------------------
template<class TKey>
using HashFunc_t = Hash_t(*)(const TKey&);

//-----------------------------------------------------------------------------
template<class TKey>
Hash_t defaultHashFunc(const TKey&) {
	static_assert("defaultHashFunc must be specialized");
}

//-----------------------------------------------------------------------------
template<>
Hash_t defaultHashFunc<int>(const int& key) {
	return 17 + static_cast<Hash_t>(key) * 2654435761;
}

//-----------------------------------------------------------------------------
template<class TKey, HashFunc_t<TKey> hashFunc = defaultHashFunc<TKey>>
class LPHashSet {
public:
	//-----------------------------------------------------------------------------
	LPHashSet()
		: count_(0) 
		, exponent_(4)
	{
		capacity_ = 1 << exponent_;
		data_ = new TKey[capacity_];
		metadata_ = new uint8_t[capacity_];
		memset(metadata_, 0, capacity_);
	}
	//-----------------------------------------------------------------------------
	~LPHashSet() {
		delete[] metadata_;
		delete[] data_;
	}
	//-----------------------------------------------------------------------------
	void insert(const TKey& key) {
		TKey* insertSpot = findInsertSpot(key);
		// Spot not found or the key is already present
		if (insertSpot == nullptr)
			return;

		// put key to place
		*insertSpot = key;

		if (loadFactor() > MAX_LOAD_FACTOR) {
			rehash();
		}
	}
	//-----------------------------------------------------------------------------
	bool contains(const TKey& key) {
		const Hash_t hash = hashFunc(key);
		const uint8_t hashLow = computeHashLow(hash);
		const uint32_t hashHigh = computeHashHigh(hash);
		const uint32_t modMask = capacity_ - 1;
		const uint32_t startIndex = hashHigh & modMask;

		for (int i = startIndex;;) {
			// data_[i] is empty and not a tombstone, return false
			if ((metadata_[i] & FULL_MASK) == 0 && (metadata_[i] & TOMBSTONE_MASK) == 0)
				return false;
			
			// if metadata_[i] has the same hash as `hash` && data_[i] == key // return true
			if ((metadata_[i] & LOW_MASK) == hashLow && data_[i] == key)
				return true;

			i = (i + 1) & modMask;
			if (i == startIndex) // Wrapped around
				return false;
		}

		return false;
	}
	//-----------------------------------------------------------------------------
	size_t size() {
		return count_;
	}

private:
	static constexpr uint32_t FULL_MASK = 1 << 7;
	static constexpr uint32_t TOMBSTONE_MASK = 1 << 6;
	static constexpr uint32_t LOW_MASK = 0x7F; // 0b0111_1111
	static constexpr float MAX_LOAD_FACTOR = 0.8f;

	size_t count_;
	uint32_t capacity_;
	size_t exponent_;

	TKey* data_;
	uint8_t* metadata_;

	//-----------------------------------------------------------------------------
	uint32_t computeHashHigh(uint32_t hash) const {
		return hash >> 8;
	}
	//-----------------------------------------------------------------------------
	uint8_t computeHashLow(uint32_t hash) const {
		return static_cast<uint8_t>(hash & LOW_MASK);
	}
	//-----------------------------------------------------------------------------
	float loadFactor() const {
		return static_cast<float>(count_) / capacity_;
	}
	//-----------------------------------------------------------------------------
	void rehash() {
		// TODO
	}
	//-----------------------------------------------------------------------------
	TKey* findInsertSpot(const TKey& key) {
		const Hash_t hash = hashFunc(key);
		const uint8_t hashLow = computeHashLow(hash);
		const uint32_t hashHigh = computeHashHigh(hash);
		const uint32_t modMask = capacity_ - 1;
		const uint32_t startIndex = hashHigh & modMask;

		for (int i = startIndex;;) {
			// data_[i] is empty - this is the spot
			if ((metadata_[i] & FULL_MASK) == 0) {
				metadata_[i] = hashLow | FULL_MASK;
				return &data_[i];
			}

			// if key already present, disallow second insertion
			if ((metadata_[i] & LOW_MASK) == hashLow && data_[i] == key)
				return nullptr;

			i = (i + 1) & modMask;
			// Wrap around is not possible - it would mean that the table is 100% full
		}

		return nullptr;
	}
};

} // namespace hs

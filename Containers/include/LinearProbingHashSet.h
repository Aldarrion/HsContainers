
#include <stdint.h>

namespace hs {

//-----------------------------------------------------------------------------
using Hash_t = uint64_t;

//-----------------------------------------------------------------------------
template<class TKey>
using HashFunc_t = Hash_t(*)(const TKey&);

//-----------------------------------------------------------------------------
template<class TKey>
Hash_t defaultHashFunc(const TKey&) {
	static_assert(false, "defaultHashFunc must be specialized");
}

//-----------------------------------------------------------------------------
template<>
Hash_t defaultHashFunc<int>(const int& key) {
	return 17 + static_cast<Hash_t>(key) * 2654435761;
}

//-----------------------------------------------------------------------------
template<>
Hash_t defaultHashFunc<uint32_t>(const uint32_t& key) {
	return 17 + static_cast<Hash_t>(key) * 2654435761;
}

//-----------------------------------------------------------------------------
template<class TKey, HashFunc_t<TKey> hashFunc = defaultHashFunc<TKey>>
class LPHashSet {
public:
	//-----------------------------------------------------------------------------
	LPHashSet()
		: count_(0) 
		, exponent_(3)
	{
		capacity_ = static_cast<size_t>(1) << exponent_;
		allocArrays();
	}
	//-----------------------------------------------------------------------------
	~LPHashSet() {
		for (size_t i = 0; i < capacity_; ++i) {
			if (metadata_[i] & VALID_ELEMENT_MASK) {
				data_[i].~TKey();
			}
		}

		free(metadata_);
		free(data_);
	}
	//-----------------------------------------------------------------------------
	void insert(const TKey& key) {
		TKey* insertSpot = findInsertSpot(key);
		
		// Spot not found or the key is already present
		if (insertSpot == nullptr)
			return;

		*insertSpot = key;
		++count_;

		if (loadFactor() > MAX_LOAD_FACTOR) {
			rehash();
		}
	}
	//-----------------------------------------------------------------------------
	void remove(const TKey& key) {
		size_t idx = indexOf(key);
		if (idx == NPOS)
			return;

		metadata_[idx] = TOMBSTONE_MASK;
		data_[idx].~TKey();
		--count_;

		// TODO should the hashset shrink?
	}
	//-----------------------------------------------------------------------------
	bool contains(const TKey& key) const {
		return indexOf(key) != NPOS;
	}
	//-----------------------------------------------------------------------------
	size_t count() const {
		return count_;
	}
	//-----------------------------------------------------------------------------
	size_t capacity() const {
		return capacity_;
	}

private:
	static constexpr Hash_t VALID_ELEMENT_MASK = 1 << 7;
	static constexpr uint8_t TOMBSTONE_MASK = 1 << 6;
	static constexpr uint8_t LOW_MASK = 0x7F; // 0b0111_1111
	static constexpr float MAX_LOAD_FACTOR = 0.8f;
	static constexpr size_t NPOS = -1;

	size_t count_;
	size_t capacity_;
	size_t exponent_;

	TKey* data_;
	uint8_t* metadata_;

	//-----------------------------------------------------------------------------
	Hash_t computeHashHigh(Hash_t hash) const {
		return hash >> 8;
	}
	//-----------------------------------------------------------------------------
	uint8_t computeHashLow(Hash_t hash) const {
		return static_cast<uint8_t>(hash & LOW_MASK);
	}
	//-----------------------------------------------------------------------------
	float loadFactor() const {
		return static_cast<float>(count_) / capacity_;
	}
	//-----------------------------------------------------------------------------
	void allocArrays() {
		data_ = static_cast<TKey*>(malloc(sizeof(TKey) * capacity_));
		metadata_ = static_cast<uint8_t*>(malloc(capacity_));
		memset(metadata_, 0, capacity_);
	}
	//-----------------------------------------------------------------------------
	void rehash() {
		++exponent_;
		Hash_t oldCapacity = capacity_;
		capacity_ = static_cast<size_t>(1) << exponent_;
		
		TKey* oldData = data_;
		uint8_t* oldMetadata = metadata_;

		allocArrays();

		for (size_t i = 0; i < oldCapacity; ++i) {
			if (oldMetadata[i] & VALID_ELEMENT_MASK) {
				// this insert may be optimized - duplicate keys do not have to be checked
				TKey* insertSpot = findInsertSpot(oldData[i]);
				*insertSpot = std::move(oldData[i]);
				oldData[i].~TKey();
			}
		}

		free(oldData);
		free(oldMetadata);
	}
	//-----------------------------------------------------------------------------
	TKey* findInsertSpot(const TKey& key) const {
		const Hash_t hash = hashFunc(key);
		const uint8_t hashLow = computeHashLow(hash);
		const Hash_t hashHigh = computeHashHigh(hash);
		const Hash_t modMask = capacity_ - 1;
		const Hash_t startIndex = hashHigh & modMask;

		for (Hash_t i = startIndex;;) {
			// data_[i] is empty - this is the spot
			if ((metadata_[i] & VALID_ELEMENT_MASK) == 0) {
				metadata_[i] = hashLow | VALID_ELEMENT_MASK;
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
	//-----------------------------------------------------------------------------
	size_t indexOf(const TKey& key) const {
		const Hash_t hash = hashFunc(key);
		const uint8_t hashLow = computeHashLow(hash);
		const Hash_t hashHigh = computeHashHigh(hash);
		const Hash_t modMask = capacity_ - 1;
		const Hash_t startIndex = hashHigh & modMask;

		// iterate metadata
		for (Hash_t i = startIndex;;) {
			if ((metadata_[i] & VALID_ELEMENT_MASK) == 0 && (metadata_[i] & TOMBSTONE_MASK) == 0)
				return NPOS;

			// if metadata_[i] has the same hash as `hash` && data_[i] == key // return true
			if ((metadata_[i] & LOW_MASK) == hashLow && data_[i] == key)
				return i;

			i = (i + 1) & modMask;
			if (i == startIndex) // Wrapped around
				return NPOS;
		}
	}
};

} // namespace hs

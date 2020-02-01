
#include <stdint.h>
#include <emmintrin.h>
#include <immintrin.h>

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

struct DefaultHash {
	size_t operator()(const uint32_t& key) const {
		return 17 + static_cast<size_t>(key) * 2654435761;
	}
};

enum class LPHashSetPolicy {
	Simple,
	SSE,
	AVX
};

//-----------------------------------------------------------------------------
template<class TKey, LPHashSetPolicy Policy, HashFunc_t<TKey> hashFunc = defaultHashFunc<TKey>>
class LPHashSet {
public:
	#define TESTING
	#if defined(TESTING)
		mutable uint64_t QueryCount{ 0 };
		mutable uint64_t ElementsTested{ 0 };
	#endif

	//-----------------------------------------------------------------------------
	LPHashSet()
		: count_(0) 
		, exponent_(5)
		, EMPTY_MASK_128(_mm_set1_epi8(VALID_ELEMENT_MASK | TOMBSTONE_MASK))
		, EMPTY_MASK_256(_mm256_set1_epi8(VALID_ELEMENT_MASK | TOMBSTONE_MASK))
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
		size_t idx = indexOfTemplate(key);
		if (idx == NPOS)
			return;

		metadata_[idx] = TOMBSTONE_MASK;
		data_[idx].~TKey();
		--count_;
	}
	//-----------------------------------------------------------------------------
	bool contains(const TKey& key) const {
		return indexOfTemplate(key) != NPOS;
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
	static constexpr Hash_t VALID_ELEMENT_MASK = 1 << 7;	// 0b1000_0000
	static constexpr uint8_t TOMBSTONE_MASK = 1 << 6;		// 0b0100_0000
	static constexpr uint8_t LOW_MASK = 0x7F;				// 0b0111_1111
	static constexpr float MAX_LOAD_FACTOR = 0.8f;
	static constexpr size_t NPOS = -1;
	const __m128i EMPTY_MASK_128; // TODO make static
	const __m256i EMPTY_MASK_256; // TODO make static

	size_t count_;
	size_t capacity_;
	size_t exponent_;

	TKey* data_;
	union {
		uint8_t* metadata_;
		__m128i* metadata_m128_;
		__m256i* metadata_m256_;
	};

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
	TKey* findInsertSpotSSE(const TKey& key) const {
		// TODO implement

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

		#if defined(TESTING)
			++QueryCount;
		#endif

		// iterate metadata
		for (Hash_t i = startIndex;;) {
			#if defined(TESTING)
				++ElementsTested;
			#endif
			if ((metadata_[i] & VALID_ELEMENT_MASK) == 0 && (metadata_[i] & TOMBSTONE_MASK) == 0)
				return NPOS;

			// if metadata_[i] has the same hash as `hash` && data_[i] == key // return true
			if ((metadata_[i] & LOW_MASK) == hashLow && data_[i] == key)
				return i;

			i = (i + 1) & modMask;
			if (i == startIndex) // Wrap around is possible if the table is full of tombstones
				return NPOS;
		}
	}
	//-----------------------------------------------------------------------------
	size_t indexOfSSE(const TKey& key) const {
		const Hash_t hash = hashFunc(key);
		const uint8_t hashLow = computeHashLow(hash);
		const Hash_t hashHigh = computeHashHigh(hash);
		const Hash_t modMask = capacity_ - 1;
		const Hash_t modMask128 = modMask >> 4;
		const Hash_t startIndex = hashHigh & modMask;

		const Hash_t start = startIndex >> 4;
		const Hash_t overlap = startIndex & 15;

		const __m128i elemMask = _mm_set1_epi8(hashLow | VALID_ELEMENT_MASK);
		const __m128i emptyMask = _mm_set1_epi8(0);

		#if defined(TESTING)
			++QueryCount;
		#endif

		// iterate metadata
		for (Hash_t i = start;;) {
			// if metadata_[i] has the same hash as `hash` && data_[i] == key // return true
			const __m128i eqResult = _mm_cmpeq_epi8(metadata_m128_[i], elemMask);
			int resultMask = _mm_movemask_epi8(eqResult);
			#if 1
			while (true) {
				#if defined(TESTING)
					++ElementsTested;
				#endif
				unsigned long firstSet;
				// _BitScanForward intrinsic is faster than manual bit iteration
				const char hasAnySet = _BitScanForward(&firstSet, resultMask);
				if (!hasAnySet)
					break;
				
				// Do comparison of the value
				const Hash_t dataIdx = (i << 4) + firstSet;
				if (data_[dataIdx] == key)
					return dataIdx;

				// Try the next one
				resultMask &= ~(1 << firstSet);
			}
			#endif

			// Check if we can escape - if we found an empty spot already
			const __m128i emptyResult = _mm_cmpeq_epi8(metadata_m128_[i], emptyMask);
			int emptyResultMask = _mm_movemask_epi8(emptyResult);
			unsigned long emptyFirstSet;
			const char emptyAnySet = _BitScanForward(&emptyFirstSet, emptyResultMask);
			if (emptyAnySet && (i != start || emptyFirstSet >= overlap))
				return NPOS;

			i = ((i + 1) & modMask128);
			if (i == start) // Wrap around is possible if the table is full of tombstones
				return NPOS;
		}
	}
	//-----------------------------------------------------------------------------
	size_t indexOfAVX(const TKey& key) const {
		const Hash_t hash = hashFunc(key);
		const uint8_t hashLow = computeHashLow(hash);
		const Hash_t hashHigh = computeHashHigh(hash);
		const Hash_t modMask = capacity_ - 1;
		const Hash_t modMask256 = modMask >> 5;
		const Hash_t startIndex = hashHigh & modMask;

		// For indexing 32byte chunks
		const Hash_t start = startIndex >> 5;
		const Hash_t overlap = startIndex & 31;

		const __m256i elemMask = _mm256_set1_epi8(hashLow | VALID_ELEMENT_MASK);
		const __m256i emptyMask = _mm256_setzero_si256();

		#if defined(TESTING)
			++QueryCount;
		#endif

		for (Hash_t i = start;;) {
			const __m256i eqResult = _mm256_cmpeq_epi8(metadata_m256_[i], elemMask);
			int resultMask = _mm256_movemask_epi8(eqResult);
			while (true) {
				#if defined(TESTING)
					++ElementsTested;
				#endif
				unsigned long firstSet;
				const char hasAnySet = _BitScanForward(&firstSet, resultMask);
				if (!hasAnySet)
					break;

				// Do comparison of the value
				const Hash_t dataIdx = (i << 5) + firstSet;
				if (data_[dataIdx] == key)
					return dataIdx;

				// Try the next one
				resultMask &= ~(1 << firstSet);
			}

			const __m256i emptyResult = _mm256_cmpeq_epi8(metadata_m256_[i], emptyMask);
			int emptyResultMask = _mm256_movemask_epi8(emptyResult);
			// TODO try masking out part of result before overlap (if start == i)
			unsigned long emptyFirstSet;
			const char emptyAnySet = _BitScanForward(&emptyFirstSet, emptyResultMask);
			if (emptyAnySet && (i != start || emptyFirstSet >= overlap))
				return NPOS;

			i = ((i + 1) & modMask256);
			if (i == start) // Wrap around is possible if the table is full of tombstones
				return NPOS;
		}
	}
	//-----------------------------------------------------------------------------
	size_t indexOfTemplate(const TKey& key) const {
		if constexpr (Policy == LPHashSetPolicy::SSE) {
			return indexOfSSE(key);
		} else if constexpr (Policy == LPHashSetPolicy::AVX) {
			return indexOfAVX(key);
		} else {
			return indexOf(key);
		}
	}
};



} // namespace hs

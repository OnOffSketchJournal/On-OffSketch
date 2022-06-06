#ifndef OO_SIMD_8_H
#define OO_SIMD_8_H

/*
 * On-Off sketch implemented by AVX2 SIMD instructions
 */

#include "bitset.h"
#include "Abstract.h"

template<typename DATA_TYPE, typename COUNT_TYPE, uint32_t SLOT_NUM>
class OO_SIMD_8_32_32 : public Abstract<DATA_TYPE, COUNT_TYPE> {
public:

    struct Bucket{
        uint32_t items[8];
        int32_t counters[8];

        inline uint32_t Match_Item(const uint32_t item){
            __m256i vec = _mm256_set1_epi32(item);
            __m256i cmp = _mm256_cmpeq_epi32(vec, _mm256_loadu_si256((__m256i*)items));
            return _mm256_movemask_ps((__m256)cmp);
        }

        inline uint32_t Match_Counter(const int32_t counter){
            __m256i vec = _mm256_set1_epi32(counter);
            __m256i cmp = _mm256_cmpeq_epi32(vec, _mm256_loadu_si256((__m256i*)counters));
            return _mm256_movemask_ps((__m256)cmp);
        }

        inline int32_t Query(const uint32_t item){
            uint32_t match = Match_Item(item);

            if (match != 0){
                uint32_t index = __builtin_ctz(match);
                return counters[index];
            }
            return 0;
        }
    };

    OO_SIMD_8_32_32(uint64_t memory) :
            length((double)memory / (sizeof(Bucket) + sizeof(int32_t) + (8 + 1) * BITSIZE)){
                printf("OO_FPI slot:%u\n",length * (8 + 1));
        buckets = new Bucket[length];
        sketch = new int32_t [length];

        memset(buckets, 0, length * sizeof(Bucket));
        memset(sketch, 0, length * sizeof(int32_t));

        bucketBitsets = new BitSet(8 * length);
        sketchBitsets = new BitSet(length);
    }

    ~OO_SIMD_8_32_32(){
        delete [] buckets;
        delete [] sketch;
        delete bucketBitsets;
        delete sketchBitsets;
    }

    void Insert(const DATA_TYPE item, const COUNT_TYPE window){
        uint32_t pos = this->hash(item) % length;

        uint32_t match = buckets[pos].Match_Item(item);

        if (match != 0){
            buckets[pos].counters[__builtin_ctz(match)] += bucketBitsets->SetByte(pos, match);
            return;
        }

        if(!sketchBitsets->Get(pos)){
            match = buckets[pos].Match_Counter(sketch[pos]);

            if (match == 0){
                sketch[pos] += 1;
                sketchBitsets->Set(pos);
            }
            else{
                uint32_t offset = __builtin_ctz(match);
                buckets[pos].items[offset] = item;
                buckets[pos].counters[offset] += 1;
                bucketBitsets->Set(pos, offset);
            }
        }
    }

    int32_t Query(const uint32_t item){
        return buckets[this->hash(item) % length].Query(item);
    }

    void NewWindow(const int32_t window){
        bucketBitsets->Clear();
        sketchBitsets->Clear();
    }

private:
    const uint32_t length;

    BitSet* bucketBitsets;
    Bucket* buckets;

    BitSet* sketchBitsets;
    int32_t* sketch;
};

template<typename DATA_TYPE, typename COUNT_TYPE, uint32_t SLOT_NUM>
class OO_SIMD_8_64_32 : public Abstract<DATA_TYPE, COUNT_TYPE> {
public:

    struct Bucket{
        uint64_t items[8];
        int32_t counters[8];

        inline uint32_t Match_Item(const uint64_t item){
            __m256i vec = _mm256_set1_epi64x(item);
            __m256i cmp = _mm256_cmpeq_epi64(vec, _mm256_loadu_si256((__m256i*)items));
            __m256i cmp1 = _mm256_cmpeq_epi64(vec, _mm256_loadu_si256((__m256i*)(&items[4])));

            return (_mm256_movemask_pd((__m256d)cmp1) << 4) |
                            _mm256_movemask_pd((__m256d)cmp);
        }

        inline uint32_t Match_Counter(const int32_t counter){
            __m256i vec = _mm256_set1_epi32(counter);
            __m256i cmp = _mm256_cmpeq_epi32(vec, _mm256_loadu_si256((__m256i*)counters));
            return _mm256_movemask_ps((__m256)cmp);
        }

        inline int32_t Query(const uint64_t item){
            uint32_t match = Match_Item(item);

            if (match != 0){
                uint32_t index = __builtin_ctz(match);
                return counters[index];
            }
            return 0;
        }
    };

    OO_SIMD_8_64_32(uint64_t memory) :
            length((double)memory / (sizeof(Bucket) + sizeof(int32_t) + (8 + 1) * BITSIZE)){
                printf("OO_FPI slot:%u\n",length * (8 + 1));
        buckets = new Bucket[length];
        sketch = new int32_t [length];

        memset(buckets, 0, length * sizeof(Bucket));
        memset(sketch, 0, length * sizeof(int32_t));

        bucketBitsets = new BitSet(8 * length);
        sketchBitsets = new BitSet(length);
    }

    ~OO_SIMD_8_64_32(){
        delete [] buckets;
        delete [] sketch;
        delete bucketBitsets;
        delete sketchBitsets;
    }

    void Insert(const DATA_TYPE item, const COUNT_TYPE window){
        uint32_t pos = this->hash(item) % length;

        uint32_t match = buckets[pos].Match_Item(item);

        if (match != 0){
            buckets[pos].counters[__builtin_ctz(match)] += bucketBitsets->SetByte(pos, match);
            return;
        }

        if(!sketchBitsets->Get(pos)){
            match = buckets[pos].Match_Counter(sketch[pos]);

            if (match == 0){
                sketch[pos] += 1;
                sketchBitsets->Set(pos);
            }
            else{
                uint32_t offset = __builtin_ctz(match);
                buckets[pos].items[offset] = item;
                buckets[pos].counters[offset] += 1;
                bucketBitsets->Set(pos, offset);
            }
        }
    }

    int32_t Query(const uint64_t item){
        return buckets[this->hash(item) % length].Query(item);
    }

    void NewWindow(const int32_t window){
        bucketBitsets->Clear();
        sketchBitsets->Clear();
    }

private:
    const uint32_t length;

    BitSet* bucketBitsets;
    Bucket* buckets;

    BitSet* sketchBitsets;
    int32_t* sketch;
};


#endif //OO_SIMD_8_H

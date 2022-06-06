#ifndef OO_SIMD_32_H
#define OO_SIMD_32_H

/*
 * On-Off sketch implemented by AVX2 SIMD instructions
 */

#include "bitset.h"
#include "Abstract.h"

template<typename DATA_TYPE, typename COUNT_TYPE, uint32_t SLOT_NUM>
class OO_SIMD_32_32_32 : public Abstract<DATA_TYPE, COUNT_TYPE> {
public:

    struct Bucket{
        uint32_t items[32];
        int32_t counters[32];

        inline uint32_t Match_Item(const __m256i& vec, uint32_t offset){
            __m256i cmp = _mm256_cmpeq_epi32(vec, _mm256_loadu_si256((__m256i*)(&items[offset << 3])));
            return _mm256_movemask_ps((__m256)cmp);
        }

        inline uint32_t Match_Counter(const __m256i& vec, uint32_t offset){
            __m256i cmp = _mm256_cmpeq_epi32(vec, _mm256_loadu_si256((__m256i*)(&counters[offset << 3])));
            return _mm256_movemask_ps((__m256)cmp);
        }

        inline int32_t Query(const uint32_t item){
        __m256i vec = _mm256_set1_epi32(item);

            for(uint32_t i = 0;i < 4;++i){
                uint32_t match = Match_Item(vec, i);
                if(match != 0){
                    uint32_t index = __builtin_ctz(match);
                    return counters[(i << 3) + index];
            }
            }

            return 0;
        }
    };

    OO_SIMD_32_32_32(uint64_t memory) :
            length((double)memory / (sizeof(Bucket) + sizeof(int32_t) + (32 + 1) * BITSIZE)){
                printf("OO_FPI slot:%u\n",length * (32 + 1));
        buckets = new Bucket[length];
        sketch = new int32_t [length];

        memset(buckets, 0, length * sizeof(Bucket));
        memset(sketch, 0, length * sizeof(int32_t));

        bucketBitsets = new BitSet(32 * length);
        sketchBitsets = new BitSet(length);
    }

    ~OO_SIMD_32_32_32(){
        delete [] buckets;
        delete [] sketch;
        delete bucketBitsets;
        delete sketchBitsets;
    }

    void Insert(const DATA_TYPE item, const COUNT_TYPE window){
        uint32_t pos = this->hash(item) % length;
        __m256i vec = _mm256_set1_epi32(item);

        for(uint32_t i = 0;i < 4;++i){
        uint32_t match = buckets[pos].Match_Item(vec, i);
            if(match != 0){
            buckets[pos].counters[(i << 3) + __builtin_ctz(match)] += bucketBitsets->SetByte((pos << 2) + i, match);
            return;
        }
        }

        if(!sketchBitsets->Get(pos)){
            vec = _mm256_set1_epi32(sketch[pos]);
        for(uint32_t i = 0;i < 4;++i){
                uint32_t match = buckets[pos].Match_Counter(vec, i);
                if(match != 0){
                    uint32_t offset = __builtin_ctz(match);
                    buckets[pos].items[(i << 3) + offset] = item;
                buckets[pos].counters[(i << 3) + offset] += 1;
                    bucketBitsets->Set((pos << 2) + i, offset);
            return;
                }
        }

            sketch[pos] += 1;
            sketchBitsets->Set(pos);
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
class OO_SIMD_32_64_32 : public Abstract<DATA_TYPE, COUNT_TYPE> {
public:

    struct Bucket{
        uint64_t items[32];
        int32_t counters[32];

        inline uint32_t Match_Item(const __m256i& vec, uint32_t offset){
            __m256i cmp = _mm256_cmpeq_epi64(vec, _mm256_loadu_si256((__m256i*)(&items[offset << 3])));
            __m256i cmp1 = _mm256_cmpeq_epi64(vec, _mm256_loadu_si256((__m256i*)(&items[(offset << 3) + 4])));

        return (_mm256_movemask_pd((__m256d)cmp1) << 4) |
                            _mm256_movemask_pd((__m256d)cmp);
        }

        inline uint32_t Match_Counter(const __m256i& vec, uint32_t offset){
            __m256i cmp = _mm256_cmpeq_epi32(vec, _mm256_loadu_si256((__m256i*)(&counters[offset << 3])));
            return _mm256_movemask_ps((__m256)cmp);
        }

        inline int32_t Query(const uint64_t item){
        __m256i vec = _mm256_set1_epi64x(item);

            for(uint32_t i = 0;i < 4;++i){
                uint32_t match = Match_Item(vec, i);
                if(match != 0){
                    uint32_t index = __builtin_ctz(match);
                    return counters[(i << 3) + index];
                }
            }

            return 0;
        }
    };

    OO_SIMD_32_64_32(uint64_t memory) :
            length((double)memory / (sizeof(Bucket) + sizeof(int32_t) + (32 + 1) * BITSIZE)){
                printf("OO_FPI slot:%u\n",length * (32 + 1));
        buckets = new Bucket[length];
        sketch = new int32_t [length];

        memset(buckets, 0, length * sizeof(Bucket));
        memset(sketch, 0, length * sizeof(int32_t));

        bucketBitsets = new BitSet(32 * length);
        sketchBitsets = new BitSet(length);
    }

    ~OO_SIMD_32_64_32(){
        delete [] buckets;
        delete [] sketch;
        delete bucketBitsets;
        delete sketchBitsets;
    }

    void Insert(const DATA_TYPE item, const COUNT_TYPE window){
        uint32_t pos = this->hash(item) % length;
        __m256i vec = _mm256_set1_epi64x(item);

        for(uint32_t i = 0;i < 4;++i){
            uint32_t match = buckets[pos].Match_Item(vec, i);
            if(match != 0){
                buckets[pos].counters[(i << 3) + __builtin_ctz(match)] += bucketBitsets->SetByte((pos << 2) + i, match);
                return;
            }
        }

        if(!sketchBitsets->Get(pos)){
            vec = _mm256_set1_epi32(sketch[pos]);
            for(uint32_t i = 0;i < 4;++i){
                uint32_t match = buckets[pos].Match_Counter(vec, i);
                if(match != 0){
                    uint32_t offset = __builtin_ctz(match); 
                    buckets[pos].items[(i << 3) + offset] = item;
                    buckets[pos].counters[(i << 3) + offset] += 1;
                    bucketBitsets->Set((pos << 2) + i, offset);
                    return;
                }
            }
            
            sketch[pos] += 1;
            sketchBitsets->Set(pos);
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

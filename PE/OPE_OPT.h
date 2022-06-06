#ifndef OPE_OPT_H
#define OPE_OPT_H

/*
 * On-Off sketch on persistence estimation
 */

#include "bitset.h"
#include "Abstract.h"

template<typename DATA_TYPE,typename COUNT_TYPE>
class OPE_OPT : public Abstract<DATA_TYPE, COUNT_TYPE>{
public:

    OPE_OPT(uint32_t _hash_num, uint32_t _length):
            hash_num(_hash_num), length(_length){
        counters = new COUNT_TYPE* [hash_num];
        bitsets = new BitSet* [hash_num];
        for(uint32_t i = 0;i < hash_num;++i){
            counters[i] = new COUNT_TYPE [length];
            bitsets[i] = new BitSet(length);
            memset(counters[i], 0, length * sizeof(COUNT_TYPE));
        }
    }

    ~OPE_OPT(){
        for(uint32_t i = 0;i < hash_num;++i){
            delete [] counters[i];
            delete bitsets[i];
        }
        delete [] counters;
        delete [] bitsets;
    }

    void Insert(const DATA_TYPE item, const COUNT_TYPE window){
        COUNT_TYPE value[hash_num];
        uint32_t pos[hash_num];
        pos[0]=this->hash(item, 0) % length;
        value[0]=counters[0][pos[0]]-bitsets[0]->Get(pos[0]);
        COUNT_TYPE min_val=value[0];
        for(uint32_t i = 1;i < hash_num;++i){
            pos[i] = this->hash(item, i) % length;
            value[i]=counters[i][pos[i]]-(bitsets[i]->Get(pos[i]));
            min_val=MIN(min_val,value[i]);
        }
        ++min_val;
        for(uint32_t i = 0;i < hash_num;++i){
            if(counters[i][pos[i]]<min_val)
            {
                bitsets[i]->Set(pos[i]);
                ++counters[i][pos[i]];
            }
        }
    }

    COUNT_TYPE Query(const DATA_TYPE item){
        COUNT_TYPE ret = INT_MAX;
        for(uint32_t i = 0;i < hash_num;++i){
            uint32_t pos = this->hash(item, i) % length;
            ret = MIN(ret, counters[i][pos]);
        }
        return ret;
    }

    void NewWindow(const COUNT_TYPE window){
        for(uint32_t i = 0;i < hash_num;++i){
            bitsets[i]->Clear();
        }
    }

private:
    const uint32_t hash_num;
    const uint32_t length;

    BitSet** bitsets;
    COUNT_TYPE** counters;
};

#endif //OPE_OPT_H

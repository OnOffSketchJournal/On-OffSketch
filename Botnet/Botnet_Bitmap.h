#ifndef Botnet_Bit_Map_H
#define Botnet_Bit_Map_H

/*
 * On-Off sketch on finding persistent items
 */

#include "bitset.h"
#include "Abstract.h"

template<typename DATA_TYPE, typename COUNT_TYPE>
class Bot_Map : public Abstract<DATA_TYPE, COUNT_TYPE> {
public:

        DATA_TYPE* itemarr;
        BitSet* itemBitsets;

    Bot_Map(uint64_t memory, double Time_Interval){
                window_num = 4050 / Time_Interval;
                if(4050>window_num*Time_Interval)
                {
                    window_num+=1;
                }
                capacity=(double)memory / (sizeof(DATA_TYPE) + BITSIZE*(window_num));
                // printf("OO_FPI slot:%u\n",capacity);
                printf("Window_num :%u\n",window_num);
        cur_element=0;
        itemarr = new DATA_TYPE[capacity];
        memset(itemarr,0,sizeof(DATA_TYPE)*capacity);
        itemBitsets = new BitSet(capacity * window_num);
    }

    ~Bot_Map(){
        delete [] itemarr;
        delete itemBitsets;
    }

    void Insert(const DATA_TYPE item, const COUNT_TYPE window){
        uint32_t pos = this->hash(item) % capacity;
        int cnt=0;
        while(itemarr[pos]!=0)
        {
            if(itemarr[pos]==item)
            {
                itemBitsets->Set(pos*window_num+window-1);
                return;
            }
           ++pos;
           pos%=capacity;
           ++cnt;
           if(cnt>capacity)
           {
               printf("Mem Overflow!\n");
               return;
           }
        }
        itemarr[pos]=item;
        itemBitsets->Set(pos*window_num+window);
        ++cur_element;
        return;
    }

    COUNT_TYPE Query(const DATA_TYPE item){
        uint32_t pos = this->hash(item) % capacity;
        bool add_flag=true;
        int sum=0;
        while(itemarr[pos]!=0)
        {
            if(itemarr[pos]==item)
            {
                for(int i=0;i<window_num;++i)
                {
                    sum+=itemBitsets->Get(pos*window_num+i);
                }
                return sum;
            }
           ++pos; 
        }
        return 0;
    }

    void NewWindow(const COUNT_TYPE window){
    }

private:
    uint32_t capacity;
    int cur_element;
    uint32_t window_num;
};

#endif //OO_FPI_H

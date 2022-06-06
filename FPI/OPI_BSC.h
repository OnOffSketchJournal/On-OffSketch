#ifndef OPI_BSC_H
#define OPI_BSC_H

#include "Abstract.h"
#include "bitset.h"

template<typename DATA_TYPE,typename COUNT_TYPE>
class SSDATA_Node;
template<typename DATA_TYPE,typename COUNT_TYPE>
class SSCOUNT_Node;

template<typename DATA_TYPE,typename COUNT_TYPE>
class SSDATA_Node{
public:
    DATA_TYPE key;
    SSDATA_Node<DATA_TYPE,COUNT_TYPE> * prev;
    SSDATA_Node<DATA_TYPE,COUNT_TYPE> * next;
    SSCOUNT_Node<DATA_TYPE,COUNT_TYPE> * pc;
    SSDATA_Node(){
        key=0;
        prev=NULL;
        next=NULL;
        pc=NULL;
    }
    void Delete(){
        Connect(prev, next);
    }
    void Connect(SSDATA_Node<DATA_TYPE,COUNT_TYPE>* prev, SSDATA_Node<DATA_TYPE,COUNT_TYPE>* next){
        if(prev)
            prev->next = next;
        if(next)
            next->prev = prev;
    }
};

template<typename DATA_TYPE,typename COUNT_TYPE>
class SSCOUNT_Node{
public:
    COUNT_TYPE val;
    SSCOUNT_Node<DATA_TYPE,COUNT_TYPE> * prev;
    SSCOUNT_Node<DATA_TYPE,COUNT_TYPE> * next;
    SSDATA_Node<DATA_TYPE,COUNT_TYPE> * pd;
    int ref;
    SSCOUNT_Node(COUNT_TYPE init_val=0){
        val=init_val;
        prev=NULL;
        next=NULL;
        pd=NULL;
        ref=0;
    }
    void Delete(){
        Connect(prev, next);
    }
    void Connect(SSCOUNT_Node<DATA_TYPE,COUNT_TYPE>* prev, SSCOUNT_Node<DATA_TYPE,COUNT_TYPE>* next){
        if(prev)
            prev->next = next;
        if(next)
            next->prev = prev;
    }
};

template<typename DATA_TYPE, typename COUNT_TYPE>
class OPI_BSC : public Abstract<DATA_TYPE, COUNT_TYPE>{
public:

    OPI_BSC(uint64_t memory){
        now_element = 0;
        // capacity=memory;
        int SLOT_NUM = 8;
        // capacity = ((double)memory / ( sizeof(DATA_TYPE) + sizeof(COUNT_TYPE) + 6*sizeof(void *) + BITSIZE));
        capacity = ((double)memory / ( sizeof(DATA_TYPE) + sizeof(COUNT_TYPE) + BITSIZE));
        printf("OPI_BSC slot:%d\n",capacity);
        sslist = new SSDATA_Node<DATA_TYPE,COUNT_TYPE>[capacity];
        ssmin = new SSCOUNT_Node<DATA_TYPE,COUNT_TYPE>;
        ssBitsets = new BitSet(capacity);
        sslist[0].prev = NULL; 
    }

    ~OPI_BSC(){
        delete sslist;
        delete ssBitsets;
    }

    void Insert(const DATA_TYPE item, const COUNT_TYPE window){
        auto itr = HashTable.find(item);
        if(itr == HashTable.end()){
            append_new_key(item);
            return;
        }
        if(!ssBitsets->SetNGet(itr->second)){
            SSDATA_Node<DATA_TYPE,COUNT_TYPE>* nextdata = sslist[itr->second].next;
            SSDATA_Node<DATA_TYPE,COUNT_TYPE>* prevdata = sslist[itr->second].prev;
            SSCOUNT_Node<DATA_TYPE,COUNT_TYPE>* pcount = sslist[itr->second].pc;
            add_counter(pcount,itr->second);
            prevdata->Connect(prevdata,nextdata);
            if(prevdata == NULL){
                if(nextdata){
                    pcount->pd = nextdata;
                }
                else{
                    pcount->Delete();
                    delete pcount;
                }
            }
        }
    }

    COUNT_TYPE Query(const DATA_TYPE item){
        auto itr = HashTable.find(item);
        if(itr != HashTable.end()){
            return sslist[itr->second].pc->val;
        }
        return 0;
    }

    void NewWindow(const COUNT_TYPE window){
        ssBitsets->Clear();
    }

private:
    uint32_t now_element;
    uint32_t capacity;
    SSDATA_Node<DATA_TYPE,COUNT_TYPE>* sslist;
    SSCOUNT_Node<DATA_TYPE,COUNT_TYPE>* ssmin;
    std::unordered_map<DATA_TYPE,uint32_t> HashTable;
    BitSet* ssBitsets;

    void append_new_key(const DATA_TYPE& item) {
        if (now_element < capacity) {
            uint32_t idx = now_element++;
            sslist[idx].key = item;
            HashTable[item] = idx;
            ssBitsets->Set(idx);
            add_counter(ssmin, idx);
        }
        else {
            SSCOUNT_Node<DATA_TYPE,COUNT_TYPE>* pcount = ssmin->next;
            uint32_t idx = HashTable[pcount->pd->key];
            if(!ssBitsets->SetNGet(idx))
            {
                HashTable.erase(pcount->pd->key);
                sslist[idx].key = item;
                HashTable[item] = idx;
                SSDATA_Node<DATA_TYPE,COUNT_TYPE>* nextdata = sslist[idx].next;
                add_counter(pcount, idx);
                if(nextdata){
                    pcount->pd = nextdata;
                    nextdata->prev = NULL;
                }
                else{
                    pcount->Delete();
                    delete pcount;
                }
            }
        }
    }

    void add_counter(SSCOUNT_Node<DATA_TYPE,COUNT_TYPE>* pcount,uint32_t idx) {

        if(!pcount->next){
            pcount->Connect(pcount, new SSCOUNT_Node<DATA_TYPE,COUNT_TYPE>(pcount->val+1));
        }
        else if((pcount->next->val)-(pcount->val)>1){
            SSCOUNT_Node<DATA_TYPE,COUNT_TYPE>* padd = new SSCOUNT_Node<DATA_TYPE,COUNT_TYPE>(pcount->val+1);
            pcount->Connect(padd, pcount->next);
            pcount->Connect(pcount, padd);
        }
        SSDATA_Node<DATA_TYPE,COUNT_TYPE>* my = sslist + idx;
        pcount->next->ref++;
        pcount->ref--;
        my->prev = NULL;
        my->pc = pcount->next;
        my->Connect(my,my->pc->pd);
        my->pc->pd=my;
    }
};

#endif
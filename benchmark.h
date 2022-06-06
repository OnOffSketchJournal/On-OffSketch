#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <sys/stat.h>

#include <chrono>

#include "SS.h"
#include "PIE.h"
#include "OPI_OPT.h"
#include "OPI_BSC.h"
#include "OO_SIMD.h"
#include "OO_SIMD_8.h"
#include "OO_SIMD_32.h"

#include "CM_HT.h"
#include "CM_BF.h"
#include "OPE_BSC.h"
#include "OPE_OPT.h"


template<typename DATA_TYPE,typename COUNT_TYPE>
class BenchMark{
public:

    typedef std::vector<Abstract<DATA_TYPE, COUNT_TYPE>*> AbsVector;
    typedef std::unordered_map<DATA_TYPE, COUNT_TYPE> HashMap;

    BenchMark(const char* _PATH, const COUNT_TYPE _T):
            PATH(_PATH){
	    struct stat statbuf;
	    stat(PATH, &statbuf);
	    LENGTH = ceil(statbuf.st_size / (double)(_T * sizeof(DATA_TYPE)));

        FILE* file = fopen(PATH, "rb");

        COUNT_TYPE number = 0;
        DATA_TYPE item;
        HashMap record;

        TOTAL = 0;
        T = 0;

        while(fread(&item, sizeof(DATA_TYPE), 1, file) > 0){
            if(number % LENGTH == 0)
                T += 1;
            number += 1;
            if(record[item] != T){
                record[item] = T;
                mp[item] += 1;
                TOTAL += 1;
            }
        }

        distinct_item_num=record.size();
        printf("read from %s,DATA STREAM NUMBER %d\n",PATH,number);
        printf("DISTINCT ITEM NUMBER %ld\n",record.size());

        fclose(file);
    }

    void SketchError(uint32_t section){
        AbsVector PEs = {
                new CM_BF<DATA_TYPE, COUNT_TYPE>(2, 1000000 / 2.0 / (1 + sizeof(COUNT_TYPE))),
                new OPE_BSC<DATA_TYPE, COUNT_TYPE>(2, 1000000 / 2.0 / (BITSIZE + sizeof(COUNT_TYPE))),//mem=1MB
                new OPE_OPT<DATA_TYPE, COUNT_TYPE>(2, 1000000 / 2.0 / (BITSIZE + sizeof(COUNT_TYPE))),//mem=1MB
        };

        BenchInsert(PEs);
        // IdealInsert(PEs);

        for(auto PE : PEs){
            // printf("Check distribution\n");
            CheckDistribution(PE, section);
            // printf("PE Check\n");
            PECheckError(PE);
            delete PE;
        }
    }

    void TopKError(double alpha){
        AbsVector FPIs = {
                new OPI_OPT<DATA_TYPE, COUNT_TYPE, 8>(200000),
                new OPI_BSC<DATA_TYPE, COUNT_TYPE>(200000),
                
	    };
        BenchInsert(FPIs);
        for(auto FPI : FPIs){
            FPICheckError(FPI, alpha * TOTAL);
            delete FPI;
        }
    }

    void Thp(){
        for(uint32_t i = 0;i < 5;++i){
	        std::cout << i << std::endl;

	        AbsVector FPIs = {
                    // THP W=2 W=8 W=32 SIMD-8 SIMD-32
                    new OPI_OPT<DATA_TYPE, COUNT_TYPE, 2>(200000),
                    new OPI_OPT<DATA_TYPE, COUNT_TYPE, 8>(200000),
                    new OPI_OPT<DATA_TYPE, COUNT_TYPE, 32>(200000),
                    new OPI_BSC<DATA_TYPE, COUNT_TYPE>(200000),
                    // new OO_SIMD_8_32_32<DATA_TYPE, COUNT_TYPE, 8>(200000),
                    // new OO_SIMD_32_32_32<DATA_TYPE, COUNT_TYPE, 32>(200000),
                    // new OO_SIMD_8_64_32<DATA_TYPE, COUNT_TYPE, 8>(200000),            
                    // new OO_SIMD_32_64_32<DATA_TYPE, COUNT_TYPE, 8>(200000),                    

		    };

            for(auto FPI : FPIs){
                InsertThp(FPI);
                delete FPI;
            }
        }
    }

private:

    double TOTAL;
    COUNT_TYPE T;
    COUNT_TYPE LENGTH;
    COUNT_TYPE distinct_item_num;

    HashMap mp;
    const char* PATH;

    typedef std::chrono::high_resolution_clock::time_point TP;

    inline TP now(){
        return std::chrono::high_resolution_clock::now();
    }

    inline double durationms(TP finish, TP start){
        return std::chrono::duration_cast<std::chrono::duration<double,std::ratio<1,1000000>>>(finish - start).count();
    }

    unsigned int transIP(char* ipv4)
    {
        char* token=strtok(ipv4,".");
        unsigned int ip=0;
        int cur;
        while(token!=NULL)
        {
            int cur=atoi(token);
            if(cur>=0&&cur<=255)
            {
                ip<<=8;
                ip+=cur;
            }
            token=strtok(NULL,".");
        }
        return ip;
    }
    
    void BenchInsert(AbsVector sketches){
        FILE* file = fopen(PATH, "rb");
        DATA_TYPE item;
        COUNT_TYPE number = 0, windowId = 0;

        while(fread(&item, sizeof(DATA_TYPE), 1, file) > 0){
            if(number % LENGTH == 0){
                windowId += 1;
                for(auto sketch : sketches)
                    sketch->NewWindow(windowId);
            }
            number += 1;
            for(auto sketch : sketches)
            {
                sketch->Insert(item, windowId);
            }
        }
        fclose(file);
    }

    void IdealInsert(AbsVector sketches){

        for(auto it = mp.begin();it != mp.end();++it){
            sketches[0]->ideal_set(it->first,it->second);
        }
    }

    void InsertThp(Abstract<DATA_TYPE, COUNT_TYPE>* sketch){
        TP start, finish;

        FILE* file = fopen(PATH, "rb");
        DATA_TYPE item;
        COUNT_TYPE number = 0, windowId = 0;
        HashMap record;

        start = now();
        while(fread(&item, sizeof(DATA_TYPE), 1, file) > 0){
            if(number % LENGTH == 0){
                windowId += 1;
                sketch->NewWindow(windowId);
            }
            number += 1;

            sketch->Insert(item, windowId);
        }
        finish = now();

        fclose(file);
	std::cout << "Thp: " << number / durationms(finish, start) << std::endl; 
    }

    void FPICheckError(Abstract<DATA_TYPE, COUNT_TYPE>* sketch, COUNT_TYPE HIT){
        double real = 0, estimate = 0, both = 0;
        double aae = 0, cr = 0, pr = 0, f1 = 0;
        double fnr = 0, fpr = 0;
        int rcd[100] = {0};
        int delta;
        int sum = 0;
        for(auto it = mp.begin();it != mp.end();++it){
            COUNT_TYPE value = sketch->Query(it->first);

            if(value > HIT){
                estimate += 1;
                if(it->second > HIT) {
                    both += 1;
                    delta = abs(it->second - value);
                    aae += delta;
                    if(delta<100)
                    {
                        ++rcd[delta];
                    }
                }
                else
                {
                    fpr += 1;
                }
            }
            if(it->second > HIT)
            {
                real += 1;
                if(value < HIT)
                {
                    fnr += 1;
                }
            }
        }

        if(both <= 0){
            std::cout << "Not Find Real Persistent" << std::endl;
        }
        else{
            aae /= both;
        }
        if(real!=0)
        {
            cr = both / real;

            fnr /= real;
            fpr /= (distinct_item_num - real);
        }

        if(estimate <= 0){
            std::cout << "Not Find Persistent" << std::endl;
        }
        else{
            pr = both / estimate;
        }

        if(cr == 0 && pr == 0)
            f1 = 0;
        else
            f1 = (2*cr*pr)/(cr+pr);

        // for(int i=0;i<100;++i)
        // {
        //     sum+=rcd[i];
        //     printf("%lf,",sum/both);
        // }
        // printf("\n");
	    // std::cout << HIT << std::endl;
		std::cout << "AAE: " << aae << ", ";
        std::cout << "FNR: " << fnr << ", ";
        std::cout << "FPR: " << fpr << std::endl;
		// std::cout << "F1: " << f1 << std::endl;
    }

    void PECheckError(Abstract<DATA_TYPE, COUNT_TYPE>* sketch){
        double aae = 0;
        int delta;
        int cnt=0;
        int rcd[65]={0};
        int sum=0;
        for(auto it = mp.begin();it != mp.end();++it){
            COUNT_TYPE value = sketch->Query(it->first);
            delta = abs(it->second - value);
            aae += delta;
            if(delta<65)
            {
                ++rcd[delta];
            }
            ++cnt;
        }
	    std::cout << "AAE: " << aae / mp.size() << std::endl;
        // printf("COUNT: %d\nsum:\n",cnt);
        // for(int i=0;i<=64;++i)
        // {
        //     sum+=rcd[i];
        //     printf("%d,",sum);
        // }
        // printf("\n");
    }

    void CheckDistribution(Abstract<DATA_TYPE, COUNT_TYPE>* sketch, const uint32_t section){
        uint32_t* aae = new uint32_t[section];
        uint32_t* number = new uint32_t[section];

        memset(aae, 0, sizeof(uint32_t) * section);
        memset(number, 0, sizeof(uint32_t) * section);

        printf("Check Distribution T:%d\n",T);

        for(auto it = mp.begin();it != mp.end();++it){
            COUNT_TYPE value = sketch->Query(it->first);
            uint32_t pos = (it->second * section - 1) / T;

            aae[pos] += abs(it->second - value);
            number[pos] += 1;
        }

        for(uint32_t i = 0;i < section;++i){
             if(number[i] != 0)
                std::cout << aae[i] / (double)number[i] << ",";
            else
                std::cout << "NULL,";
        }
        std::cout << std::endl;

        delete [] aae;
        delete [] number;
    }
};

#endif //BENCHMARK_H

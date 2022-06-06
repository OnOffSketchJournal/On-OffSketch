#ifndef BOTNET_H
#define BOTNET_H

#include <sys/stat.h>
#include <iostream>

#include <chrono>

#include "SS.h"
#include "PIE.h"
#include "OPI_OPT.h"
#include "OPI_BSC.h"
#include "OO_SIMD.h"
#include "Botnet_Bitmap.h"

#include "CM_HT.h"
#include "CM_BF.h"
#include "OPE_BSC.h"
#include "OPE_OPT.h"


template<typename DATA_TYPE,typename COUNT_TYPE>
class Botnet_Detect{
public:

    typedef std::vector<Abstract<DATA_TYPE, COUNT_TYPE>*> AbsVector;
    typedef std::unordered_map<DATA_TYPE, COUNT_TYPE> HashMap;

    Botnet_Detect(const char* _PATH, const double _T, float base_time):
            PATH(_PATH){
	    Time_Window = _T;

        FILE* file = fopen(PATH, "rb");

        COUNT_TYPE number = 0;
        DATA_TYPE item;
        HashMap record;

        TOTAL = 0;

        COUNT_TYPE windowId=0;
        bool window_flag=true;
        float time_stamp;
        time_stamp0=base_time;
        uint32_t label;
        char str_dstIP[100];
        char str_srcIP[100];
        uint32_t srcIP,dstIP;
        int window_item_num=0;

        rcd_timestamp=new float[16000000];
        rcd_item=new DATA_TYPE[16000000];

        while(fscanf(file,"%f %s %s %u\n",&time_stamp,str_srcIP,str_dstIP,&label)!=EOF){
            if((time_stamp-Time_Window*windowId-time_stamp0)>=0)
            {
                window_flag=true;
            }
            if(window_flag)
            {
                printf("Window:%d\t item:%d\n",windowId,window_item_num);
                windowId += 1;
                window_item_num=0;
                window_flag=false;
            }
            item=transIP(str_dstIP);
            rcd_timestamp[number]=time_stamp;
            rcd_item[number]=item;
            number += 1;
            window_item_num+=1;
            if(record[item] != windowId){
                record[item] = windowId;
                mp[item] += 1;
                TOTAL += 1;
            }
        }
        printf("Window:%d\t item:%d\n",windowId,window_item_num);
        Window_num=windowId;
        item_num=number;
        printf("read from %s,DATA STREAM NUMBER %d\n",PATH,item_num);
        printf("DISTINCT ITEM NUMBER %ld\n",record.size());

        fclose(file);
    }

    void TopKError(){
        AbsVector FPIs = {
                new OPI_OPT<DATA_TYPE, COUNT_TYPE, 8>(80000),
                // new Bot_Map<DATA_TYPE, COUNT_TYPE>(4100000,10),
	    };
        BenchInsert(FPIs);
        for(auto FPI : FPIs){
            FPICheckError(FPI);
            delete FPI;
        }
    }

    void Thp(){
        for(uint32_t i = 0;i < 5;++i){
	        // std::cout << i << std::endl;

	        AbsVector FPIs = {
                new OPI_OPT<DATA_TYPE, COUNT_TYPE, 8>(80000),
                new Bot_Map<DATA_TYPE, COUNT_TYPE>(4100000,10)
		    };

            for(auto FPI : FPIs){
                InsertThp(FPI);
                delete FPI;
            }
        }
    }

private:

    double TOTAL;
    double Time_Window;
    uint32_t Window_num;
    float time_stamp0;
    COUNT_TYPE item_num;

    float* rcd_timestamp;
    DATA_TYPE* rcd_item;

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
        uint32_t item;
        COUNT_TYPE windowId = 0;
        bool window_flag=true;
        float time_stamp;
        int label;
        char str_dstIP[100];
        char str_srcIP[100];
        uint32_t srcIP,dstIP;
        COUNT_TYPE cur_num=0;
        while(cur_num<item_num){
            if((rcd_timestamp[cur_num]-Time_Window*windowId-time_stamp0)>=0)
            {
                window_flag=true;
            }
            if(window_flag){
                windowId += 1;
                // printf("Window:%d\n",windowId);
                window_flag=false;
                for(auto sketch : sketches)
                    sketch->NewWindow(windowId);
            }
            for(auto sketch : sketches)
            {
                sketch->Insert(rcd_item[cur_num], windowId);
            }
            ++cur_num;
        }
        fclose(file);
    }

    void InsertThp(Abstract<DATA_TYPE, COUNT_TYPE>* sketch){
        TP start, finish;

        FILE* file = fopen(PATH, "rb");
        uint32_t item;
        COUNT_TYPE windowId = 0;
        bool window_flag=true;
        float time_stamp;
        int label;
        char str_dstIP[100];
        char str_srcIP[100];
        uint32_t srcIP,dstIP;
        COUNT_TYPE cur_num=0;
        uint64_t rdtsc_start, rdtsc_end;
        uint64_t cpucycle;
        uint64_t max_cycle = 0;
        uint32_t arrcpucycle[2000]={0};

        // start = now();
        while(cur_num<item_num){
            if((rcd_timestamp[cur_num]-Time_Window*windowId-time_stamp0)>=0)
            {
                window_flag=true;
            }
            if(window_flag){
                windowId += 1;
                // printf("Window:%d\n",windowId);
                window_flag=false;
                sketch->NewWindow(windowId-1);
            }
            rdtsc_start = rdtsc();
            sketch->Insert(rcd_item[cur_num], windowId-1);
            rdtsc_end = rdtsc();
            ++cur_num;
            cpucycle=rdtsc_end-rdtsc_start;
            if(cpucycle>max_cycle)
            {
                max_cycle=cpucycle;
            }
            if(cpucycle<2000)
            {
                ++arrcpucycle[cpucycle];
            }
        }
        // finish = now();
        fclose(file);
        std::cout << max_cycle << std::endl;
        int sum=0;
        for(int i=0;i<500;++i)
        {
            sum+=arrcpucycle[i];
            printf("%lf,",(double)sum/item_num);
        }
	    // std::cout << item_num / durationms(finish, start) << std::endl; 
    }

    uint64_t rdtsc()
    {
        uint32_t lo,hi;
        __asm__ volatile ("rdtsc" : "=a" (lo), "=d" (hi));
        return ((uint64_t)hi<<32) | lo;
    }


    bool compare(DATA_TYPE  item)
    {
        std::string str[10]={
            "147.32.84.165",
            "147.32.84.191",
            "147.32.84.192",
            "147.32.84.193",
            "147.32.84.204",
            "147.32.84.205",
            "147.32.84.206",
            "147.32.84.207",
            "147.32.84.208",
            "147.32.84.209",
        };
        for(int i=0;i<10;++i)
        {
            if(item==transIP((char*)str[i].c_str()))
                return true;
        }
        return false;
    }

    void FPICheckError(Abstract<DATA_TYPE, COUNT_TYPE>* sketch){
        double real = 0, estimate = 0, both = 0;
        double aae = 0, cr = 0, pr = 0, f1 = 0;
        double find_persistent[2000]={0};
        double recall[2000]={0};
        double top2k[2000]={0};
        double top2kcnt=0;

        for(auto it = mp.begin();it != mp.end();++it){
            COUNT_TYPE value = sketch->Query(it->first);
            for(int i=0;i<=Window_num;++i)
            {
                if(value >=i){
                    find_persistent[i] += 1;
                    if(compare(it->first)){
                        recall[i]+=1;
                    }
                    if(it->second>13){
                        top2k[i]+=1;
                    }
                }
            }
            if(it->second>13)
            {
                ++top2kcnt;
            }
        }
        // printf("\tWindow num %d",Window_num);
        // printf("\tpersistence item number\t recall\n");
        // for(int i=0;i<=Window_num;++i)
        // {
        //     printf("%d,%.0lf,%.0lf\n",i,find_persistent[i],recall[i]);
        // }
        // printf("\tthreshold,FNR,FPR\n");
        for(int i=1;i<=1;++i)
        {
        // for(int i=0;i<=Window_num;++i)
        // {
            printf("%lf,%lf,%lf,%lf,%lf\n",(double)i/Window_num,(10-recall[i])/10,top2kcnt,(top2kcnt-top2k[i])/top2kcnt,(find_persistent[i]-recall[i])/(find_persistent[0]-10));
        }
    }

};

#endif //BENCHMARK_H

#include "benchmark.h"
#include "Botnet_Detection.h"

int main(int argc, char *argv[]) {
    const char* datasrc[5];
    datasrc[0]="/root/onoff_sketch/dataset/015.dat";//Synthetic
    datasrc[1]="/root/onoff_sketch/dataset/dc.dat";//DataCenter
    datasrc[2]="/root/onoff_sketch/dataset/net.dat";//Network
    datasrc[3]="/root/onoff_sketch/dataset/ip16-1.dat";//IP_CAIDA
    datasrc[4]="/root/onoff_sketch/20110818_packet_label_trunc";//IP_CAIDA

    //code for PE

    // for(uint32_t i = 0;i < 3;++i){
	//     std::cout << datasrc[i] << std::endl;
    //     BenchMark<uint32_t, int32_t> dataset(datasrc[i], 1600);
    //     dataset.SketchError(5);
    //     // dataset.Thp();
    // }
    // std::cout << datasrc[3] << std::endl;
    // BenchMark<uint64_t, int32_t> dataset(datasrc[3], 1600);// CAIDA 8byte srcIP+dstIP
    // dataset.SketchError(5);
    // dataset.Thp();

    //code for FPI

    // for(uint32_t i = 0;i < 3;++i){
	//     std::cout << datasrc[i] << std::endl;
    //     BenchMark<uint32_t, int32_t> dataset(datasrc[i], 1600);
    //     dataset.TopKError(0.00005);
    //     dataset.Thp();
    // }
    // std::cout << datasrc[3] << std::endl;
    // BenchMark<uint64_t, int32_t> dataset(datasrc[3], 1600);// CAIDA 8byte srcIP+dstIP
    // dataset.TopKError(0.00005);
    // dataset.Thp();

    // Code for Botnet

    // std::cout << datasrc[4] << std::endl;
    // Botnet_Detect<uint32_t, int32_t> botnetdataset(datasrc[4], 10, 3000);
    // botnetdataset.TopKError();
    // botnetdataset.Thp();

    // return 0;
}

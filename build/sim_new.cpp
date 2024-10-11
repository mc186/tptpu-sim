#include "common.hpp"
#include <unistd.h> // for getopt()

int main(int argc, char *argv[]) {
    int opt, channels, ranks, X, Y, Z;
    std::string dram_name, dimension_layout;
    bool is_nchw;
    // set default values:
    channels = 2;
    ranks = 2;
    X = 640;
    Y = 640;
    Z = 1080;
    dram_name = "HBM";  //DDR4_3200
    dimension_layout = "nchw";
    // get values given via CLI
    while((opt = getopt(argc, argv, "c:d:l:r:x:y:z:")) != -1) {
        switch(opt) {
            case 'c':
                channels = atoi(optarg);
                break;
            case 'd':
                dram_name = optarg;
                break;
            case 'l':
                dimension_layout = optarg;
                break;
            case 'r':
                ranks = atoi(optarg);
                break;
            case 'x':
                X = atoi(optarg);
                break;
            case 'y':
                Y = atoi(optarg);
                break;
            case 'z':
                Z = atoi(optarg);
                break;
            default:
                // shouldn't reach here
                std::cerr << "Something wrong with command line arguments..." << std::endl;
                return -1;
        }
    }
    // check for dimension layout
    if (dimension_layout == "nchw")
        is_nchw = true;
    else if (dimension_layout == "nhwc")
        is_nchw = false;
    else {
        std::cerr << "Something wrong with dimension layout: only nchw and nhwc allowed" << std::endl;
        return -1;
    }
    // 24MiB buffer  ->32/4 = 8MiB
    float buffer_size = (float)(1 * (1 << 23)); //3
    float clock         = 0.7 ; //1 //0.94//GHz                            // 700MHz  //940MHz
    // the two bw should be changed later to be more flexible and add up to 300GB/s  //225 maybe
    float bw_dram_wf = 300; // 267GB/s //300
    float bw_dram_ub = 300;  //225 33GB/s //300
    int sa_width = 128;  //vector mode: 1
    int sa_height = 128; //vector mode: 128
    int accumulator_size = 1024; //4096; //1024 per mmu //8x sa size
    int vector_units = 128;  //replacing sa_width
    // int vector_size = 1024; //1024/4096 per mmu
    float bw_ub_mmu = 250;  // 256 meaningless
    float bw_wf_mmu = 128;  // 100 meaningless - not used
    float bw_ub_vu = 256;  // 256 meaningless
    float bw_wf_vu = 256;  // 256 meaningless
    UnifiedBuffer *ub = new UnifiedBuffer(buffer_size);
    DRAM *dram = new DRAM(dram_name, clock, channels, ranks);
    WeightFetcher *wf = new WeightFetcher(sa_width * sa_height, 4, sa_height); //depth = 4 //meaningless //"weight buffer"

    //should it be dram_ub or interconnect_router_ub?
    Interconnect *dram_ub_icnt = new Interconnect((Unit *)dram, (Unit *)ub, clock, bw_dram_ub, ub->GetCapacity(),
                                                  dram->IsMainMemory(), dram->GetUBSenderQueue(),
                                                  ub->GetServedQueue(), ub->GetWaitingQueue(), ub->GetRequestQueue());
    Interconnect *dram_wf_icnt = new Interconnect((Unit *)dram, (Unit *)wf, clock, bw_dram_wf, wf->GetCapacity(),
                                                  dram->IsMainMemory(), dram->GetWFSenderQueue(),
                                                  wf->GetServedQueue(), wf->GetWaitingQueue(), wf->GetRequestQueue());
    MatrixMultiplyUnit *mmu = new MatrixMultiplyUnit(sa_width, sa_height, accumulator_size, ub, wf);
    VectorUnit *vu = new VectorUnit(vector_units, accumulator_size, ub);  //new vector unit to perform activations and other elementwise operations
    Interconnect *ub_mmu_icnt = new Interconnect((Unit *)ub, (Unit *)mmu, clock, bw_ub_mmu, mmu->GetCapacity(),
                                                 ub->IsMainMemory(), ub->GetSenderQueue(),
                                                 mmu->GetUBServedQueue(), mmu->GetUBWaitingQueue(), mmu->GetUBRequestQueue());
    Interconnect *wf_mmu_icnt = new Interconnect((Unit *)wf, (Unit *)mmu, clock, bw_wf_mmu, mmu->GetCapacity(),
                                                 wf->IsMainMemory(), wf->GetSenderQueue(),
                                                 mmu->GetWFServedQueue(), mmu->GetWFWaitingQueue(), mmu->GetWFRequestQueue());
    Interconnect *ub_vu_icnt = new Interconnect((Unit *)ub, (Unit *)vu, clock, bw_ub_vu, vu->GetCapacity(),
                                                 ub->IsMainMemory(), ub->GetSenderQueue(),
                                                 vu->GetUBServedQueue(), vu->GetUBWaitingQueue(), vu->GetUBRequestQueue());

    std::vector<Interconnect *> *icnt_list = new std::vector<Interconnect *>();
    icnt_list->push_back(dram_ub_icnt); 
    // icnt_list->push_back(dram_wf_icnt); 
    // icnt_list->push_back(ub_mmu_icnt);
    // icnt_list->push_back(wf_mmu_icnt);
    icnt_list->push_back(ub_vu_icnt); 
    Controller *ctrl = new Controller(mmu, vu, icnt_list, dram->GetWeightTileQueue(), dram->GetActivationTileQueue()); //vu
    // setting complete
    // generate request for matrix multiplication
    unsigned int weight_starting_address        = 0xcc000000;
    unsigned int activation_starting_address    = 0xe0000000;
    //ctrl->MatrixMultiply(X, Y, Z, is_nchw, 3, weight_starting_address, activation_starting_address);
    ctrl->VectorOp(X, Y, activation_starting_address);

    // std::cout << "Number of requests in VU queue: " << vu->GetRequestQueue()->size() << std::endl;
    // std::cout << "DRAM issued " << dram->GetRequestQueue()->size() << " requests." << std::endl;


    //add if-else?
    
    // while (!(dram_ub_icnt->IsIdle() && dram_wf_icnt->IsIdle() && ub_mmu_icnt->IsIdle() && wf_mmu_icnt->IsIdle() && mmu->IsIdle())) {
    //     mmu->Cycle();
    //     ub_mmu_icnt->Cycle();
    //     wf_mmu_icnt->Cycle();
    //     ub->Cycle();
    //     wf->Cycle();
    //     dram_ub_icnt->Cycle();
    //     dram_wf_icnt->Cycle();
    //     dram->Cycle();
    // }

    while (!(dram_ub_icnt->IsIdle() && ub_vu_icnt->IsIdle() && vu->IsIdle())) {
        vu->Cycle();
        ub_vu_icnt->Cycle();
        ub->Cycle();
        wf->Cycle();
        dram_ub_icnt->Cycle();
        dram->Cycle();
    }

    // test complete
    dram_ub_icnt->PrintStats("DRAM - Unified Buffer Interconnect"); 
    dram_wf_icnt->PrintStats("DRAM - Weight Fetcher Interconnect");
    ub_mmu_icnt->PrintStats("MMU - Unified Buffer Interconnect");
    wf_mmu_icnt->PrintStats("MMU - Weight Fetcher Interconnect");
    ub_vu_icnt->PrintStats("VU - Unified Buffer Interconnect"); 
    dram->PrintStats();  
    // mmu->PrintStats();
    vu->PrintStats();  
    return 0;
}

#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <assert.h>

#include "buffer.hpp"
#include "common.hpp"
#include "unit.hpp"
#include "weightfetcher.hpp"

void pop_front(std::vector<request> &v);
void pop_front(std::vector<float> &v);
void find_and_delete_by_order(std::vector<request> &v, int order);

class MatrixMultiplyUnit: public Unit {
public:
    MatrixMultiplyUnit(int sa_width, int sa_height, int acc_size, UnifiedBuffer *unifiedbuffer, WeightFetcher *weightfetcher);

    void UpdateTilingQueue();
    bool IsIdle();
    void Cycle();
    void PrintStats();

    bool IsMainMemory() {return is_main_memory;}
    bool IsMatrixMultiplyUnit() {return true;}
    // Matrix Multiply Unit should not call these methods
    std::vector<request> *GetSenderQueue()  {assert(0); return new std::vector<request>();}
    std::vector<request> *GetServedQueue()  {assert(0); return new std::vector<request>();}
    std::vector<request> *GetWaitingQueue() {assert(0); return new std::vector<request>();}
    std::vector<request> *GetRequestQueue() {assert(0); return new std::vector<request>();}
    // connected to Weight Fetcher
    std::vector<request> *GetWFServedQueue()    {return wf_served_queue;}
    std::vector<request> *GetWFWaitingQueue()   {return wf_waiting_queue;}
    std::vector<request> *GetWFRequestQueue()   {return wf_request_queue;}
    // connected to Unified Buffer
    std::vector<request> *GetUBServedQueue()    {return ub_served_queue;}
    std::vector<request> *GetUBWaitingQueue()   {return ub_waiting_queue;}
    std::vector<request> *GetUBRequestQueue()   {return ub_request_queue;}

    float GetCapacity() {return capacity;}
private:
    float capacity;                 // systolic_array_width x systolic_array_height
    int systolic_array_width;       // how many values come each cycle from UnifiedBuffer
    int systolic_array_height;      // how far deep the values can go
    bool is_main_memory;

    int idle_cycle;                 // number of cycles Matrix Multiply Unit performed no computations
    int busy_cycle;                 // number of cycles Matrix Multiply Unit performed computations
    int wait_cycle;                 // number of cycles Matrix Multiply Unit needs to wait (is computing)
    float total_computation_number; // total number of computations Matrix Multiply Unit performed
    int current_order;              // the order being computer right now, 0 if none

    int accumulator_size;           // size of accumulator, probably 2048
    

    UnifiedBuffer *ub;              // pointer to Unified Buffer connected to this unit
    WeightFetcher *wf;              // pointer to Weight Fetcher connected to this unit

    // connected to Weight Fetcher
    std::vector<request> *wf_served_queue;
    std::vector<request> *wf_waiting_queue;
    std::vector<request> *wf_request_queue;
    // connected to Unified Buffer
    std::vector<request> *ub_served_queue;
    std::vector<request> *ub_waiting_queue;
    std::vector<request> *ub_request_queue;
    // queue to keep track of computations
    std::vector<request> *tiling_queue;
};
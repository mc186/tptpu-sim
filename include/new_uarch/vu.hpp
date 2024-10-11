#ifndef VU_H
#define VU_H

#include "common.hpp"
#include "unit.hpp"

struct request;
typedef struct request request;
class UnifiedBuffer;
class Unit;

class VectorUnit: public Unit {
public:
    VectorUnit(int vector_width, int acc_size, UnifiedBuffer *unifiedbuffer);
    ~VectorUnit();

    void UpdateTilingQueue();
    bool IsIdle();
    void Cycle();
    void PrintStats();

    bool IsVectorUnit() {return true;}
    
    float GetCapacity() {return capacity;}
    int GetVectorWidth() {return vector_width;}
    int GetAccumulatorSize() {return accumulator_size;}
    bool IsMainMemory() {return is_main_memory;}

    // Vector Unit should not call these methods
    std::vector<request> *GetSenderQueue()  {assert(0); return new std::vector<request>();}
    std::vector<request> *GetServedQueue()  {assert(0); return new std::vector<request>();}
    std::vector<request> *GetWaitingQueue() {assert(0); return new std::vector<request>();}
    std::vector<request> *GetRequestQueue() {assert(0); return new std::vector<request>();}
    // connected to Unified Buffer
    std::vector<request> *GetUBSenderQueue()    {return ub_sender_queue;}
    std::vector<request> *GetUBServedQueue()    {return ub_served_queue;}
    std::vector<request> *GetUBWaitingQueue()   {return ub_waiting_queue;}
    std::vector<request> *GetUBRequestQueue()   {return ub_request_queue;}
    // queue to keep track of computations
    std::vector<request> *GetTilingQueue()      { return tiling_queue;}

private:
    float capacity;                 // vector_width
    int vector_width;       // how many values come each cycle from UnifiedBuffer
    int accumulator_size;           // size of accumulator, probably 2048
    bool is_main_memory;

    int idle_cycle;                 // number of cycles Matrix Multiply Unit performed no computations
    int busy_cycle;                 // number of cycles Matrix Multiply Unit performed computations
    int wait_cycle;                 // number of cycles Matrix Multiply Unit needs to wait (is computing)
    float total_computation_number; // total number of computations Matrix Multiply Unit performed
    int current_order;              // the order being computer right now, 0 if none

    UnifiedBuffer *ub;              // pointer to Unified Buffer connected to this unit

    // connected to Unified Buffer
    std::vector<request> *ub_sender_queue;
    std::vector<request> *ub_served_queue;
    std::vector<request> *ub_waiting_queue;
    std::vector<request> *ub_request_queue;
    // queue to keep track of computations
    std::vector<request> *tiling_queue;
};

#endif /* VU_H */

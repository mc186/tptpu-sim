#include "vu.hpp"

VectorUnit::VectorUnit(int vector_units, int acc_size, UnifiedBuffer *unifiedbuffer) {
    capacity = (float)vector_units*acc_size;
    vector_width = vector_units;
    accumulator_size = acc_size;
    is_main_memory = false;
    
    // Optional: Add zero check here to prevent future issues
    if (this->vector_width == 0) {
        std::cerr << "Error: vector_width cannot be zero." << std::endl;
        exit(EXIT_FAILURE);
    }

    idle_cycle = 0;
    busy_cycle = 0;
    wait_cycle = 0;
    total_computation_number = (float)0;
    current_order = 0;

    ub = unifiedbuffer;

    ub_sender_queue = unifiedbuffer->GetSenderQueue();
    ub_served_queue = new std::vector<request>();
    ub_waiting_queue = new std::vector<request>();
    ub_request_queue = new std::vector<request>();
    
    tiling_queue = new std::vector<request>();
}

VectorUnit::~VectorUnit() {
    delete ub_served_queue;
    delete ub_waiting_queue;
    delete ub_request_queue;
    delete tiling_queue;
}

/* Updates tiling_queue so that all requests in ub_sender_queue share
 * the same 'order' value are deleted from their respective queues and pushed into tiling_queue */
void VectorUnit::UpdateTilingQueue() {
    // update tiling_queue
    std::vector<request>::iterator ubit;
    std::vector<request>::iterator begin;

    std::cout << "UB sender queue size: " << ub_sender_queue->size() << std::endl;
    while (!ub_request_queue->empty()) {
        ub_sender_queue->push_back(ub_request_queue->front());
        ub_request_queue->erase(ub_request_queue->begin());
        std::cout << "Transferred request from UB request queue to UB sender queue." << std::endl;
    }
    
}

bool VectorUnit::IsIdle() {
    UpdateTilingQueue();
    return (tiling_queue->empty() && (wait_cycle == 0));
}

void VectorUnit::Cycle() {
    // update tiling queue and sender_queues
    UpdateTilingQueue();
    // check for cycles
    if (wait_cycle == 0) {
        if (tiling_queue->empty()) {
            // not computing, not ready to compute -> idle
            idle_cycle++;
            return;
        }
        // not computing, but ready to compute-> not idle
        wait_cycle = 10; //accumulator_size + systolic_array_width + systolic_array_height - 1; //some fixed latency for vector ops/ activations
        current_order = tiling_queue->front().order;
    }
    // 'compute'
    wait_cycle--;
    busy_cycle++;;
    // check if computation is complete
    if (wait_cycle == 0) {
        // increase total number of computation
        total_computation_number += (float)(vector_width);
        // delete from tiling_queue
        find_and_delete_by_order(*tiling_queue, current_order);
        // delete from Vector Unit's waiting queues
        find_and_delete_by_order(*ub_waiting_queue, current_order);
        // delete from Unified Buffer or Weight Fetcher's sender queues
        find_and_delete_by_order(*(ub->GetSenderQueue()), current_order);
    }
}

void VectorUnit::PrintStats() {
    std::cout << "======================================================================" << std::endl;
    std::cout << "\t\tVector Unit Statistics:" << std::endl;
    std::cout << "\tidle cycles: " << idle_cycle << ",\t\tbusy cycles: " << busy_cycle << std::endl;
    std::cout << "total number of computations performed by this VU:\t" << total_computation_number << std::endl;
    std::cout << "======================================================================" << std::endl;
}


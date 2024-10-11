#include "mmu.hpp"

MatrixMultiplyUnit::MatrixMultiplyUnit(int sa_width, int sa_height, int acc_size, UnifiedBuffer *unifiedbuffer, WeightFetcher *weightfetcher) {
    capacity = (float)sa_width * (float)sa_height;
    systolic_array_width = sa_width;
    systolic_array_height = sa_height;
    accumulator_size = acc_size;
    is_main_memory = false;

    idle_cycle = 0;
    busy_cycle = 0;
    wait_cycle = 0;
    total_computation_number = (float)0;
    current_order = 0;

    ub = unifiedbuffer;
    wf = weightfetcher;

    wf_sender_queue = weightfetcher->GetSenderQueue();
    wf_served_queue = new std::vector<request>();
    wf_waiting_queue = new std::vector<request>();
    wf_request_queue = new std::vector<request>();
    ub_sender_queue = unifiedbuffer->GetSenderQueue();
    ub_served_queue = new std::vector<request>();
    ub_waiting_queue = new std::vector<request>();
    ub_request_queue = new std::vector<request>();
    
    tiling_queue = new std::vector<request>();
}

MatrixMultiplyUnit::~MatrixMultiplyUnit() {
    delete wf_served_queue;
    delete wf_waiting_queue;
    delete wf_request_queue;
    delete ub_served_queue;
    delete ub_waiting_queue;
    delete ub_request_queue;
    delete tiling_queue;
}

/* Updates tiling_queue so that all requests in ub_sender_queue and wf_sender_queue that share
 * the same 'order' value are deleted from their respective queues and pushed into tiling_queue */
void MatrixMultiplyUnit::UpdateTilingQueue() {
    // update tiling_queue
    std::vector<request>::iterator ubit, wfit;
    std::vector<request>::iterator begin;
    for (ubit = ub_sender_queue->begin(); ubit != ub_sender_queue->end(); ++ubit) {
        for (wfit = wf_sender_queue->begin(); wfit != wf_sender_queue->end(); ++wfit) {
            if ((ubit->order + wfit->order) == 0)
                tiling_queue->push_back(MakeRequest(ubit->order, ubit->size));
        }
    }
    // a little redundant, but delete requests moved to tiling_queue from sender_queues
    for (begin = tiling_queue->begin(); begin != tiling_queue->end(); ++begin) {
        int order = begin->order;
        find_and_delete_by_order(*ub_sender_queue, order);
        find_and_delete_by_order(*wf_sender_queue, order);
    }
}

bool MatrixMultiplyUnit::IsIdle() {
    UpdateTilingQueue();
    return (tiling_queue->empty() && (wait_cycle == 0));
}

void MatrixMultiplyUnit::Cycle() {
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
        wait_cycle = accumulator_size + systolic_array_width + systolic_array_height - 1; //some fixed latency for vector ops/ activations
        current_order = tiling_queue->front().order;
    }
    // 'compute'
    wait_cycle--;
    busy_cycle++;;
    // check if computation is complete
    if (wait_cycle == 0) {
        // increase total number of computation
        total_computation_number += (float)(2 * systolic_array_width * systolic_array_height * accumulator_size);
        // delete from tiling_queue
        find_and_delete_by_order(*tiling_queue, current_order);
        // delete from Matrix Multiply Unit's waiting queues
        find_and_delete_by_order(*ub_waiting_queue, current_order);
        find_and_delete_by_order(*wf_waiting_queue, current_order);
        // delete from Unified Buffer or Weight Fetcher's sender queues
        find_and_delete_by_order(*(ub->GetSenderQueue()), current_order);
        find_and_delete_by_order(*(wf->GetSenderQueue()), current_order);
    }
}

void MatrixMultiplyUnit::PrintStats() {
    std::cout << "======================================================================" << std::endl;
    std::cout << "\t\tMatrix Multiply Unit Statistics:" << std::endl;
    std::cout << "\tidle cycles: " << idle_cycle << ",\t\tbusy cycles: " << busy_cycle << std::endl;
    std::cout << "total number of computations performed by this MMU:\t" << total_computation_number << std::endl;
    std::cout << "======================================================================" << std::endl;
}


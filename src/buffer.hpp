#ifndef BUFFER_H
#define BUFFER_H

#include "common.hpp"
#include "unit.hpp"

class UnifiedBuffer: public Unit {
public:
    UnifiedBuffer(float buffer_size);
    void Cycle();
    
    bool IsMainMemory() {return is_main_memory;}
    // connected to Matrix Multiply Unit
    std::vector<request> *GetSenderQueue() {return sender_queue;}
    // connected to CPU
    std::vector<request> *GetServedQueue() {return served_queue;}
    std::vector<request> *GetWaitingQueue() {return waiting_queue;}
    std::vector<request> *GetRequestQueue() {return request_queue;}

    float GetCapacity() {return capacity;}
private:
    float capacity;         // 2 x size of one buffer
    bool is_main_memory;

    // connected to Matrix Multiply Unit
    std::vector<request> *sender_queue;
    // connected to CPU
    std::vector<request> *served_queue;
    std::vector<request> *waiting_queue;
    std::vector<request> *request_queue;
};

#endif /* BUFFER_H */

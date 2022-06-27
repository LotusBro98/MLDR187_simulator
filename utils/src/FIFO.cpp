#include "FIFO.h"

FIFO::FIFO(uint32_t size): size(size) {
    data = new uint32_t[size];
}

FIFO::~FIFO() {
    delete[] data;
}

bool FIFO::empty() const {
    return stored == 0;
}

void FIFO::push(uint32_t elem) {
    data[(head + stored) % size] = elem;
    if (full()) {
        head = (head + 1) % size;
    } else {
        stored++;
    }
}

bool FIFO::full() const {
    return stored == size;
}

uint32_t FIFO::pop() {
    if (empty())
        return 0;
    uint32_t elem = data[head];
    head = (head + 1) % size;
    stored--;
    return elem;
}

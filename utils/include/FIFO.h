#ifndef MLDR187_SIMULATOR_FIFO_H
#define MLDR187_SIMULATOR_FIFO_H

#include <cstdint>

class FIFO {
public:
    explicit FIFO(uint32_t size);

    virtual ~FIFO();

    void push(uint32_t elem);
    uint32_t pop();

    bool empty() const;
    bool full() const;

private:
    uint32_t size;
    uint32_t head = 0;
    uint32_t stored = 0;
    uint32_t* data;
};


#endif //MLDR187_SIMULATOR_FIFO_H

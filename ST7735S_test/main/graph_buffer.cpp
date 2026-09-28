#include "graph_buffer.hpp"
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 160

void GraphBuffer::enqueue(float value)
{
    data[rear] = value;

    rear = (rear + 1) % SCREEN_HEIGHT;

    if (count < SCREEN_HEIGHT)
    {
        count++;
    }
    else
    {
        front = (front + 1) % SCREEN_HEIGHT; // lets oldest value be overwritten
    }
}

float GraphBuffer::get(int index) const
{
    if (index < 0 || index >= count)
        return 0.0f;

    return data[(front + index) % SCREEN_HEIGHT];
}

int GraphBuffer::size() const
{
    return count;
}
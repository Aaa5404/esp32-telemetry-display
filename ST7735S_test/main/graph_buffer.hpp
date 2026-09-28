#pragma once
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 160

class GraphBuffer
{
private:
    float data[SCREEN_HEIGHT];
    int front = 0;
    int rear = 0;
    int count = 0;

public:
    void enqueue(float value);
    float get(int index) const;
    int size() const;
};
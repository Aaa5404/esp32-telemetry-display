#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>

#define PROTOCOL_VERSION 1

#pragma pack(push, 1)

typedef struct
{
    uint32_t packet_id;

    float gas;
    float brake;
    float fuel;

    int32_t gear;
    int32_t rpm;

} PhysicsData;

#pragma pack(pop)

#endif // PROTOCOL_H
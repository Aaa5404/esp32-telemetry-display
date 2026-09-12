#pragma once

#include <Windows.h>
#include "../common/protocol.h"

class TelemetryReader
{
public:
    TelemetryReader();
    ~TelemetryReader();

    bool initialize();
    void update();
    void shutdown();

    float getGas() const;
    float getBrake() const;
    int getRPM() const;
    PhysicsData getPhysicsData() const;

private:
    HANDLE m_hMap;
    void *m_data;

    float m_gas;
    float m_brake;
    int m_rpm;
    PhysicsData m_physicsData;
};
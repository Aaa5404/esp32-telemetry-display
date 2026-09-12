#include "telemetry.h"
#include "udp.h"
#include <thread>
#include <chrono>

int main()
{
    TelemetryReader telemetry;
    Udpclient udp;

    if (!telemetry.initialize())
    {
        return 1;
    }
    if (!udp.initialize())
    {
        return 1;
    }

    // Main Loop
    bool done = false;
    while (!done)
    {
        telemetry.update();
        auto data = telemetry.getPhysicsData();
        udp.send(data, sizeof(data));
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return 0;
}
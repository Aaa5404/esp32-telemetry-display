#include "telemetry.h"
#include "udp.h"
#include <thread>
#include <chrono>
#include <iostream>

using namespace std;
int main()
{
    TelemetryReader telemetry;
    Udpclient udp;

    bool done = false;

    if (telemetry.initialize())
    {
        done = true;
    }

    if (!udp.initialize())
    {
        return 1;
    }

    // Main Loop

    while (done)
    {
        telemetry.update();
        auto data = telemetry.getPhysicsData();
        udp.send(data, sizeof(data));
        std::this_thread::sleep_for(std::chrono::milliseconds(20)); // 60Hz update rate
    }
    if (!done)
    {
        cout << "Assetto Corsa not open, reading mouse postion" << endl;
        PhysicsData mouseData;
        float gas = 0.0, brake = 0.0;
        while (!done)
        {

            cin >> gas >> brake;
            cout << "Gas: " << gas << endl;
            cout << "Brake: " << brake << endl;
            mouseData.gas = gas;
            mouseData.brake = brake;
            udp.send(mouseData, sizeof(mouseData));

            // this_thread::sleep_for(std::chrono::milliseconds(20));
        }
    }
    return 0;
}
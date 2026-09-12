#include <windows.h>//ac uses win32 api
#include <iostream>


#include "telemetry.h"

//Constructor
TelemetryReader::TelemetryReader()
{
    m_hMap = nullptr;
    m_data = nullptr;
    m_gas = 0.0f;
    m_brake = 0.0f;
    m_rpm = 0;
	m_physicsData = {};//initialize to zero
}

//Destructor
TelemetryReader::~TelemetryReader()
{
    shutdown();
}

bool TelemetryReader::initialize()
{
    m_hMap = OpenFileMappingA(
        FILE_MAP_READ,
        FALSE,
        "Local\\acpmf_physics"
    );

    if (!m_hMap)
        return false;

    m_data = MapViewOfFile(
        m_hMap,
        FILE_MAP_READ,
        0,
        0,
        2048
    );

    return m_data != nullptr;
}

void TelemetryReader::update()
{
    if (!m_data)
        return;

    auto* physics =
        static_cast<PhysicsData*>(m_data);

    m_gas = physics->gas;
    m_brake = physics->brake;
    m_rpm = physics->rpm;
	m_physicsData = *physics;
}

void TelemetryReader::shutdown()
{
    if (m_data)
    {
        UnmapViewOfFile(m_data);
        m_data = nullptr;
    }

    if (m_hMap)
    {
        CloseHandle(m_hMap);
        m_hMap = nullptr;
    }
}

float TelemetryReader::getGas() const
{
    return m_gas;
}

float TelemetryReader::getBrake() const
{
    return m_brake;
}

int TelemetryReader::getRPM() const
{
    return m_rpm;
}

PhysicsData TelemetryReader::getPhysicsData() const
{
    return m_physicsData;
}

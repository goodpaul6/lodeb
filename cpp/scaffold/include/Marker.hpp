#pragma once

#include <chrono>
#include <string>
#include <vector>
#include <memory>

#include <iostream>

namespace Scaffold
{
    typedef std::chrono::time_point<std::chrono::high_resolution_clock> TimePoint;
    typedef std::chrono::nanoseconds NanoSecs;

    struct Marker
    {
        std::string name = "Unnamed";
        float durationAsMilliseconds = 0;
        float durationAsSeconds = 0;

        size_t measurementCount = 0;

        TimePoint start;
        TimePoint end;

        std::vector<std::unique_ptr<Marker>> subMarkers;
        Marker* parentMarker = nullptr;

        Marker();
        Marker(const std::string name);

        void StartMeasurement();
        void CompleteMeasurement();

        Marker* CreateSubMarker(std::string name);
    };
} // namespace Scaffold

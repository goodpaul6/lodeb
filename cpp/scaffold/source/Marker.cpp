#include <Marker.hpp>

// ------
Scaffold::TimePoint GetTime()
{
    return std::chrono::high_resolution_clock::now();
}

float GetElapsed(Scaffold::TimePoint start, Scaffold::TimePoint end)
{
    Scaffold::NanoSecs measurement = std::chrono::duration_cast<Scaffold::NanoSecs>(end - start);
    float duration = measurement.count() * 0.000001;

    return duration;
}
// ------

Scaffold::Marker::Marker(const std::string name)
{
    this->name = name;
    StartMeasurement();
}

Scaffold::Marker::Marker()
{
    StartMeasurement();
}

void Scaffold::Marker::StartMeasurement()
{
    start = GetTime();
}

void Scaffold::Marker::CompleteMeasurement()
{
    measurementCount++;

    end = GetTime();
    durationAsMilliseconds += GetElapsed(start, end);
    durationAsSeconds = durationAsMilliseconds * 0.001;

    if (measurementCount > 1)
    {
        durationAsMilliseconds *= 0.5f;
    }
}

Scaffold::Marker* Scaffold::Marker::CreateSubMarker(std::string name)
{
    std::unique_ptr<Marker>& newMarker = subMarkers.emplace_back(std::make_unique<Marker>(name));
    Marker* markerPtr = newMarker.get();
    return markerPtr;
}

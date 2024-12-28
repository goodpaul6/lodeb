#pragma once

#include <string>

namespace Scaffold
{
    struct Manifest
    {
        std::string title = "Scaffold";

        uint32_t initialWidth = 1280;
        uint32_t initialHeight = 720;

        uint32_t swapInterval = 1;

        float eventWaitTimeout = -1.0f;

        bool dockspaceOverViewport = false;

        bool useProfilerLayer = false;
        bool useInputInfoLayer = false;
        bool useDemoLayer = true;
    };
} // namespace Scaffold
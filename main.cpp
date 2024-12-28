#include <Application.hpp>
#include <lldb/API/LLDB.h>

#include "lodeb/AppLayer.hpp"

int main(int, char**)
{
    lldb::SBDebugger::Initialize();

    Scaffold::Manifest manifest = {};

    manifest.title = "Lodeb";
    manifest.initialWidth = 1280;
    manifest.initialHeight = 920;

    manifest.dockspaceOverViewport = true;

    manifest.useProfilerLayer = false;
    manifest.useInputInfoLayer = false;
    manifest.useDemoLayer = false;

    manifest.swapInterval = 0;

    Scaffold::Application app{manifest};

    app.CreateObject<lodeb::AppLayer>("App");

    app.Run();

    lldb::SBDebugger::Terminate();

    return 0;
}

#include <Application.hpp>

int main(int, char**)
{
    Scaffold::Manifest manifest = {};

    manifest.title = "Scaffold";
    manifest.initialWidth = 1280;
    manifest.initialHeight = 920;

    manifest.dockspaceOverViewport = true;

    manifest.useProfilerLayer = true;
    manifest.useInputInfoLayer = true;
    manifest.useDemoLayer = true;

    manifest.swapInterval = 0;

    Scaffold::Application app(manifest);

    app.Run();

    return 0;
}
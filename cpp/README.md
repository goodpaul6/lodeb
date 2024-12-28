# Scaffold
A simple framework for quickstarting GUI applications with ImGui. Inspired by [StudioCherno/Walnut](https://github.com/StudioCherno/Walnut). 

My main goal for Scaffold is having a personal, stable codebase for quickly setting up other projects. If it proves useful to you, that's a plus too.

### Third Party Libraries
- [ImGui](https://github.com/ocornut/imgui)
- [GLFW](https://github.com/glfw/glfw)
- [GLAD](https://github.com/Dav1dde/glad)
- [glm](https://github.com/g-truc/glm)

# Getting Started
### Building
Scaffold uses CMake, so everything should be ready to go after cloning this repository and configuring CMake. GLFW, ImGui and GLAD is built alongside Scaffold and linked statically. glm is header only, so you compile what you include.

I'm still a CMake newbie and this is only tested under Linux. This might not work under your environment and further tinkering might be required on your part.

### Using
You use Scaffold by creating "AppObjects". An AppObject is defined as a C++ concept like so:

```cpp
#include <interface/IUpdate.hpp>
#include <interface/IRenderUI.hpp>

template <typename Derived>
concept AppObjectConcept =
    std::derived_from<Derived, Scaffold::IUpdate> ||
    std::derived_from<Derived, Scaffold::IRenderUI>;
```

This is a fancy way of saying "an AppObject is an object which inherits from either IUpdate or IRenderUI, or both." IUpdate and IRenderUI are simple interfaces, they define a single virtual function for you to override. 

> These interfaces also inherit from a root object called IAppObject which only has a single member `std::string name`.

In case of IRenderUI it defines `virtual void OnRenderUI(float deltaTime)`:

```cpp
class HelloLayer : public Scaffold::IRenderUI
{
public:
    void OnRenderUI(float deltaTime)
    {
        ImGui::Begin("Hello");
        ImGui::TextUnformatted("Hello from Scaffold!");
        ImGui::End();
    }
};

```

I tend to call an object which only inherits from IRenderUI a "Layer". Walnut does this too, I think it's an appropriate terminology. It's not formalized in code however, you can name your objects however you want.

Anyway, you then tell the main app to create a such object with `CreateObject<AppObject>(std::string)`. It acts like a factory:

```cpp
int main(int, char**)
{
    Scaffold::Manifest manifest = {};
    Scaffold::Application app(manifest);

    HelloLayer& helloObj = app.CreateObject<HelloLayer>("Demo UI");

    app.Run();
}
```

The app calls the objects OnRenderUI method every frame. You can use ImGui inside this method to draw your UI. The function also returns a reference to the created object if you want to further set it up, since you can not forward constructor arguments (yet). This means that every AppObject should be default constructible.

Same story goes with IUpdate. It defines a function, `virtual void OnUpdate(float deltaTime)`, which gets called every frame, before UI rendering. You can't call ImGui functions inside OnUpdate; ImGui frame setup code runs after every OnUpdate finishes. You can use this to implement simple update loops for your objects.

There's also a Manifest object that gets passed into the Application constructor. It contains simple configuration values for your app, such as the title of the window and starting size. Check the header file `Manifest.hpp`.

# Utilities
Scaffold also includes a few utilities. 

## Input
This is a simple input utility. Through this utility you can query the state of a key. Keys go through a lifecycle like so:

> None -> (From GLFW) Pressed -> (After one frame, if not released) Held -> (From GLFW) Released -> (After one frame) None

You can also enable a built-in layer that shows key state info from the app manifest;

```cpp
Scaffold::Manifest manifest = {};
manifest.useInputInfoLayer = true;
```

### Usage
```cpp
#include <Input.hpp>
#include <KeyCodes.h>

Scaffold::Input& input = Scaffold::Application::GetInput();
Scaffold::KeyState state = input.GetKeyState(Scaffold::KeyCode::C);
glm::vec2 mousePos = input.GetMousePosition();

if (state == Scaffold::KeyState::Pressed) ...
```

## Profiler
Profiler is used to, well, you guessed it, profile a section of code. It accomplishes this through markers. You begin a marker, do your work, and then end your marker. The marker records the time difference between begin/end calls.

You can also begin another marker before ending one, the new marker becomes the child of the preceding marker. This is used to keep parent-child relationships between measurements and does not directly impact a measurement.

You do not have direct access to markers, though you can get the root marker and iterate its children (and their children and so on) to access your measurements. You can also call GenerateReport to generate a pretty printed string of your measurements.

Scaffold's internal app loop contains its own markers, so you can see how long it takes to iterate through all IUpdate objects for example.

The root marker always contains the measurements of the previous frame.

There's also a ProfilerLayer which draws a tree view of all measurements. You can enable it like so:
```cpp
Scaffold::Manifest manifest = {};
manifest.useProfilerLayer = true;
```

### Usage
```cpp
#include <Profiler.hpp>

Scaffold::Profiler profiler = Scaffold::Application::GetProfiler();

profiler.BeginMarker("Solve Collisions");

    PrepareSolver();
    
    profiler.BeginMarker("Solver::BroadPhase()");
    BroadPhase();
    profiler.EndMarker();

    profiler.BeginMarker("Solver::NarrowPhase()");
    NarrowPhase();
    profiler.EndMarker();

profiler.EndMarker();
```
#pragma once

#include <glad/gl.h>
#include <glfw3.h>

#include <vector>
#include <memory>

#include <Manifest.hpp>
#include <Input.hpp>
#include <Profiler.hpp>

#include <interface/IUpdate.hpp>
#include <interface/IRenderUI.hpp>

namespace Scaffold
{
  template <typename Derived>
  concept AppObjectConcept =
    std::derived_from<Derived, IUpdate> ||
    std::derived_from<Derived, IRenderUI>;

  class Application
  {
  public:
    Application(const Manifest manifest);
    ~Application();

  public:
    void Run();
    GLFWwindow* GetWindowHandle();

  public:
    static Input& GetInput();
    static Profiler& GetProfiler();
    static Application& GetInstance();

  public:
    template<AppObjectConcept Object>
    Object& CreateObject(std::string name)
    {
      std::shared_ptr<Object> object = std::make_shared<Object>();
      object.get()->name = name;

      if constexpr (std::is_base_of_v<IUpdate, Object>)
      {
        std::shared_ptr<IUpdate> updateObject(object);
        m_updateObjects.push_back(updateObject);
      }

      if constexpr (std::is_base_of_v<IRenderUI, Object>)
      {
        std::shared_ptr<IRenderUI> uiObject(object);
        m_uiObjects.push_back(uiObject);
      }

      return *object.get();
    }

  private:
    bool m_initialized;

    Manifest m_manifest;

    std::unique_ptr<Profiler> m_profiler;
    std::unique_ptr<Input> m_input;

    GLFWwindow* m_glfwWindow;

    std::vector<std::shared_ptr<IUpdate>> m_updateObjects;
    std::vector<std::shared_ptr<IRenderUI>> m_uiObjects;

    Application(const Application&) = delete;
    Application(Application&&) = delete;
  };
} // namespace Scaffold

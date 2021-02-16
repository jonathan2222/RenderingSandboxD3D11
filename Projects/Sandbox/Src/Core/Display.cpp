#include "PreCompiled.h"
#include "Display.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "Renderer/Renderer.h"

using namespace RS;

// TODO: Put this into a input class!
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);

std::shared_ptr<Display> Display::Get()
{
    static std::shared_ptr<Display> s_Display = std::make_shared<Display>();
    return s_Display;
}

void RS::Display::Init(const DisplayDescription& description)
{
	m_pSelf = this;

	SetDescription(description);
	m_ShouldClose = false;

	RS_ASSERT(glfwInit(), "Failed to initialize GLFW!");

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

	glfwSetErrorCallback(Display::ErrorCallback);

	GLFWmonitor* monitor = nullptr;
	if (m_Description.Fullscreen)
	{
		monitor = glfwGetPrimaryMonitor();

		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		glfwWindowHint(GLFW_RED_BITS, mode->redBits);
		glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
		glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
		glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
		m_Description.Width = mode->width;
		m_Description.Height = mode->height;
	}

	m_pWindow = glfwCreateWindow(m_Description.Width, m_Description.Height, m_Description.Title.c_str(), monitor, nullptr);
	if (m_pWindow == NULL)
	{
		glfwTerminate();
		RS_ASSERT(false, "Failed to create GLFW window!");
	}
	m_HWND = glfwGetWin32Window(m_pWindow);
	
	glfwSetFramebufferSizeCallback(m_pWindow, Display::FrameBufferResizeCallback);

	glfwSetInputMode(m_pWindow, GLFW_STICKY_KEYS, 1);
	glfwSetKeyCallback(m_pWindow, KeyCallback);
}

void RS::Display::Release()
{
	glfwDestroyWindow(m_pWindow);
	glfwTerminate();
}

bool RS::Display::ShouldClose() const
{
	return glfwWindowShouldClose(m_pWindow) != 0 || m_ShouldClose;
}

void RS::Display::Close() noexcept
{
	m_ShouldClose = true;
}

void RS::Display::PollEvents()
{
	glfwPollEvents();
}

GLFWwindow* Display::GetGLFWWindow()
{
	return m_pWindow;
}

HWND RS::Display::GetHWND()
{
	return m_HWND;
}

void Display::SetDescription(const DisplayDescription& description)
{
    m_Description = description;
}

DisplayDescription& Display::GetDescription()
{
    return m_Description;
}

uint32 Display::GetWidth() const
{
    return m_Description.Width;
}

uint32 Display::GetHeight() const
{
    return m_Description.Height;
}

float Display::GetAspectRatio() const
{
    return (float)m_Description.Width / (float)m_Description.Height;
}

void RS::Display::ErrorCallback(int error, const char* description)
{
	RS_UNREFERENCED_VARIABLE(error);
	RS_ASSERT(false, "{0}", description);
}

void RS::Display::FrameBufferResizeCallback(GLFWwindow* window, int width, int height)
{
	RS_UNREFERENCED_VARIABLE(window);
	// TODO: Call resize on the renderer! (Maybe send an event?)

	// Update display description.
	DisplayDescription& description = m_pSelf->GetDescription();
	description.Width = width;
	description.Height = height;
	description.Fullscreen = false;

	Renderer::Get()->Resize((uint32)width, (uint32)height);
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	using namespace RS;
	RS_UNREFERENCED_VARIABLE(mode);
	RS_UNREFERENCED_VARIABLE(scancode);

	LOG_INFO("Pressed key: {}", key);
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}
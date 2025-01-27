#include <Timer.hpp>

#include <GLFW/glfw3.h>

Timer::Timer()
	:
	m_prevTime(0.0),
	m_currentTime(0.0),
	m_timeData()
{
	//
}

void Timer::Tick()
{
	m_prevTime = m_currentTime;
	m_currentTime = glfwGetTime();

	m_timeData.DeltaTime = m_currentTime - m_prevTime;
	m_timeData.FPS = 1.0f / m_timeData.DeltaTime;
}
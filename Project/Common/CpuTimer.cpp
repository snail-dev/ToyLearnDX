#include <Windows.h>
#include "CpuTimer.h"

CpuTimer::CpuTimer() {
	__int64 countsPerSec {};
	QueryPerformanceFrequency((LARGE_INTEGER*) &countsPerSec);
	m_SecondsPerCount = 1.0 / (double) countsPerSec;
}

float CpuTimer::TotalTime() const {
	if(m_Stopped) {
		return (float) (((m_StopTime - m_PausedTime) - m_BaseTime) * m_SecondsPerCount);

	}  else {
		return (float)(((m_CurrTime - m_PausedTime) - m_BaseTime) * m_SecondsPerCount);
	}

}

float CpuTimer::DeltaTime() const {
	return (float) m_DeltaTime;
}

bool CpuTimer::IsStopped() const {
	return m_Stopped;
}


void CpuTimer::Reset() {
	__int64 currTime{};
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

	m_BaseTime = currTime;
	m_PrevTime = currTime;
	m_StopTime = 0;
	m_PausedTime = 0;
	m_Stopped = false;

}

void CpuTimer::Start() {

	__int64 startTime {};
	QueryPerformanceCounter((LARGE_INTEGER*)&startTime);

	if(m_Stopped) {
		m_Stopped = false;

		m_PausedTime += (startTime - m_StopTime);
		m_StopTime = 0;
		m_PrevTime = startTime;
	}
}

void CpuTimer::Stop() {
	if(!m_Stopped) {
		__int64 currTime {};
		QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

		m_StopTime = currTime;
		m_Stopped = true;
	}
}

void CpuTimer::Tick() {

	if(m_Stopped) {
		m_DeltaTime = 0.0;
		return;
	}

	__int64 currTime {};
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
	m_CurrTime = currTime;

	m_DeltaTime = (m_CurrTime - m_PrevTime) * m_SecondsPerCount;
	m_PrevTime = m_CurrTime;

	if(m_DeltaTime < 0.0) {
		m_DeltaTime = 0.0;
	}
}

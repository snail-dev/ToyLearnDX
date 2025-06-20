#ifndef _CPU_TIMER_H_
#define _CPU_TIMER_H_

class CpuTimer {
public:
    CpuTimer();

    float TotalTime() const;
    float DeltaTime() const;
    bool IsStopped() const;

    void Reset();
    void Start();
    void Stop();
    void Tick();

private:
    double m_SecondsPerCount = 0.0;
    double m_DeltaTime = -1.0;

    __int64 m_BaseTime = 0;
    __int64 m_PausedTime = 0;
    __int64 m_StopTime = 0;
    __int64 m_PrevTime = 0;
    __int64 m_CurrTime = 0;

    bool m_Stopped = false;
};

#endif  // _CPU_TIMER_H_

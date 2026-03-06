#pragma once
#include "PdhQuery.h"
#include <vector>
#include <string>

struct ProcessCpuInfo
{
    std::wstring name;
    int cpu_percent{};
};

class CTopProcessMonitor : public CPdhQuery
{
public:
    CTopProcessMonitor();
    ~CTopProcessMonitor();

    // 获取CPU占用率最高的前3个进程
    bool GetTopProcesses(std::vector<ProcessCpuInfo>& top3);

    bool IsAvailable() const { return m_isAvailable; }

private:
    bool m_isAvailable{ false };
    int m_processor_count{ 1 };
};

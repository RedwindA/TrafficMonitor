#include "stdafx.h"
#include "TopProcessMonitor.h"
#include <algorithm>

CTopProcessMonitor::CTopProcessMonitor()
    : CPdhQuery(_T("\\Process(*)\\% Processor Time"))
{
    m_isAvailable = Initialize();
    if (m_isAvailable)
    {
        // 预热：触发PDH内部初始化
        std::vector<CounterValueItem> dummy;
        QueryValues(dummy);
    }

    // 获取逻辑处理器数
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    m_processor_count = static_cast<int>(sysInfo.dwNumberOfProcessors);
    if (m_processor_count < 1)
        m_processor_count = 1;
}

CTopProcessMonitor::~CTopProcessMonitor()
{
}

bool CTopProcessMonitor::GetTopProcesses(std::vector<ProcessCpuInfo>& top3)
{
    top3.clear();
    if (!m_isAvailable)
        return false;

    std::vector<CounterValueItem> values;
    if (!QueryValues(values) || values.empty())
        return false;

    // 转换为ProcessCpuInfo，过滤掉_Total和Idle
    std::vector<ProcessCpuInfo> processes;
    for (const auto& item : values)
    {
        if (item.name == L"_Total" || item.name == L"Idle")
            continue;

        ProcessCpuInfo info;
        info.name = item.name;
        // 多核归一化：PDH返回的值是所有核心的总和，需除以处理器数
        double normalized = item.value / m_processor_count;
        info.cpu_percent = static_cast<int>(normalized + 0.5);
        if (info.cpu_percent > 100)
            info.cpu_percent = 100;
        if (info.cpu_percent < 0)
            info.cpu_percent = 0;
        processes.push_back(info);
    }

    // 按CPU占用降序排序
    std::sort(processes.begin(), processes.end(),
        [](const ProcessCpuInfo& a, const ProcessCpuInfo& b) {
            return a.cpu_percent > b.cpu_percent;
        });

    // 取前3
    int count = (std::min)(static_cast<int>(processes.size()), 3);
    for (int i = 0; i < count; i++)
    {
        top3.push_back(processes[i]);
    }

    // 不足3个时补空
    while (static_cast<int>(top3.size()) < 3)
    {
        ProcessCpuInfo empty;
        empty.name = L"-";
        empty.cpu_percent = 0;
        top3.push_back(empty);
    }

    return true;
}

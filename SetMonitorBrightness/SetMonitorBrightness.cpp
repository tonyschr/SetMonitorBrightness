#include "pch.h"
#include "resource.h"

HINSTANCE g_hInstance = nullptr;

struct PHYSICAL_MONITOR_INFO
{
    HANDLE hPhysicalMonitor;
    DWORD minimumBrightness;
    DWORD maximumBrightness;
    DWORD currentBrightness;
    std::wstring description;
};

void DebugLog(const WCHAR* formatString, ...)
{
#ifdef _DEBUG
    va_list argList;
    va_start(argList, formatString);
    WCHAR debug[256] = {};
    _vsnwprintf_s(debug, ARRAYSIZE(debug), formatString, argList);
    OutputDebugString(debug);
    OutputDebugString(L"\r\n");
    va_end(argList);
#endif // DEBUG
}

void ShowMessage(int resourceId)
{
    WCHAR caption[128];
    WCHAR message[512];
    if (LoadString(g_hInstance, IDS_MESSAGE_CAPTION, caption, ARRAYSIZE(caption)) != 0 &&
        LoadString(g_hInstance, resourceId, message, ARRAYSIZE(message)) != 0)
    {
        MessageBox(nullptr, message, caption, MB_OK);
    }
}

DWORD GetRequestedBrightness()
{
    PCWSTR commandLine = GetCommandLine();

    int cArgs = 0;
    PWSTR* arguments = CommandLineToArgvW(commandLine, &cArgs);
    if (cArgs == 2)
    {
        return wcstoul(arguments[1], nullptr, 10);
    }

    return 0;
}

BOOL CALLBACK RecordMonitorProc(
    HMONITOR monitor,
    HDC hdc,
    LPRECT rect,
    LPARAM lParam)
{
    std::vector<HMONITOR>* monitors = reinterpret_cast<std::vector<HMONITOR>*>(lParam);
    monitors->push_back(monitor);
    return true;
}

std::vector<PHYSICAL_MONITOR_INFO> GetAllPhysicalMonitors()
{
    std::vector<PHYSICAL_MONITOR_INFO> physicalMonitorInfos;

    std::vector<HMONITOR> monitors;
    if (!EnumDisplayMonitors(nullptr, nullptr, RecordMonitorProc, reinterpret_cast<LPARAM>(&monitors)))
    {
        return physicalMonitorInfos;
    }

    for (const auto& monitor : monitors)
    {
        DWORD cMonitors = 0;
        if (GetNumberOfPhysicalMonitorsFromHMONITOR(monitor, &cMonitors))
        {
            PHYSICAL_MONITOR* physicalMonitors = new PHYSICAL_MONITOR[cMonitors];
            if (GetPhysicalMonitorsFromHMONITOR(monitor, cMonitors, physicalMonitors))
            {
                for (DWORD i = 0; i < cMonitors; i++)
                {
                    DWORD minimumBrightness = 0;
                    DWORD currentBrightness = 0;
                    DWORD maximumBrightness = 0;

                    // Note: We currently don't use GetMonitorCapabilities; we just expect this to
                    // fail on any monitors that don't support adjusting the brightness and continue
                    // on.
                    if (GetMonitorBrightness(physicalMonitors[i].hPhysicalMonitor, 
                        &minimumBrightness, 
                        &currentBrightness, 
                        &maximumBrightness))
                    {
                        PHYSICAL_MONITOR_INFO monitorInfo = {};
                        monitorInfo.hPhysicalMonitor = physicalMonitors[i].hPhysicalMonitor;
                        monitorInfo.minimumBrightness = minimumBrightness;
                        monitorInfo.maximumBrightness = maximumBrightness;
                        monitorInfo.currentBrightness = currentBrightness;
                        monitorInfo.description = physicalMonitors[i].szPhysicalMonitorDescription;
                        physicalMonitorInfos.push_back(monitorInfo);
                    }
                }
            }

            delete[] physicalMonitors;
        }
    }

    return physicalMonitorInfos;
}

void SetBrightnessForAllMonitors(DWORD newBrightness)
{
    std::vector<PHYSICAL_MONITOR_INFO> physicalMonitorInfos = GetAllPhysicalMonitors();
    for (const auto& physicalMonitorInfo : physicalMonitorInfos)
    {
        DebugLog(L"Monitor=%s, minBrightness=%d, maxBrightness=%d, currentBrightness=%d",
            physicalMonitorInfo.description.c_str(), 
            physicalMonitorInfo.minimumBrightness, 
            physicalMonitorInfo.maximumBrightness, 
            physicalMonitorInfo.currentBrightness);
        SetMonitorBrightness(physicalMonitorInfo.hPhysicalMonitor, newBrightness);
    }
}

int APIENTRY wWinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPWSTR    lpCmdLine,
    int       nCmdShow)
{
    g_hInstance = hInstance;
    SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);

    DWORD newBrightness = GetRequestedBrightness();
    if (newBrightness >= 10)
    {
        SetBrightnessForAllMonitors(newBrightness);
    }
    else
    {
        // Unreasonably dim. Ideally this should leverage GetAllPhysicalMonitors to tell the user
        // the capabilities of their monitors, or if no monitors are found where the brightness
        // can be programmatically set.
        ShowMessage(IDS_ERROR_USAGE);
    }

    return 0;
}


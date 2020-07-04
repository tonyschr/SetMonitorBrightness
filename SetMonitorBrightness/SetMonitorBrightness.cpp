#include "pch.h"
#include "resource.h"

HINSTANCE g_hInstance = nullptr;

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

int APIENTRY wWinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPWSTR    lpCmdLine,
    int       nCmdShow)
{
    g_hInstance = hInstance;
    SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);

    DWORD newBrightness = GetRequestedBrightness();
    if (newBrightness < 10)
    {
        // Unreasonably dim. Ideally this should compare against the monitor's capabilities.
        ShowMessage(IDS_ERROR_USAGE);
        return 0;
    }

    std::vector<HMONITOR> monitors;
    if (!EnumDisplayMonitors(nullptr, nullptr, RecordMonitorProc, reinterpret_cast<LPARAM>(&monitors)))
    {
        return 0;
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

                    if (GetMonitorBrightness(physicalMonitors[i].hPhysicalMonitor, &minimumBrightness, &currentBrightness, &maximumBrightness))
                    {
                        if (newBrightness >= minimumBrightness && newBrightness <= maximumBrightness)
                        {
                            SetMonitorBrightness(physicalMonitors[i].hPhysicalMonitor, newBrightness);
                        }
                    }
                }
            }

            delete[] physicalMonitors;
        }
    }

    return 0;
}


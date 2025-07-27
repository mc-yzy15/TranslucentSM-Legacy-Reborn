#include <windows.h>

class TranslucentSM {
  public:
    void applyTransparencySettings();
    DWORD GetBuildNumber();

private:
    DWORD GetProcessIdByName(const wchar_t* processName);
    bool InjectDLL(DWORD processId, const wchar_t* dllPath);
    bool SetRegistryValues();
};
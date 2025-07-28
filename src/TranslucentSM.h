#include <windows.h>
#include <QString>

class TranslucentSM {
  public:
    void applyTransparencySettings(const QString& processName, int transparencyValue);
    DWORD GetBuildNumber();

private:
    DWORD GetProcessIdByName(const wchar_t* processName);
    bool InjectDLL(DWORD processId, const wchar_t* dllPath);
    bool SetRegistryValues();
};
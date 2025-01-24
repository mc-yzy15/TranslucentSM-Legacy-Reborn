class TranslucentSM {
public:
    void applyTransparencySettings();

private:
    DWORD GetProcessIdByName(const wchar_t* processName);
    bool InjectDLL(DWORD processId, const wchar_t* dllPath);
    bool SetRegistryValues();
};
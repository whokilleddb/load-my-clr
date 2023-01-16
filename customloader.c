#include "customloader.h"

ICLRMetaHost * metahost = NULL;
IEnumUnknown * runtime = NULL;
ICLRRuntimeInfo * runtimeinfo = NULL;
ICLRRuntimeHost * runtimehost = NULL;

void CleanUp(){
    printf("[i] Performing Cleanup!\n");
    
    if (runtime != NULL) {
        IEnumUnknown_Release(runtime);
    }

    if (runtimeinfo != NULL) {
        ICLRRuntimeInfo_Release(runtimeinfo);
    }

    if (runtimehost != NULL) {
        ICLRRuntimeHost_Release(runtimehost);
    }
    
    if (metahost != NULL){
        ICLRMetaHost_Release(metahost);
    }
    printf("[i] Cleanup Done!\n");
}

HRESULT GetCLRInterface(){
    DWORD count;
    IUnknown *unk;
    WCHAR buf[MAX_PATH];
    HRESULT result = S_OK;

    // Create instance of CLR Metahost for managing the versioning and loading of the CLR on a system. 
    // Also note that the C and C++ versions of the same will be different
    result = CLRCreateInstance(
                &CLSID_CLRMetaHost,          // The CLSID_CLRMetaHost is used to create an instance of the CLRMetaHost class 
                &IID_ICLRMetaHost,           // Corressponding RIID 
                (LPVOID*)&metahost);        // Interface

    if (result != S_OK){
        fprintf(stderr, "[!] CLRCreateInstance() function failed (0x%x)\n", result);
        return result;
    }
    printf("[i] Created CLR Metahost Instance\n");

    // Enumerate installed CLR runtimes
    result = ICLRMetaHost_EnumerateInstalledRuntimes(metahost, &runtime);
    if (result != S_OK){
        fprintf(stderr, "[!] EnumerateInstalledRuntimes() function failed (0x%x)\n", result);
        return result;
    }
    printf("[i] Enumerated Runtimes\n"); 

    // Iterate over runtimes
    while((result = IEnumUnknown_Next(
                                runtime,
                                1,          // Number of elements to retrieve
                                &unk,       // Pointer to an array of IUnknown pointers that will receive the elements
                                0)          // Pointer to a ULONG variable that will receive the number of elements actually retrieved
                            ) == S_OK){

        // Obtain a pointer to another interface on an object. 
        result = IUnknown_QueryInterface(
                    unk,                     // Pointer to the IUnknown interface of the object being queried.
                    &IID_ICLRRuntimeInfo,    
                    (void**)&runtimeinfo);

        count = MAX_PATH;
        if (result == S_OK){
            // Get Version String
            result =  ICLRRuntimeInfo_GetVersionString(
                runtimeinfo,
                buf,
                &count                
            );
            if (result == S_OK && count != 0) {
                wprintf(L"[i] Found Runtime Version: %s\n", buf);
            }
            else {
                fprintf(stderr, "[!] ICLRRuntimeInfo_GetVersionString() Failed (0x%x)\n", result);
            }
        }   
        IUnknown_Release(unk);
        break;
    }

    // Check if a specific version of the CLR is loadable on the current system
    BOOL bLoadable;
    ICLRRuntimeInfo_IsLoadable(runtimeinfo, &bLoadable);
    if (bLoadable == FALSE){
        fprintf(stderr, "[!] Specified version of CLR is not loadable\n");
        return OLEOBJ_S_INVALIDHWND;
    }

    // Get last supported runtime
    result = ICLRRuntimeInfo_GetInterface(
        runtimeinfo,
        &CLSID_CLRRuntimeHost, 
        &IID_ICLRRuntimeHost, 
        (LPVOID*)&runtimehost);
    
    if (result != S_OK){
        fprintf(stderr, "[!] ICLRRuntimeInfo_GetInterface() function failed (0x%x)\n", result);
        return result;
    }

    // Check runtime host
    if (count != 0 && NULL == runtimehost){
        fprintf(stderr, "[!] No Valid Runtime Found\n");
        return OLEOBJ_S_INVALIDHWND;
    }

    wprintf(L"[i] Using Runtime: %s\n", buf);
    return result;
};

HRESULT StartRuntime(){
    HRESULT result;

    // Start Runtime
    printf("[i] Starting Runtime\n");
    result =  ICLRRuntimeHost_Start(runtimehost);
    if (result != S_OK){
        fprintf(stderr, "[!] ICLRRuntimeHost_Start() function failed (0x%x)\n", result);
        return result;
    }

    return S_OK;
};

HRESULT RunAssembly(const LPCWSTR assembly_path, const LPCWSTR namespace_class, const LPCWSTR function_name, const LPCWSTR cmd_arguments){
    HRESULT result;
    DWORD res = 0;
    printf("\n[i] Entering Land of Managed Code\n");
    
    // Run Assembly
    result = ICLRRuntimeHost_ExecuteInDefaultAppDomain(
        runtimehost,
        assembly_path,
        namespace_class,
        function_name,
        cmd_arguments,
        &res
    );
    if (result != S_OK){
        fprintf(stderr, "[!] ICLRRuntimeHost_ExecuteInDefaultAppDomain() function failed (0x%x)\n", result);
        if (result==0x80070002){
            fprintf(stderr, "[!] The speicified .NET assembly could not be found\n");
        }
        return result;
    }
    printf("[i] Exit Code: %d\n", res);
    printf("[i] Back to Unmanaged Land\n\n");
    return S_OK;
}

HRESULT StopRuntime(){
    HRESULT result;

    // Stop Runtime
    printf("[i] Stopping Runtime\n");
    result = ICLRRuntimeHost_Stop(runtimehost);
    if (result != S_OK){
        fprintf(stderr, "[!] ICLRRuntimeHost_Stop() function failed (0x%x)\n", result);
        return result;
    }

    return result;
}

int main(){
    if (GetCLRInterface() != S_OK){
        fprintf(stderr, "[!] Failed to get CLR interface!\n");
        CleanUp();
        return -1;
    }

    if (StartRuntime() != S_OK){
        fprintf(stderr, "[!] Failed to Start runtime!\n");
        CleanUp();
        return -2;
    }

    if (RunAssembly(
        L"C:\\Users\\whokilleddb\\Codes\\load-my-clr\\helloworld.dll",
        L"HelloWorld.Program",
        L"EntryPoint",
        L"Hello There(General Kenobi)"
    ) != S_OK){
        fprintf(stderr, "[!] Failed to Run Assembly!\n");
        CleanUp();
        return -3;       
    }

    if (StopRuntime() != S_OK){
        fprintf(stderr, "[!] Failed to Stop runtime!\n");
        CleanUp();
        return -4;
    }

    CleanUp();
    return 0;
}
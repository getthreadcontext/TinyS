# SeliwareWrapper - C# COM Wrapper for SeliwareAPI

SeliwareWrapper is a C# Class Library project that acts as a COM (Component Object Model) wrapper around the `SeliwareAPI.dll`. This allows native C++ applications, like the TinyS Text Editor, to easily interact with the .NET-based Seliware API.

## Purpose

*   To expose Seliware API functionality to COM-compatible clients (e.g., C++, VB6).
*   To handle the interop details between native code and the managed .NET Seliware API.

## Features Exposed via COM

The wrapper exposes the following Seliware API functionalities:

*   `Initialize()`: Initializes the Seliware API.
*   `Inject()`: Injects into the default Roblox process.
*   `InjectWithPid(int pid)`: Injects into a Roblox process with a specific Process ID.
*   `IsInjected()`: Checks if Seliware is currently injected.
*   `Execute(string script)`: Executes a script via Seliware.
*   `GetClientsCount()`: Gets the number of injected Roblox clients.
*   `GetVersion()`: Gets the Seliware API version.

*(Note: The Seliware `Injected` event is handled internally by the wrapper but not directly exposed as a COM event in this basic implementation for simplicity. The C++ client would typically poll `IsInjected()` or rely on the return status of `Inject()`.)*

## Prerequisites

*   .NET Framework 4.8 (or the version targeted by `SeliwareAPI.dll`).
*   `SeliwareAPI.dll`
*   Dependencies of `SeliwareAPI.dll`:
    *   `Newtonsoft.Json.dll`
    *   `websocket-sharp.dll`

## Building SeliwareWrapper

### Requirements:

*   Visual Studio (e.g., 2019, 2022) with .NET Desktop Development workload.

### Steps:

1.  **Open Project:**
    *   Clone or download the SeliwareWrapper source code.
    *   Open `SeliwareWrapper.sln` (or `SeliwareWrapper.csproj`) in Visual Studio.

2.  **Add Dependencies as References:**
    *   Right-click on "References" in the Solution Explorer for the SeliwareWrapper project.
    *   Select "Add Reference..."
    *   Click "Browse..." and navigate to and select:
        *   `SeliwareAPI.dll`
        *   `Newtonsoft.Json.dll`
        *   `websocket-sharp.dll`
    *   Click "OK".

3.  **Configure Project Properties:**
    *   Right-click the `SeliwareWrapper` project in Solution Explorer and select "Properties".
    *   **Application Tab:**
        *   Ensure "Target framework" is set to **.NET Framework 4.8** (or the version required by `SeliwareAPI.dll`).
    *   **Build Tab:**
        *   Set "Platform target" to **x64**.
        *   Check the box for **"Register for COM interop"**. This is essential for making the DLL usable by COM clients and for generating the Type Library (`.tlb`).

4.  **Build the Project:**
    *   Select a configuration (e.g., Release, x64).
    *   Build the project (Build -> Build Solution).
    *   **Administrator Privileges:** Visual Studio may need to be run as **Administrator** for the "Register for COM interop" step to succeed, as it needs to write to the system registry. If the build fails with an access denied error during registration, close Visual Studio and re-open it as Administrator, then rebuild.

5.  **Output Files:**
    *   Upon successful build, the following key files will be in the output directory (e.g., `bin\x64\Release\`):
        *   `SeliwareWrapper.dll` (The COM-visible .NET assembly)
        *   `SeliwareWrapper.tlb` (The Type Library, needed by C++ clients for `#import`)

## Using SeliwareWrapper (from C++)

1.  The C++ client project will use the `#import "SeliwareWrapper.tlb"` directive to generate C++ header files for the COM interfaces.
2.  The C++ client will need to initialize COM (`CoInitialize`) and then create an instance of the `SeliwareWrapper.SeliwareWrapper` class using its CLSID or ProgID (e.g., `seliwareComPtr.CreateInstance(__uuidof(SeliwareWrapper));`).
3.  At runtime, `SeliwareWrapper.dll` and all its dependencies (`SeliwareAPI.dll`, `Newtonsoft.Json.dll`, `websocket-sharp.dll`) must be accessible to the C++ application (e.g., in the same directory as the C++ executable or in the system path).

---

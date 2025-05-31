
# TinyS - A Lightweight UI for Seliware

TinyS is a minimal, Win32-based C++ desktop UI for seliware designed to be super small and light. 
## Features

* **Extremely Small:** Only **159 KB**

  * **8.2× smaller** than normal Seliware (1,304 KB)
  * **2,949× smaller** than SynX UI (469 MB)
* **Ultra Fast Startup:** Launches in just **12ms**

  * **6,780× faster** than Seliware
  * **8,679× faster** than SynX UI
* **Efficient CPU & Memory Use:**

  * Uses on average **20× less CPU**
  * Uses on average **9× less memory**
  * Compared to Seliware UI or SynX UI
* **Win32-Based UI:** No heavy frameworks or unnecessary overhead

## Why TinyS?

This project was built to be **as tiny and fast as possible**, with the main purpose of saving system resources on **low-end PCs**. Traditional UIs like Seliware and SynX use significantly more memory and CPU, which can impact overall system performance.

## Prerequisites

*   Windows Operating System.
*   Microsoft Visual C++ Redistributable (if C++ runtime is not statically linked during build).
*   **.NET Framework 4.8** (or the version SeliwareAPI and SeliwareWrapper target) installed for Seliware functionality.
*   Seliware API files (`SeliwareAPI.dll`, `Newtonsoft.Json.dll`, `websocket-sharp.dll`).

## Building TinyS (C++ UI)

### Requirements:

*   Visual Studio (e.g., 2019, 2022) with C++ Desktop Development workload.
*   The `SeliwareWrapper.tlb` (Type Library) file generated from the `SeliwareWrapper` C# project.

### Steps:

1.  **Build `SeliwareWrapper` First:**
    *   Open the `SeliwareWrapper.sln` (or `SeliwareWrapper.csproj`) in Visual Studio.
    *   Ensure the project targets **.NET Framework 4.8** (or matching SeliwareAPI version) and **x64** platform.
    *   In Project Properties -> Build, ensure **"Register for COM interop"** is checked.
    *   Build the `SeliwareWrapper` project (preferably in Release x64 mode). This will generate `SeliwareWrapper.dll` and `SeliwareWrapper.tlb` in its output directory (e.g., `SeliwareWrapper\bin\x64\Release`).
    *   **Important:** You might need to run Visual Studio as **Administrator** for the COM registration step to succeed.

2.  **Prepare C++ Project:**
    *   Clone or download the TinyS source code.
    *   Open the `TinyS.sln` (or `TinyS.vcxproj`) in Visual Studio.
    *   **Copy `SeliwareWrapper.tlb`:** Copy the `SeliwareWrapper.tlb` file from the C# wrapper's output directory into the **main source directory of the `TinyS` C++ project** (i.e., the same folder as `main.cpp` and `resource.h`). This allows the `#import` directive in `main.cpp` to find it.

3.  **Configure C++ Project (if needed):**
    *   Ensure the project is set to build for the **x64** platform (to match the SeliwareWrapper DLL).
    *   (Optional but Recommended for smaller EXE and fewer dependencies) Static Linking C++ Runtime:
        *   Right-click `TinyS` project -> Properties.
        *   Configuration Properties -> C/C++ -> Code Generation.
        *   Runtime Library: Set to **Multi-threaded (/MT)** for Release or **Multi-threaded Debug (/MTd)** for Debug.

4.  **Build `TinyS`:**
    *   Select the desired configuration (e.g., Release, x64).
    *   Build the `TinyS` project (Build -> Build Solution).
    *   The `TinyS.exe` will be generated in the project's output directory (e.g., `x64\Release`).

## Running TinyS

1.  After building, navigate to the output directory of the `TinyS` project (e.g., `x64\Release`).
2.  **Ensure the following DLLs are in the SAME directory as `TinyS.exe`:**
    *   `SeliwareWrapper.dll` (from the C# wrapper project)
    *   `SeliwareAPI.dll`
    *   `Newtonsoft.Json.dll`
    *   `websocket-sharp.dll`
    *(You can set up a post-build event in the TinyS C++ project to automatically copy these DLLs to the output directory).*
3.  Run `TinyS.exe`.
4.  **Administrator Privileges:** Seliware API often requires administrator privileges to inject into other processes. It's recommended to run `TinyS.exe` as an administrator for Seliware features to work correctly.


---

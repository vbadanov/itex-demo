
#include <stdio.h>
#include <cstdint>

//==============================================================================
typedef uint32_t DWORD;
typedef size_t DWORD_PTR;
typedef DWORD* PDWORD;
typedef uint16_t WORD;
typedef void VOID;
typedef VOID* PVOID;
typedef void* LPVOID;
typedef char TCHAR; //wchar_t ?
typedef uint8_t BYTE;
typedef uint32_t BOOL;
typedef BOOL* PBOOL;
typedef PVOID HANDLE;

struct SYSTEM_INFO
{
  union
  {
    DWORD  dwOemId;
    struct
	{
      WORD wProcessorArchitecture;
      WORD wReserved;
    };
  };
  DWORD     dwPageSize;
  LPVOID    lpMinimumApplicationAddress;
  LPVOID    lpMaximumApplicationAddress;
  DWORD_PTR dwActiveProcessorMask;
  DWORD     dwNumberOfProcessors;
  DWORD     dwProcessorType;
  DWORD     dwAllocationGranularity;
  WORD      wProcessorLevel;
  WORD      wProcessorRevision;
};

struct OSVERSIONINFOEX
{
  DWORD dwOSVersionInfoSize;
  DWORD dwMajorVersion;
  DWORD dwMinorVersion;
  DWORD dwBuildNumber;
  DWORD dwPlatformId;
  TCHAR szCSDVersion[128];
  WORD  wServicePackMajor;
  WORD  wServicePackMinor;
  WORD  wSuiteMask;
  BYTE  wProductType;
  BYTE  wReserved;
};

//==============================================================================
extern "C" void foo(void);

extern "C" BOOL GetProductInfo(DWORD dwOSMajorVersion, DWORD dwOSMinorVersion, DWORD dwSpMajorVersion, DWORD dwSpMinorVersion, PDWORD pdwReturnedProductType);
extern "C" void GetSystemInfo(SYSTEM_INFO* sysinfo);
extern "C" int GetSystemMetrics(int val);
extern "C" BOOL GetVersionEx(OSVERSIONINFOEX* os_version);
extern "C" BOOL IsWow64Process(HANDLE hProcess, PBOOL wow64Process);

//==============================================================================
void foo(void)
{
    puts("Hello, I'm a shared library");
}

//==============================================================================
BOOL GetProductInfo(DWORD dwOSMajorVersion, DWORD dwOSMinorVersion, DWORD dwSpMajorVersion, DWORD dwSpMinorVersion, PDWORD pdwReturnedProductType)
{
	pdwReturnedProductType = new DWORD;
	return static_cast<BOOL>(true);
}

//==============================================================================
void GetSystemInfo(SYSTEM_INFO* sysinfo)
{
	sysinfo = new SYSTEM_INFO;
}

//==============================================================================
int GetSystemMetrics(int val)
{
	return 0;
}

//==============================================================================
BOOL GetVersionEx(OSVERSIONINFOEX* os_version)
{
	os_version = new OSVERSIONINFOEX;
	return static_cast<BOOL>(true);
}

//==============================================================================
BOOL IsWow64Process(HANDLE hProcess, PBOOL wow64Process)
{
	wow64Process = new BOOL;
	*wow64Process = static_cast<BOOL>(false);
	return static_cast<BOOL>(true);
}

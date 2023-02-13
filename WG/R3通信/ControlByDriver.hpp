
#include <stdio.h>
#include <Windows.h>
#include <Tlhelp32.h>
#include <Windows.h>
#include<psapi.h>
#include <WinIoCtl.h>


#define SYMBOL_NAME_LINK L"\\\\.\\papayadevice"

#define CODE_READ CTL_CODE(FILE_DEVICE_UNKNOWN,0x800,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define CODE_WRITE CTL_CODE(FILE_DEVICE_UNKNOWN,0x900,METHOD_BUFFERED,FILE_ANY_ACCESS)

class ControlByDriver {
	
private:

	DWORD GetPidByProcessName(const wchar_t* processName)
	{
		HANDLE hSnapshot;
		PROCESSENTRY32 lppe;
		//创建系统快照 
		hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL); //#include<Tlhelp32.h>
		if (hSnapshot == NULL)
			return 0;
		//初始化 lppe 的大小 
		lppe.dwSize = sizeof(lppe);
		//查找第一个进程 
		if (!Process32First(hSnapshot, &lppe))
			return 1;
		do
		{
			if (wcscmp(lppe.szExeFile, processName) == 0)//#include<shlwapi.h>
			{
				return lppe.th32ProcessID;
			}
		} while (Process32Next(hSnapshot, &lppe)); //查找下一个进程  

		return 0;
	}

	BOOLEAN OpenDevice(HANDLE* handle) {
		HANDLE _handle = CreateFile(SYMBOL_NAME_LINK, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		*handle = _handle;
		return (int)_handle > 0;
	}

	void CloseDevice(HANDLE handle) {
		CloseHandle(handle);
	}
	BOOLEAN SendCode(HANDLE handle, DWORD code, PVOID inData, ULONG inLen, PVOID outData, ULONG outLen, LPDWORD retLen) {
		return DeviceIoControl(handle, code, inData, inLen, outData, outLen, retLen, NULL);
	}

public:
	DWORD pid;

	ControlByDriver() {
		pid = 0;
	}

	ControlByDriver (const wchar_t* processName) {
		pid = GetPidByProcessName(processName);
	}
	~ControlByDriver() {
		pid = 0;
	}

	DWORD FindModuleBase(const char* moduleName) {
		HANDLE hpro = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
		if (hpro == 0) {
			printf("OpenProcess error\n");
			return 0;
		}
		DWORD pro_base = NULL;
		HMODULE hModule[300] = { 0 };
		DWORD dwRet = 0;
		int num = 0;
		int bRet = K32EnumProcessModulesEx(hpro, (HMODULE*)(hModule), sizeof(hModule), &dwRet, LIST_MODULES_32BIT);
		if (bRet == 0) {
			printf("EnumProcessModules error");
			return 0;
		}
		num = dwRet / sizeof(HMODULE);
		//printf("总模块个数: %d\n", num);
		// 打印每一个模块加载基址
		char lpBaseName[100];
		for (int i = 0; i < num; i++) {
			GetModuleBaseNameA(hpro, hModule[i], lpBaseName, sizeof(lpBaseName));
			if (strcmp(lpBaseName, moduleName) == 0) {
				//printf("%-2d %-25s基址: 0x%p\n", i, lpBaseName, hModule[i]);
				return (DWORD)hModule[i];
			}
			
		}
		return 0;
	}

	BOOLEAN ReadN(int address, int readLen, PVOID result) {
		HANDLE deviceHandle;
		if (!OpenDevice(&deviceHandle)) {
			printf("open device error\n");
			return FALSE;
		}

		char buf[0x1000] = { 0 };
		DWORD len = 0;

		int* int_ptr = (int*)&buf;
		*(int_ptr + 0) = pid;
		*(int_ptr + 1) = address;
		*(int_ptr + 2) = readLen;
		SendCode(deviceHandle, CODE_WRITE, buf, 0x1000, buf, 0x1000, &len); // 写入给驱动 Handle address datalen 信息
		Sleep(100);
		SendCode(deviceHandle, CODE_READ, buf, 0x1000, buf, 0x1000, &len); // 读 驱动去读取得到的第三方进程的数据
		CloseDevice(deviceHandle);
		memcpy((char*)result, buf, readLen);
		return TRUE;
	}



};


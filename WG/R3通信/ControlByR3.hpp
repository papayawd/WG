
#include <stdio.h>
#include <psapi.h>
#include <Windows.h>
#include <Tlhelp32.h>



class ControlByR3 {

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

public:
	DWORD pid;

	ControlByR3() {
		pid = 0;
	}

	ControlByR3(const wchar_t* processName) {
		pid = GetPidByProcessName(processName);
	}
	~ControlByR3() {
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
		CloseHandle(hpro);
		return 0;
	}

	BOOLEAN ReadN(int address, int readLen, PVOID result) {


		SIZE_T len = 0;

		HANDLE hpro = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
		if (hpro == 0) {
			printf("OpenProcess error\n");
			return FALSE;
		}

		if (ReadProcessMemory(hpro, (LPCVOID)address, result, readLen, (SIZE_T*)&len) == 0) {
			printf("ReadProcessMemory error\n");
			return FALSE;
		}
		
		CloseHandle(hpro);
		return TRUE;
	}



};


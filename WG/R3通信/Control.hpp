#include<psapi.h>

class Control {

private:

	DWORD GetPidByProcessName(const wchar_t* processName)
	{
		HANDLE hSnapshot;
		PROCESSENTRY32 lppe;
		//����ϵͳ���� 
		hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL); //#include<Tlhelp32.h>
		if (hSnapshot == NULL)
			return 0;
		//��ʼ�� lppe �Ĵ�С 
		lppe.dwSize = sizeof(lppe);
		//���ҵ�һ������ 
		if (!Process32First(hSnapshot, &lppe))
			return 1;
		do
		{
			if (wcscmp(lppe.szExeFile, processName) == 0)//#include<shlwapi.h>
			{
				return lppe.th32ProcessID;
			}
		} while (Process32Next(hSnapshot, &lppe)); //������һ������  

		return 0;
	}

	

public:
	DWORD pid;

	ControlByDriver() {
		pid = 0;
	}

	ControlByDriver(const wchar_t* processName) {
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
		//printf("��ģ�����: %d\n", num);
		// ��ӡÿһ��ģ����ػ�ַ
		char lpBaseName[100];
		for (int i = 0; i < num; i++) {
			GetModuleBaseNameA(hpro, hModule[i], lpBaseName, sizeof(lpBaseName));
			if (strcmp(lpBaseName, moduleName) == 0) {
				//printf("%-2d %-25s��ַ: 0x%p\n", i, lpBaseName, hModule[i]);
				return (DWORD)hModule[i];
			}

		}
		return 0;
	}

	BOOLEAN ReadN(int address, int readLen, PVOID result) {

		char buf[0x1000] = { 0 };
		DWORD len = 0;

		
		memcpy((char*)result, buf, readLen);
		return TRUE;
	}



};


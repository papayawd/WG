
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <Tlhelp32.h>
#include <math.h>
//#include "ControlByDriver.hpp"
#include "ControlByR3.hpp"
#include "D3D����.h"





D3Ddraw d3dDraw = NULL;
int �ߴ� = 2;
D3DCOLOR ��ɫ = D3DCOLOR_ARGB(255, 255, 255, 0);
int cnt = 0;
float places[20 * 9];
float mouse[2]; // mouse[0] �����Y�Ƕ�     mouse[1]�����x�Ƕ�
double pi = acos(-1.0);
//ControlByDriver control = ControlByDriver(L"csgo.exe");
ControlByR3 control = ControlByR3(L"csgo.exe");
DWORD clientDllBase;
DWORD serverDllBase;
float g_Matrix[4][4];





void ����()
{
	// d3dDraw.����(100, 100, 100, 100, �ߴ�, D3DCOLOR_ARGB(255, 255, 255, 0));
	
	// ��ȡȫ��������λ����Ϣ
	control.ReadN(serverDllBase + 0xB0B16C, 20 * 9 * sizeof(float), &places);
	control.ReadN(clientDllBase + 0x4DF0E34, 4 * 4 * sizeof(float), &g_Matrix);

	d3dDraw.���ƿ�ʼ();

	// �Լ�������
	double X, Y, Z;
	X = places[0], Y = places[1], Z = places[2];
	

	for (int i = 1; i<10; i++) {
		double x, y, z;
		x = places[i * 9], y = places[i * 9 + 1], z = places[i * 9 + 2];
		double dx = X - x, dy = Y - y, dz = Z - z;
		double dis2D = sqrt(pow(dx, 2) + pow(dy, 2));
		double dis3D = sqrt(pow(dx, 2) + pow(dy, 2) + pow(dz, 2));

		float pl[2] = { 0 };
		
		if (d3dDraw.WordToScreen(g_Matrix,&places[i * 9],pl)) {
			//printf("huahua\n");
			float rect[4] = { 0 };
			d3dDraw.MakeRect(rect, pl[0], pl[1], dis3D);
			d3dDraw.����(rect[0], rect[1], rect[2], rect[3], �ߴ�, ��ɫ);
		}
		d3dDraw.����(100, 100, 10, 10, �ߴ�, ��ɫ);
	}
	
	d3dDraw.���ƽ���();
}

void ��ʼ()
{
	d3dDraw.����͸������(����);
	d3dDraw.������Ϣѭ��();
}

int main() {

	/*
	* �ֱ���������1920*1080 ���ȵ����Էֱ��� Ȼ�󿪴���ģʽ
	* Ŀǰʹ�õ���ȫ��λ����Ϣ��û�����ֵ��� û���ж�Ѫ�� Ĭ��10�� 
	* ����һ�����ͨ��Ѫ���ж�������Ҫ��Ҫ���򣬲�д�˰��ã�
	* 
	*/

	
	
	int result;
	printf("pid : %d\n", control.pid);
	if (control.pid == 0) {
		printf("can't find process name\n");
		return 0;
	}
	// ��ȡdll����ַ
	clientDllBase = control.FindModuleBase("client.dll");
	serverDllBase = control.FindModuleBase("server.dll");


	
	Sleep(3000);
	d3dDraw = D3Ddraw((HWND)FindWindow(NULL, L"Counter-Strike: Global Offensive - Direct3D 9"));

	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)��ʼ, NULL, 0, NULL);

	while (1)
	{
		cout << "����233�ر�:" << endl;
		int a = 0;
		cin >> a;
		if (a == 233)
		{
			return 0;
		}
	}

	

	/*
	// ��ȡ�������� ��ӡ������Ϣ
	DWORD listHead = clientDllBase + 0x4DFFEF4;
	DWORD res = 0;
	control.ReadN(listHead, sizeof(int), &res);
	for(int i = 0;res != 0;i++){ // ��һ�����Լ�����Ϣ
		DWORD ptr = res;

		float d[3];
		control.ReadN(ptr + 0xA0, sizeof(float)*3, &d);

		int hp,group = 0;
		control.ReadN(ptr + 0x100, sizeof(int), &hp);
		control.ReadN(ptr + 0xF4, sizeof(char), &group);
		printf("-----------------------\n");
		printf("id = %d\nx = %f,y = %f,z = %f\nhp = %d\ngroup = %s\n", i,d[0],d[1],d[2],hp,group==2?"T":"CT");

		listHead += 0x10;
		control.ReadN(listHead, sizeof(int), &res);
	}
	*/

	

	system("pause");
	return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <Tlhelp32.h>
#include <math.h>
//#include "ControlByDriver.hpp"
#include "ControlByR3.hpp"
#include "D3D绘制.h"





D3Ddraw d3dDraw = NULL;
int 线粗 = 2;
D3DCOLOR 红色 = D3DCOLOR_ARGB(255, 255, 255, 0);
int cnt = 0;
float places[20 * 9];
float mouse[2]; // mouse[0] 是鼠标Y角度     mouse[1]是鼠标x角度
double pi = acos(-1.0);
//ControlByDriver control = ControlByDriver(L"csgo.exe");
ControlByR3 control = ControlByR3(L"csgo.exe");
DWORD clientDllBase;
DWORD serverDllBase;
float g_Matrix[4][4];





void 绘制()
{
	// d3dDraw.画框(100, 100, 100, 100, 线粗, D3DCOLOR_ARGB(255, 255, 255, 0));
	
	// 读取全局所有人位置信息
	control.ReadN(serverDllBase + 0xB0B16C, 20 * 9 * sizeof(float), &places);
	control.ReadN(clientDllBase + 0x4DF0E34, 4 * 4 * sizeof(float), &g_Matrix);

	d3dDraw.绘制开始();

	// 自己的坐标
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
			d3dDraw.画框(rect[0], rect[1], rect[2], rect[3], 线粗, 红色);
		}
		d3dDraw.画框(100, 100, 10, 10, 线粗, 红色);
	}
	
	d3dDraw.绘制结束();
}

void 开始()
{
	d3dDraw.创建透明窗口(绘制);
	d3dDraw.窗口消息循环();
}

int main() {

	/*
	* 分辨率适用于1920*1080 首先调电脑分辨率 然后开窗口模式
	* 目前使用的是全局位置信息，没有区分敌我 没有判断血量 默认10人 
	* 精致一点可以通过血量判断来决定要不要画框，不写了摆烂！
	* 
	*/

	
	
	int result;
	printf("pid : %d\n", control.pid);
	if (control.pid == 0) {
		printf("can't find process name\n");
		return 0;
	}
	// 获取dll基地址
	clientDllBase = control.FindModuleBase("client.dll");
	serverDllBase = control.FindModuleBase("server.dll");


	
	Sleep(3000);
	d3dDraw = D3Ddraw((HWND)FindWindow(NULL, L"Counter-Strike: Global Offensive - Direct3D 9"));

	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)开始, NULL, 0, NULL);

	while (1)
	{
		cout << "输入233关闭:" << endl;
		int a = 0;
		cin >> a;
		if (a == 233)
		{
			return 0;
		}
	}

	

	/*
	// 获取人物链表 打印人物信息
	DWORD listHead = clientDllBase + 0x4DFFEF4;
	DWORD res = 0;
	control.ReadN(listHead, sizeof(int), &res);
	for(int i = 0;res != 0;i++){ // 第一个是自己的信息
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
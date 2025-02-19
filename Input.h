#include<Windows.h>
#include<iostream>
#define DIRECTINPUT_VERSION 0x0800

#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")
class Input
{
public:
	static const int KEY_MAX = 256;
	//Input Class Initialize
	Input(HWND hWnd);

	bool GetKey(UINT KeyCode);
	~Input();


};

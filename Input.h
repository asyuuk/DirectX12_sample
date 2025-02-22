#include<iostream>
#include<dinput.h>
#include<Windows.h>
#include<Xinput.h>


#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"xinput.lib")
class Window;

class Input
{
public:
	Input(HWND hwnd);
	~Input();
	bool CheckKey(UINT index);
	bool TriggerKey(UINT index);

	float InputPad_X();
	float InputPad_Y();
	bool CheckPad();
	bool InputA();
	bool InputB();
	bool InputX();
	bool InputY();
	bool InputL();
	bool InputR();


	static const int KEY_MAX = 256;

	float speed;

	HWND hWnd;

	LPDIRECTINPUT8       input     = nullptr;
	LPDIRECTINPUTDEVICE8 key       = nullptr;
	BYTE keys[KEY_MAX];
	BYTE olds[KEY_MAX];

	DWORD dwUserIndex;

	_XINPUT_GAMEPAD gamepad;

	int Pad_L;
	int Pad_R;
	int Pad_U;
	int Pad_D;
	int Pad_A;
	int Pad_B;
	int Pad_X;
	int Pad_Y;

	float strokeX;
	float strokeY;
	
};

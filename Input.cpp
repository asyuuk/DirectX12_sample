#include "Input.h"

#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")

Input::Input(HWND hwnd)
{
	
	memset(&keys, 0, sizeof(keys));
	memset(&olds, 0, sizeof(olds));

	
	CheckPad();

    auto result = DirectInput8Create(GetModuleHandle(0), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)(&input), NULL);

	result = input->CreateDevice(GUID_SysKeyboard, &key, NULL);
	if (FAILED(result))
	{
		MessageBox(0, TEXT("�L�[�{�[�h�̐����Ɏ��s���܂���"), TEXT("�G���["), MB_OK);
	}

	result = key->SetDataFormat(&c_dfDIKeyboard);

	result = key->SetCooperativeLevel(hWnd, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);

	key->Acquire();

	CheckPad();
}

Input::~Input()
{
	key->Unacquire();
	key->Release();
	input->Release();


}

bool Input::CheckPad()
{
	DWORD dwResult;
	for (DWORD i = 0; i < XUSER_MAX_COUNT; i++)
	{
		XINPUT_STATE state;
		ZeroMemory(&state, sizeof(XINPUT_STATE));
		dwResult = XInputGetState(i, &state);
		if (dwResult == ERROR_SUCCESS)
		{
			std::cerr << "�R���g���[���[���ڑ�����Ă��܂�" << std::endl;

			return true;
		}
		else
		{
			std::cerr << "�R���g���[���[���ڑ�����Ă��܂���" << std::endl;
			return false;
		}
	}

}

bool Input::InputA()
{
	DWORD dwResult;
	XINPUT_STATE state;
	XInputGetState(0, &state);
	if (state.Gamepad.wButtons & XINPUT_GAMEPAD_A)
	{
		std::cerr << "A�{�^����������Ă��܂�" << std::endl;

		return true;
	}
	else
	{
		std::cerr << "A�{�^����������Ă��܂���" << std::endl;
		return false;
	}
}

bool Input::InputB()
{
	DWORD dwResult;
	XINPUT_STATE state;
	XInputGetState(0, &state);
	if (state.Gamepad.wButtons & XINPUT_GAMEPAD_B)
	{
		std::cerr << "B�{�^����������Ă��܂�" << std::endl;
		return true;
	}
	else
	{
		std::cerr << "B�{�^����������Ă��܂���" << std::endl;
		return false;
	}
}
bool Input::InputX()
{
	DWORD dwResult;
	XINPUT_STATE state;
	XInputGetState(0, &state);
	if (state.Gamepad.wButtons & XINPUT_GAMEPAD_X)
	{
		std::cerr << "X�{�^����������Ă��܂�" << std::endl;
		return true;
	}
	else
	{
		std::cerr << "X�{�^����������Ă��܂���" << std::endl;
		return false;
	}
}
bool Input::InputY()
{
	DWORD dwResult;
	XINPUT_STATE state;
	XInputGetState(0, &state);
	if (state.Gamepad.wButtons & XINPUT_GAMEPAD_Y)
	{
		std::cerr << "Y�{�^����������Ă��܂�" << std::endl;
		return true;
	}
	else
	{
		std::cerr << "Y�{�^����������Ă��܂���" << std::endl;
		return false;
	}
}

bool Input::InputL()
{
	DWORD dwResult;
	XINPUT_STATE state;
	XInputGetState(0, &state);
	if (state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER)
	{
		std::cerr << "L�{�^����������Ă��܂�" << std::endl;
		return true;
	}
	else
	{
		std::cerr << "L�{�^����������Ă��܂���" << std::endl;
		return false;
	}
}

bool Input::InputR()
{
	DWORD dwResult;
	XINPUT_STATE state;
	XInputGetState(0, &state);
	if (state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER)
	{
		std::cerr << "R�{�^����������Ă��܂�" << std::endl;
		return true;
	}
	else
	{
		std::cerr << "R�{�^����������Ă��܂���" << std::endl;
		return false;
	}
}

//�C���v�b�g�̐���
float Input::InputPad_X()
{
	XINPUT_STATE state;
	XINPUT_KEYSTROKE gamepad;
	XInputGetState(0, &state);
	

	//�f�b�h�]�[��
	if (state.Gamepad.sThumbLX < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE &&
		state.Gamepad.sThumbLX > -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
	{
		state.Gamepad.sThumbLX = 0;
	}
	if (state.Gamepad.sThumbLY < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE &&
		state.Gamepad.sThumbLY > -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
	{
		state.Gamepad.sThumbLY = 0;
	}

	if (state.Gamepad.sThumbRX < XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE &&
		state.Gamepad.sThumbRX > -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
	{
		state.Gamepad.sThumbRX = 0;
	}
	if (state.Gamepad.sThumbRY < XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE &&
		state.Gamepad.sThumbRY > -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
	{
		state.Gamepad.sThumbRY = 0;
	}


	//�X�e�B�b�N�̓���
	if (state.Gamepad.sThumbLX  >= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
	{
		strokeX = 0.01f;
	}

	if (state.Gamepad.sThumbLX <= -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
	{
		strokeX = -0.01f;
	}

	

	if(state.Gamepad.sThumbRX >= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
	{
		strokeX = 0.01f;
	}
	if (state.Gamepad.sThumbRX <= -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
	{
		strokeX = -0.01f;
	}
	

	return strokeX;
}

float Input::InputPad_Y()
{
	XINPUT_STATE state;
	XINPUT_KEYSTROKE gamepad;
	XInputGetState(0, &state);

	//�f�b�h�]�[��
	if (state.Gamepad.sThumbLX < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE &&
		state.Gamepad.sThumbLX > -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
	{
		state.Gamepad.sThumbLX = 0;
	}
	if (state.Gamepad.sThumbLY < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE &&
		state.Gamepad.sThumbLY > -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
	{
		state.Gamepad.sThumbLY = 0;
	}

	if (state.Gamepad.sThumbRX < XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE &&
		state.Gamepad.sThumbRX > -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
	{
		state.Gamepad.sThumbRX = 0;
	}
	if (state.Gamepad.sThumbRY < XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE &&
		state.Gamepad.sThumbRY > -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
	{
		state.Gamepad.sThumbRY = 0;
	}


	if (state.Gamepad.sThumbLY >= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
	{
		strokeY = 0.01f;
	}

	if (state.Gamepad.sThumbLY <= -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
	{
		strokeY = -0.01f;
	}


	if (state.Gamepad.sThumbRY >= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
	{
		strokeY = 0.01f;
	}
	if (state.Gamepad.sThumbRY <= -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
	{
		strokeY = -0.01f;
	}

	return strokeY;
}

//�L�[����
bool Input::CheckKey(UINT index)
{
	bool flag = false;

	//�L�[�����擾
	key->GetDeviceState(sizeof(key), &keys);

	if (keys[index] & 0x80)
	{
		flag = true;
	}
	olds[index] = keys[index];

	return flag;
}



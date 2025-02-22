#pragma once
#include<d3d12.h>
#include<dxgi1_4.h>
#include<DirectXMath.h>
#include<wrl/client.h>
#include<Windows.h>
#include<WICTextureLoader.h>
#include<d3dcompiler.h>
#include<vector>
#include<string>
#include<cstdint>
#include<d3dcompiler.h>
#include<VertexTypes.h>
#include<vector>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib,"dxguid.lib")



template<class T>
using ComPtr = Microsoft::WRL::ComPtr<T>;



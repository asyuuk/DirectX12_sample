#include "RenderManager.h"
#include"ResourceUploadBatch.h"



RenderManager::RenderManager(uint32_t width, uint32_t height) :m_hInst(nullptr), m_hWnd(nullptr), m_Width(width), m_Height(height), m_FrameIndex(0), m_CurrentFrame(0), m_Rotate(0.0f), x(0.0f), y(0.0f)
{
	for (auto i = 0; i < FrameCount; ++i)
	{
		m_pColorBuffer[i] = nullptr;
		m_CommandAllocator[i] = nullptr;
		m_FenceCounter[i] = 0;
	}
	input = std::make_unique<Input>(m_hWnd);
}
RenderManager::~RenderManager()
{

}



void RenderManager::Run()
{
	if (InitApp()) {
		MainLoop();
	}
	TermApp();
}

bool RenderManager::InitApp()
{
	if (!InitD3D12())
	{
		return false;
	}
	if (!InitImage())
	{
		return false;
	}

	

	return true;
}

bool RenderManager::InitD3D12()
{
	if (!InitWindow())
	{
		return false;
	}
	if (!InitD3D())
	{
		return false;
	}
	return true;
}

bool RenderManager::InitImage()
{
	
	if (!Load())
	{
		return false;
	}

	if (!HeapRTV_SRV_UAV())
	{
		return false;
	}

	if (!ConstantBufferViews())
	{
		return false;
	}

	
	if (!RootSignature())
	{
		return false;
	}
	if (!Pipelinestate())
	{
		return false;
	}
	if (!DivImages())
	{
		return false;
	}
	
	return true;
}

 bool RenderManager::DivImages()
{
	if (!CreateTextureFromPng(L"E:/clip studio/pngフォルダ　自分用　高画質/02.png",1))
	{
		return false;
	}
	ViewPortScissor();
	return true;
}

 bool RenderManager::Images()
 {
	 if (!LoadImages(L"E:/clip studio/pngフォルダ　自分用　高画質/02.png", 0))
	 {
		 return false;
	 }
	 ViewPortScissor();
	 return true;
 }

void RenderManager::TermApp()
{
	OnTerm();
	TermD3D();
	TermWnd();
}

void RenderManager::ViewPortScissor()
{
	
	m_Viewport.TopLeftX = 0;
	m_Viewport.TopLeftY = 0;
	m_Viewport.Width = static_cast<float>(m_Width);
	m_Viewport.Height = static_cast<float>(m_Height);
	m_Viewport.MinDepth = 0.0f;
	m_Viewport.MaxDepth = 1.0f;

	m_Scissor.left = 0;
	m_Scissor.right = m_Width;
	m_Scissor.top = 0;
	m_Scissor.bottom = m_Height;
	
}

bool RenderManager::InitWindow()
{
	auto hInst = GetModuleHandle(nullptr);
	if (hInst == nullptr)
	{
		return false;
	}
	WNDCLASSEX wc = {};
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.hIcon = LoadIcon(hInst, IDI_APPLICATION);
	wc.hCursor = LoadCursor(hInst, IDC_ARROW);
	wc.hbrBackground = GetSysColorBrush(COLOR_BACKGROUND);
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = ClassName;
	wc.hIconSm = LoadIcon(hInst, IDI_APPLICATION);

	if (!RegisterClassEx(&wc))
	{
		return false;
	}
	m_hInst = hInst;

	RECT rc = {};
	rc.right = static_cast<LONG>(m_Width);
	rc.bottom = static_cast<LONG>(m_Height);

	auto style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
	AdjustWindowRect(&rc, style, FALSE);

	m_hWnd = CreateWindowEx
	(
		0,
		ClassName,
		TEXT("Sample"),
		style,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rc.right - rc.left,
		rc.bottom - rc.top,
		nullptr,
		nullptr,
		m_hInst,
		nullptr
	);
	if (m_hInst == nullptr)
	{
		return false;
	}
	ShowWindow(m_hWnd, SW_SHOWNORMAL);
	UpdateWindow(m_hWnd);
	SetFocus(m_hWnd);
	return true;

}

bool RenderManager::InitD3D()
{
#if defined(DEBUG)||defined(_DEBUG)
	{
		ComPtr<ID3D12Debug>debug;
		auto hr = D3D12GetDebugInterface(IID_PPV_ARGS(debug.GetAddressOf()));
		if (SUCCEEDED(hr))
		{
			debug->EnableDebugLayer();
		}
	}
#endif

//デバイスの生成
	auto hr = D3D12CreateDevice(
		nullptr,
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(m_device.GetAddressOf())

	);
	if (FAILED(hr))
	{
		return false;
	}

	//コマンドキューの生成
	{
		D3D12_COMMAND_QUEUE_DESC desc = {};
		desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		desc.NodeMask = 0;

		hr = m_device->CreateCommandQueue(&desc, IID_PPV_ARGS(m_pQueue.GetAddressOf()));
		if (FAILED(hr))
		{
			return false;
		}

	}
	//スワップチェインの生成
	{
		//DXGIファクトリーを生成
		ComPtr<IDXGIFactory4>pFactory = nullptr;
		hr = CreateDXGIFactory1(IID_PPV_ARGS(pFactory.GetAddressOf()));

		if (FAILED(hr))
		{
			return false;
		}
		DXGI_SWAP_CHAIN_DESC desc = {};
		desc.BufferDesc.Width = m_Width;
		desc.BufferDesc.Height = m_Height;
		desc.BufferDesc.RefreshRate.Numerator = 60;
		desc.BufferDesc.RefreshRate.Denominator = 1;
		desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.BufferCount = FrameCount;
		desc.OutputWindow = m_hWnd;
		desc.Windowed = TRUE;
		desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		ComPtr<IDXGISwapChain>pSwapChain;
		hr = pFactory->CreateSwapChain(m_pQueue.Get(), &desc, pSwapChain.GetAddressOf());
		if (FAILED(hr))
		{
			return false;
		}
		hr = pSwapChain.As(&m_swapchain);
		if (FAILED(hr))
		{
			return false;
		}
		m_FrameIndex = m_swapchain->GetCurrentBackBufferIndex();
		pFactory.Reset();
		pSwapChain.Reset();

	}

	//コマンドアロケータを生成
	{
		for (auto i = 0u; i < FrameCount; ++i)
		{
			hr = m_device->CreateCommandAllocator(
				D3D12_COMMAND_LIST_TYPE_DIRECT,
				IID_PPV_ARGS(m_CommandAllocator[i].GetAddressOf())
			);
			if (FAILED(hr))
			{
				return false;
			}

		}
	}
	//コマンドリストを生成
	{
		hr = m_device->CreateCommandList(
			0,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			m_CommandAllocator[m_FrameIndex].Get(),
			nullptr,
			IID_PPV_ARGS(m_CommandList.GetAddressOf())
		);
		if (FAILED(hr))
		{
			return false;
		}

	}

	//レンダーターゲットビューの生成
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};	
		desc.NumDescriptors = FrameCount;
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		desc.NodeMask = 0;

		//ディスクリプタヒープの生成
		hr = m_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(m_pHeapRTV.GetAddressOf()));
		if (FAILED(hr))
		{
			return false;
		}
		auto handle = m_pHeapRTV->GetCPUDescriptorHandleForHeapStart();
		auto incrementSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		for (auto i = 0u; i < FrameCount; ++i)
		{
			hr = m_swapchain->GetBuffer(i, IID_PPV_ARGS(m_pColorBuffer[i].GetAddressOf()));
			if (FAILED(hr))
			{
				return false;
			}
			D3D12_RENDER_TARGET_VIEW_DESC desc = {};
			desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
			desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			desc.Texture2D.MipSlice = 0;
			desc.Texture2D.PlaneSlice = 0;
			m_device->CreateRenderTargetView(m_pColorBuffer[i].Get(), &desc, handle);
			m_HandleRTV[i] = handle;
			handle.ptr += incrementSize;
		}
	}

	//フェンスの生成
	{
		for (auto i = 0u; i < FrameCount; ++i)
		{
			m_FenceCounter[i] = 0;
		}
		hr = m_device->CreateFence(
			m_FenceCounter[m_FrameIndex],
			D3D12_FENCE_FLAG_NONE,
			IID_PPV_ARGS(m_fence.GetAddressOf())
		);
		if(FAILED(hr))
		{
			return false;
		}
		m_FenceCounter[m_FrameIndex]++;
		//イベントの生成
		m_FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if(m_FenceEvent==nullptr)
		{
			return false;
		}
	}

	
	m_CommandList->Close();

	return true;
}

bool RenderManager::VertexBuffer_05()
{

	//頂点データ

	DirectX::VertexPositionTexture vertices_05[] = {
			DirectX::VertexPositionTexture(DirectX::XMFLOAT3(-0.5f,  0.5f, 0.0f), DirectX::XMFLOAT2(0.0f, 0.0f)),
			DirectX::VertexPositionTexture(DirectX::XMFLOAT3(0.5f,  0.5f, 0.0f), DirectX::XMFLOAT2(0.5f, 0.0f)),
			DirectX::VertexPositionTexture(DirectX::XMFLOAT3(0.5f, -0.5f, 0.0f), DirectX::XMFLOAT2(0.5f, 0.5f)),
			DirectX::VertexPositionTexture(DirectX::XMFLOAT3(-0.5f, -0.5f, 0.0f), DirectX::XMFLOAT2(0.0f, 0.5f))
	};

	//ヒーププロパティ
	D3D12_HEAP_PROPERTIES prop = {};
	prop.Type = D3D12_HEAP_TYPE_UPLOAD;
	prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	prop.CreationNodeMask = 1;
	prop.VisibleNodeMask = 1;
	//リソースの設定
	D3D12_RESOURCE_DESC desc = {};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.Alignment = 0;
	desc.Width = sizeof(vertices_05);
	desc.Height = 1;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	desc.Flags = D3D12_RESOURCE_FLAG_NONE;


	auto hr = m_device->CreateCommittedResource(
		&prop,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(m_pVB.GetAddressOf())
	);
	if (FAILED(hr))
	{
		return false;
	}
	//マッピング
	void* ptr = nullptr;
	hr = m_pVB->Map(0, nullptr, &ptr);
	if (FAILED(hr))
	{
		return false;
	}
	memcpy(ptr, vertices_05, sizeof(vertices_05));

	m_pVB->Unmap(0, nullptr);

	m_VBV.BufferLocation = m_pVB->GetGPUVirtualAddress();
	m_VBV.SizeInBytes = static_cast<UINT>(sizeof(vertices_05));
	m_VBV.StrideInBytes = static_cast<UINT>(sizeof(DirectX::VertexPositionTexture));

	return true;
}

bool RenderManager::VertexBuffer_10()
{
	//頂点データ

	DirectX::VertexPositionTexture vertices[] = {
		DirectX::VertexPositionTexture(DirectX::XMFLOAT3(-1.0f,  1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 0.0f)),
		DirectX::VertexPositionTexture(DirectX::XMFLOAT3(1.0f,  1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 0.0f)),
		DirectX::VertexPositionTexture(DirectX::XMFLOAT3(1.0f, -1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f)),
		DirectX::VertexPositionTexture(DirectX::XMFLOAT3(-1.0f, -1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 1.0f))
	};

	//ヒーププロパティ
	D3D12_HEAP_PROPERTIES prop = {};
	prop.Type = D3D12_HEAP_TYPE_UPLOAD;
	prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	prop.CreationNodeMask = 1;
	prop.VisibleNodeMask = 1;
	//リソースの設定
	D3D12_RESOURCE_DESC desc = {};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.Alignment = 0;
	desc.Width = sizeof(vertices);
	desc.Height = 1;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	desc.Flags = D3D12_RESOURCE_FLAG_NONE;


	auto hr = m_device->CreateCommittedResource(
		&prop,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(m_pVB.GetAddressOf())
	);
	if (FAILED(hr))
	{
		return false;
	}
	//マッピング
	void* ptr = nullptr;
	hr = m_pVB->Map(0, nullptr, &ptr);
	if (FAILED(hr))
	{
		return false;
	}
	memcpy(ptr, vertices, sizeof(vertices));

	m_pVB->Unmap(0, nullptr);

	m_VBV.BufferLocation = m_pVB->GetGPUVirtualAddress();
	m_VBV.SizeInBytes = static_cast<UINT>(sizeof(vertices));
	m_VBV.StrideInBytes = static_cast<UINT>(sizeof(DirectX::VertexPositionTexture));
	return true;
}

bool RenderManager::LoadDivImages(const wchar_t* filename, int rows, int cols)
{
	
	if (!LoadImages(filename, 6))
	{
		return false;
	}
	if (!DivVertexs(rows, cols))
	{
		return false;
	}
	ViewPortScissor();
	return true;
}

bool RenderManager::ConstantBufferViews()
{
	//ヒーププロパティ
	D3D12_HEAP_PROPERTIES prop = {};
	prop.Type = D3D12_HEAP_TYPE_UPLOAD;
	prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	prop.CreationNodeMask = 1;
	prop.VisibleNodeMask = 1;

	//リソースの設定
	D3D12_RESOURCE_DESC desc = {};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.Alignment = 0;
	desc.Width = sizeof(Transform);
	desc.Height = 1;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	desc.Flags = D3D12_RESOURCE_FLAG_NONE;

	auto incrementSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);


	for (auto i = 0; i < FrameCount; ++i)
	{
		for (int j = 0; j < 3; j++)
		{
			auto hr = m_device->CreateCommittedResource(
				&prop,
				D3D12_HEAP_FLAG_NONE,
				&desc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(m_pCB[i].GetAddressOf())
			);
			if (FAILED(hr))
			{
				return false;
			}
			auto address = m_pCB[i]->GetGPUVirtualAddress();
			auto handleCPU = m_pHeapCBV_SRV_UAV->GetCPUDescriptorHandleForHeapStart();
			auto handleGPU = m_pHeapCBV_SRV_UAV->GetGPUDescriptorHandleForHeapStart();

			handleCPU.ptr += incrementSize * i;
			handleGPU.ptr += incrementSize * i;

			m_CBV[i][j].HandleCPU = handleCPU;
			m_CBV[i][j].HandleGPU = handleGPU;
			m_CBV[i][j].Desc.BufferLocation = address;
			m_CBV[i][j].Desc.SizeInBytes = sizeof(Transform);

			m_device->CreateConstantBufferView(&m_CBV[i][j].Desc, handleCPU);

			hr = m_pCB[i]->Map(0, nullptr, reinterpret_cast<void**>(&m_CBV[i][j].pBuffer));
			if (FAILED(hr))
			{

				return false;
			}

			auto eyePos = DirectX::XMVectorSet(0.0f, 0.0f, 6.0f, 0.0f);
			auto targetPos = DirectX::XMVectorZero();
			auto upward = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

			auto fovY = DirectX::XMConvertToRadians(37.5);
			auto aspect = static_cast<float>(m_Width) / static_cast<float>(m_Height);

			m_CBV[i][j].pBuffer->World = DirectX::XMMatrixIdentity();
			m_CBV[i][j].pBuffer->View = DirectX::XMMatrixLookAtRH(eyePos, targetPos, upward);
			m_CBV[i][j].pBuffer->Proj = DirectX::XMMatrixPerspectiveFovRH(fovY, aspect, 1.0f, 1000.0f);
		}
	}

	return true;


}

bool RenderManager::HeapRTV_SRV_UAV()
{
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	desc.NumDescriptors = m_HeapSRVPoolMax;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NodeMask = 0;

	auto hr = m_device->CreateDescriptorHeap(
		&desc,
		IID_PPV_ARGS(m_pHeapCBV_SRV_UAV.GetAddressOf())
	);
	if (FAILED(hr))
	{
		return false;
	}
	for (int i = 0; i < m_HeapSRVPoolMax; i++)
	{
		m_HeapSRVPool.push_back(i);
	}
	return true;
}

bool RenderManager::Load()
{
	//頂点バッファの生成
	{
		VertexBuffer_10();
		
	}
	//インデックスバッファの生成
	{
		uint32_t indices[] = { 0,1,2,0,2,3 };
		//ヒーププロパティ
		D3D12_HEAP_PROPERTIES prop = {};
		prop.Type = D3D12_HEAP_TYPE_UPLOAD;
		prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		prop.CreationNodeMask = 1;
		prop.VisibleNodeMask = 1;

		//リソースの設定
		D3D12_RESOURCE_DESC desc = {};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Alignment = 0;
		desc.Width = sizeof(indices);
		desc.Height = 1;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		desc.Flags = D3D12_RESOURCE_FLAG_NONE;

		auto hr = m_device->CreateCommittedResource(
			&prop,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(m_pIB.GetAddressOf())
		);
		//マッピングする
		void* ptr = nullptr;
		hr = m_pIB->Map(0, nullptr, &ptr);
		if (FAILED(hr))
		{
			return false;
		}
		memcpy(ptr, indices, sizeof(indices));

		m_pIB->Unmap(0, nullptr);

		m_IBV.BufferLocation = m_pIB->GetGPUVirtualAddress();
		m_IBV.Format = DXGI_FORMAT_R32_UINT;
		m_IBV.SizeInBytes = sizeof(indices);

	}

	
	
	return true;
}



bool RenderManager::RootSignature()
{
	auto flag = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
	flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
	flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

	//ルートパラメーター
	D3D12_ROOT_PARAMETER param[2] = {};
	param[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	param[0].Descriptor.ShaderRegister = 0;
	param[0].Descriptor.RegisterSpace = 0;
	param[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	D3D12_DESCRIPTOR_RANGE range = {};
	range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	range.NumDescriptors = 2;
	range.BaseShaderRegister = 0;
	range.RegisterSpace = 0;
	range.OffsetInDescriptorsFromTableStart = 0;

	param[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	param[1].DescriptorTable.NumDescriptorRanges = 1;
	param[1].DescriptorTable.pDescriptorRanges = &range;
	param[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	

	//スタティックサンプラー
	D3D12_STATIC_SAMPLER_DESC sampler = {};

	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.MipLODBias = D3D12_DEFAULT_MIP_LOD_BIAS;
	sampler.MaxAnisotropy = 1;
	sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
	sampler.MinLOD = -D3D12_FLOAT32_MAX;
	sampler.MaxLOD = +D3D12_FLOAT32_MAX;
	sampler.ShaderRegister = 0;
	sampler.RegisterSpace = 0;
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_DESC desc = {};
	desc.NumParameters = 2;
	desc.NumStaticSamplers = 1;
	desc.pParameters = param;
	desc.pStaticSamplers = &sampler;
	desc.Flags = flag;

	ComPtr<ID3DBlob> pBlob;
	ComPtr<ID3DBlob>errorBlob;

	auto hr = D3D12SerializeRootSignature(
		&desc,
		D3D_ROOT_SIGNATURE_VERSION_1_0,
		pBlob.GetAddressOf(),
		errorBlob.GetAddressOf()
	);
	if (FAILED(hr))
	{
		return false;
	}
	//ルートシグネチャ
	hr = m_device->CreateRootSignature(
		0,
		pBlob->GetBufferPointer(),
		pBlob->GetBufferSize(),
		IID_PPV_ARGS(m_rootsignature.GetAddressOf())
	);
	if (FAILED(hr))
	{
		return false;
	}
	return true;
}

bool RenderManager::Pipelinestate()
{
	D3D12_INPUT_ELEMENT_DESC element[2];
	element[0].SemanticName = "POSITION";
	element[0].SemanticIndex = 0;
	element[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	element[0].InputSlot = 0;
	element[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	element[0].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
	element[0].InstanceDataStepRate = 0;

	element[1].SemanticName = "TEXCOORD";
	element[1].SemanticIndex = 0;
	element[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	element[1].InputSlot = 0;
	element[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	element[1].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
	element[1].InstanceDataStepRate = 0;

	//ラスタライザー
	D3D12_RASTERIZER_DESC descRS;
	descRS.FillMode = D3D12_FILL_MODE_SOLID;
	descRS.CullMode = D3D12_CULL_MODE_NONE;
	descRS.FrontCounterClockwise = FALSE;
	descRS.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	descRS.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	descRS.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	descRS.DepthClipEnable = FALSE;
	descRS.MultisampleEnable = FALSE;
	descRS.AntialiasedLineEnable = FALSE;
	descRS.ForcedSampleCount = 0;
	descRS.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	//レンダーターゲット
	D3D12_RENDER_TARGET_BLEND_DESC descRTBS
	{
			FALSE, FALSE,
			D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
			D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
			D3D12_LOGIC_OP_NOOP,
			D3D12_COLOR_WRITE_ENABLE_ALL
	};

	//ブレンドステート
	D3D12_BLEND_DESC descBS;
	descBS.AlphaToCoverageEnable = FALSE;
	descBS.IndependentBlendEnable = FALSE;
	for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
	{
		descBS.RenderTarget[i]=descRTBS;
	}
	ComPtr<ID3DBlob> pVSBlob;
	ComPtr<ID3DBlob> pPSBlob;

	std::wstring vsPath;
	std::wstring psPath;

	if (!SearchFilePath(L"SimpleTexVS.cso", vsPath))
	{
		return false;
	}
	
	if (!SearchFilePath(L"SimpleTexPS.cso", psPath))
	{
		return false;
	}
	
	//頂点シェーダー
	auto hr = D3DReadFileToBlob(vsPath.c_str(), pVSBlob.GetAddressOf());
	if (FAILED(hr))
	{
		return false;
	}
	hr = D3DReadFileToBlob(psPath.c_str(), pPSBlob.GetAddressOf());
	if (FAILED(hr))
	{
		return false;
	}
	//パイプラインステートの設定

	D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
	desc.InputLayout = { element,_countof(element) };
	desc.pRootSignature = m_rootsignature.Get();
	desc.VS = { pVSBlob->GetBufferPointer(),pVSBlob->GetBufferSize() };
	desc.PS = { pPSBlob->GetBufferPointer(),pPSBlob->GetBufferSize() };
	desc.RasterizerState = descRS;
	desc.BlendState = descBS;
	desc.DepthStencilState.DepthEnable = FALSE;
	desc.DepthStencilState.StencilEnable = FALSE;
	desc.SampleMask = UINT_MAX;
	desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	desc.NumRenderTargets = 1;
	desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	desc.DSVFormat = DXGI_FORMAT_UNKNOWN;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;

	ComPtr<ID3DBlob>errorBlob;
	//パイプラインステート
	hr = m_device->CreateGraphicsPipelineState(
		&desc,
		IID_PPV_ARGS(m_pipelinestate.GetAddressOf())
	);
	if (FAILED(hr))
	{
		if (errorBlob) {
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		}
		return false;
	}

	return true;

}

bool RenderManager::DivVertexs(int rows, int cols)
{

	float cellWidth = 2.0f / cols;
	float cellHeight = 2.0f / rows;
	float uvWidth = 1.0f / cols;
	float uvHeight = 1.0f / rows;

	for (int y = 0; y < rows; ++y)
	{
		for (int x = 0; x < cols; ++x)
		{
			float left = -1.0f + cellWidth * x;
			float right = left + cellWidth;
			float top = 1.0f - cellHeight * y;
			float bottom = top - cellHeight;

			float uvLeft = uvWidth * x;
			float uvRight = uvLeft + uvWidth;
			float uvTop = uvHeight * y;
			float uvBottom = uvTop + uvHeight;

			vertices.push_back(DirectX::VertexPositionTexture(DirectX::XMFLOAT3(left, top, 0.0f), DirectX::XMFLOAT2(uvLeft, uvTop)));
			vertices.push_back(DirectX::VertexPositionTexture(DirectX::XMFLOAT3(right, top, 0.0f), DirectX::XMFLOAT2(uvRight, uvTop)));
			vertices.push_back(DirectX::VertexPositionTexture(DirectX::XMFLOAT3(right, bottom, 0.0f), DirectX::XMFLOAT2(uvRight, uvBottom)));
			vertices.push_back(DirectX::VertexPositionTexture(DirectX::XMFLOAT3(left, bottom, 0.0f), DirectX::XMFLOAT2(uvLeft, uvBottom)));
		}
	}

	D3D12_HEAP_PROPERTIES prop = {};
	prop.Type = D3D12_HEAP_TYPE_UPLOAD;
	prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	prop.CreationNodeMask = 1;
	prop.VisibleNodeMask = 1;

	//リソースの設定
	D3D12_RESOURCE_DESC desc = {};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.Alignment = 0;
	desc.Width = sizeof(DirectX::VertexPositionTexture) * vertices.size();
	desc.Height = 1;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	desc.Flags = D3D12_RESOURCE_FLAG_NONE;

	auto hr = m_device->CreateCommittedResource(
		&prop,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(m_pVB.GetAddressOf())
	);
	if (FAILED(hr))
	{
		return false;
	}
	//マッピング
	void* ptr = nullptr;
	hr = m_pVB->Map(0, nullptr, &ptr);
	if (FAILED(hr))
	{
		return false;
	}
	memcpy(ptr, vertices.data(), sizeof(DirectX::VertexPositionTexture) * vertices.size());
	m_pVB->Unmap(0, nullptr);

	m_VBV.BufferLocation = m_pVB->GetGPUVirtualAddress();
	m_VBV.SizeInBytes = static_cast<UINT>(sizeof(DirectX::VertexPositionTexture) * vertices.size());
	m_VBV.StrideInBytes = static_cast<UINT>(sizeof(DirectX::VertexPositionTexture));

	

	//インデックスバッファの生成
	{

		for (int y = 0; y < rows; y++)
		{
			for (int x = 0; x < cols; x++)
			{
				int index = x + y * cols;
				indices.push_back(index * 4 + 0);
				indices.push_back(index * 4 + 1);
				indices.push_back(index * 4 + 2);
				indices.push_back(index * 4 + 0);
				indices.push_back(index * 4 + 2);
				indices.push_back(index * 4 + 3);
			}
		}
		//ヒーププロパティ
		D3D12_HEAP_PROPERTIES prop = {};
		prop.Type = D3D12_HEAP_TYPE_UPLOAD;
		prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		prop.CreationNodeMask = 1;
		prop.VisibleNodeMask = 1;
		//リソースの設定
		D3D12_RESOURCE_DESC desc = {};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Alignment = 0;
		desc.Width = sizeof(uint32_t) * indices.size();
		desc.Height = 1;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		desc.Flags = D3D12_RESOURCE_FLAG_NONE;
		auto hr = m_device->CreateCommittedResource(
			&prop,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(m_pIB.GetAddressOf())
		);
		//マッピングする
		void* ptr = nullptr;
		hr = m_pIB->Map(0, nullptr, &ptr);
		if (FAILED(hr))
		{
			return false;
		}
		memcpy(ptr, indices.data(), sizeof(uint32_t) * indices.size());
		m_pIB->Unmap(0, nullptr);
		m_IBV.BufferLocation = m_pIB->GetGPUVirtualAddress();
		m_IBV.Format = DXGI_FORMAT_R32_UINT;
		m_IBV.SizeInBytes = sizeof(uint32_t) * indices.size();
	}
	return true;
}


bool RenderManager::LoadImages(const wchar_t* filename,int i)
{
	
		std::wstring texturePath;
		if (!SearchFilePath(filename, texturePath))
		{
			return false;
		}

		DirectX::ResourceUploadBatch batch(m_device.Get());
		batch.Begin();

		auto hr = DirectX::CreateWICTextureFromFile(
			m_device.Get(),
			batch,
			texturePath.c_str(),
			texture.pResource.GetAddressOf(),
			true
		);
		if (FAILED(hr))
		{
			return false;
		}

		auto future = batch.End(m_pQueue.Get());

		future.wait();

		m_HeapSRVIndex[i] = CreateShaderResourceView(texture.pResource.Get());
		

		textures.push_back(texture);

	
	

	return true;
}

unsigned int RenderManager::CreateShaderResourceView(ID3D12Resource* resource)
{
	unsigned int index = m_HeapSRVPool.front();

	m_HeapSRVPool.pop_front();

	auto incrementSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);


	auto handleCPU = m_pHeapCBV_SRV_UAV->GetCPUDescriptorHandleForHeapStart();
	auto handleGPU = m_pHeapCBV_SRV_UAV->GetGPUDescriptorHandleForHeapStart();

	handleCPU.ptr += incrementSize * index;
	handleGPU.ptr += incrementSize * index;

	texture.CPUDescriptor = handleCPU;
	texture.GPUDescriptor = handleGPU;

	auto textureDesc = texture.pResource->GetDesc();

	D3D12_SHADER_RESOURCE_VIEW_DESC viewdesc = {};
	viewdesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	viewdesc.Format = textureDesc.Format;
	viewdesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	viewdesc.Texture2D.MostDetailedMip = 0;
	viewdesc.Texture2D.MipLevels = textureDesc.MipLevels;
	viewdesc.Texture2D.PlaneSlice = 0;
	viewdesc.Texture2D.ResourceMinLODClamp = 0.0f;

	m_device->CreateShaderResourceView(
		resource, &viewdesc, handleCPU
	);

	return index;

}

void RenderManager::TermWnd()
{
	if (m_hInst != nullptr)
	{
		UnregisterClass(ClassName, m_hInst);
	}
	m_hInst = nullptr;
	m_hWnd = nullptr;
}

void RenderManager::OnTerm()
{
	for (auto i = 0; i < FrameCount; ++i)
	{
		if (m_pCB[i].Get() != nullptr)
		{
			m_pCB[i]->Unmap(0, nullptr);
			memset(&m_CBV[i], 0, sizeof(m_CBV[i]));
		}
		m_pCB[i].Reset();
	}

	m_pIB.Reset();
	m_pVB.Reset();
	m_pipelinestate.Reset();
	
	m_pHeapCBV_SRV_UAV.Reset();

	m_VBV.BufferLocation = 0;
	m_VBV.SizeInBytes = 0;
	m_VBV.StrideInBytes = 0;

	m_IBV.BufferLocation = 0;
	m_IBV.Format = DXGI_FORMAT_UNKNOWN;
	m_IBV.SizeInBytes = 0;

	m_rootsignature.Reset();

	texture.pResource.Reset();
	texture.CPUDescriptor.ptr = 0;
	texture.GPUDescriptor.ptr = 0;

	textures.clear();
}

void RenderManager::TermD3D()
{
	WaitGPU();

	if (m_FenceEvent != nullptr)
	{
		CloseHandle(m_FenceEvent);
		m_FenceEvent = nullptr;
	}

	m_fence.Reset();

	m_pHeapRTV.Reset();
	for (auto i = 0u; i < FrameCount; ++i)
	{
		m_pColorBuffer[i].Reset();
	}

	m_CommandList.Reset();

	for (auto i = 0u; i < FrameCount; ++i)
	{
		m_CommandAllocator[i].Reset();
	}
	m_swapchain.Reset();

	m_pQueue.Reset();

	m_device.Reset();

}

void RenderManager::WaitGPU()
{
	assert(m_pQueue != nullptr);
	assert(m_fence != nullptr);
	assert(m_FenceEvent != nullptr);

	m_pQueue->Signal(m_fence.Get(), m_FenceCounter[m_FrameIndex]);
	m_fence->SetEventOnCompletion(m_FenceCounter[m_FrameIndex], m_FenceEvent);
	WaitForSingleObjectEx(m_FenceEvent, INFINITE, FALSE);

	m_FenceCounter[m_FrameIndex]++;
}

LRESULT CALLBACK RenderManager::WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
		case WM_DESTROY:
			{ PostQuitMessage(0); }
			break;
		default:
			{}
			break;
	}
	return DefWindowProc(hWnd, msg, wp, lp);
}

void RenderManager::MainLoop()
{
	MSG msg = {};
	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE) == TRUE)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			Update();
			Render();
		}
	}
}

void RenderManager::PreDraw()
{

	
	m_CommandList->SetGraphicsRootSignature(m_rootsignature.Get());

	ID3D12DescriptorHeap* heaps[] = {m_pHeapCBV_SRV_UAV.Get()};
	
	
	
	m_CommandList->SetDescriptorHeaps(_countof(heaps), heaps);
	
	

}

void RenderManager::SetResourceBarrier(ComPtr<ID3D12GraphicsCommandList>commandlist,ID3D12Resource *resource,D3D12_RESOURCE_STATES beforestate,D3D12_RESOURCE_STATES afterstate,unsigned int Subresource)
{
	D3D12_RESOURCE_BARRIER resourceBarrier{};
	resourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	resourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	resourceBarrier.Transition.pResource = resource;
	resourceBarrier.Transition.StateBefore = beforestate;
	resourceBarrier.Transition.StateAfter = afterstate;

	commandlist->ResourceBarrier(1, &resourceBarrier);
}

void RenderManager::DrawImage()
{
	m_CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_CommandList->IASetVertexBuffers(0, 1, &m_VBV);
	m_CommandList->IASetIndexBuffer(&m_IBV);
	m_CommandList->RSSetViewports(1, &m_Viewport);
	m_CommandList->RSSetScissorRects(1, &m_Scissor);

	for (int i = 0; i < textures.size(); i++)
	{
		m_CommandList->SetGraphicsRootConstantBufferView(0, m_CBV[m_FrameIndex][i].Desc.BufferLocation);
		
		m_CommandList->SetGraphicsRootDescriptorTable(1, textures[i].GPUDescriptor);
		m_CommandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
	}

	
}

#include <cstdio>
#include <string>

bool RenderManager::PngImages(const wchar_t* filename, int& width, int& height, png_bytep& data)
{
    std::wstring texturePath;
    if (!SearchFilePath(filename, texturePath))
    {
        return false;
    }
    FILE* fp = nullptr;
    errno_t err = _wfopen_s(&fp, texturePath.c_str(), L"rb");
    if (err != 0 || fp == nullptr)
    {
        return false;
    }

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (png_ptr == nullptr)
    {
        fclose(fp);
        return false;
    }

    png_info* info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == nullptr)
    {
        png_destroy_read_struct(&png_ptr, nullptr, nullptr);
        fclose(fp);
        return false;
    }

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
        fclose(fp);
        return false;
    }

    png_init_io(png_ptr, fp);
    png_read_info(png_ptr, info_ptr);

    width = png_get_image_width(png_ptr, info_ptr);
    height = png_get_image_height(png_ptr, info_ptr);
    png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);
    png_byte color_type = png_get_color_type(png_ptr, info_ptr);

    if (bit_depth == 16)
    {
        png_set_strip_16(png_ptr);
    }
    if (color_type == PNG_COLOR_TYPE_PALETTE)
    {
        png_set_palette_to_rgb(png_ptr);
    }
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
    {
        png_set_expand_gray_1_2_4_to_8(png_ptr);
    }
    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
    {
        png_set_tRNS_to_alpha(png_ptr);
    }
    if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_PALETTE)
    {
        png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);
    }
    if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
    {
        png_set_gray_to_rgb(png_ptr);
    }
    png_read_update_info(png_ptr, info_ptr);

    png_bytep* row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);

    for (int i = 0; i < height; i++)
    {
        row_pointers[i] = (png_bytep)malloc(png_get_rowbytes(png_ptr, info_ptr));
    }
    png_read_image(png_ptr, row_pointers);
    //画像データを一次元配列にコピー
    data = (png_bytep)malloc(width * height * 4);
    for (int i = 0; i < height; i++)
    {
        memcpy(data + width * 4 * i, row_pointers[i], width * 4);
    }
    fclose(fp);
    for (int i = 0; i < height; i++)
    {
        free(row_pointers[i]);
    }
    free(row_pointers);
    png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
    return true;
}

bool RenderManager::CreateTextureFromPng(const wchar_t* filename,int i)
{
	int width = 0;
	int height = 0;
	png_bytep data = nullptr;
	if (!PngImages(filename, width, height, data))
	{
		return false;
	}
	//テクスチャ
	D3D12_HEAP_PROPERTIES heapProp = {};	
	heapProp.Type = D3D12_HEAP_TYPE_DEFAULT;
	heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProp.CreationNodeMask = 1;
	heapProp.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC desc = {};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	desc.Alignment = 0;
	desc.Width = width;
	desc.Height = height;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	desc.Flags = D3D12_RESOURCE_FLAG_NONE;

	auto hr = m_device->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(texture.pResource.GetAddressOf())
	);
	if (FAILED(hr))
	{
		free(data);
		return false;
	}
	//中間バッファ
	D3D12_HEAP_PROPERTIES uploadHeapProp = {};
	uploadHeapProp.Type = D3D12_HEAP_TYPE_UPLOAD;
	uploadHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	uploadHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	uploadHeapProp.CreationNodeMask = 1;
	uploadHeapProp.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC bufferDesc = {};	
	bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	bufferDesc.Alignment = 0;
	bufferDesc.Width = width * height * 4;
	bufferDesc.Height = 1;
	bufferDesc.DepthOrArraySize = 1;
	bufferDesc.MipLevels = 1;
	bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
	bufferDesc.SampleDesc.Count = 1;
	bufferDesc.SampleDesc.Quality = 0;
	bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	 
	ComPtr<ID3D12Resource> uploadbuffer;
	hr = m_device->CreateCommittedResource(
		&uploadHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(uploadbuffer.GetAddressOf())
	);
	if (FAILED(hr))
	{
		free(data);
		return false;
	}
	//中間バッファにデータをコピー
	void* mappedData = nullptr;
	hr = uploadbuffer->Map(0, nullptr, &mappedData);
	if (FAILED(hr))
	{
		free(data);
		return false;
	}
	memcpy(mappedData, data, width * height * 4);
	uploadbuffer->Unmap(0, nullptr);

	//テクスチャにデータを
	D3D12_TEXTURE_COPY_LOCATION dst = {};
	dst.pResource = texture.pResource.Get();
	dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	dst.SubresourceIndex = 0;

	D3D12_TEXTURE_COPY_LOCATION src = {};
	src.pResource = uploadbuffer.Get();
	src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	src.PlacedFootprint.Footprint.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	src.PlacedFootprint.Footprint.Width = width;
	src.PlacedFootprint.Footprint.Height = height;
	src.PlacedFootprint.Footprint.Depth = 1;
	src.PlacedFootprint.Footprint.RowPitch = width * 4;

	m_CommandList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

	//textureをシェーダーリソースビューに変換
	D3D12_RESOURCE_BARRIER barrier = {};	
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = texture.pResource.Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	m_CommandList->ResourceBarrier(1, &barrier);
	//シェーダーリソースビューの作成
	//D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc = {};
	//viewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	//viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	//viewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	//viewDesc.Texture2D.MostDetailedMip = 0;
	//viewDesc.Texture2D.MipLevels = 1;
	//viewDesc.Texture2D.PlaneSlice = 0;
	//viewDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	//
	//m_device->CreateShaderResourceView(texture.pResource.Get(), &viewDesc, texture.CPUDescriptor);

	m_HeapSRVIndex[i] = CreateShaderResourceView(texture.pResource.Get());


	textures.push_back(texture);

	free(data);

	return true;
}


void RenderManager::Render()
{
	DrawUI();
	SetResourceBarrier(m_CommandList, m_pColorBuffer[m_FrameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
	m_CommandList->OMSetRenderTargets(1, &m_HandleRTV[m_FrameIndex], FALSE, nullptr);

	float clearColor[] = { 0.25f,0.25f,0.25f,1.0f };

	m_CommandList->ClearRenderTargetView(m_HandleRTV[m_FrameIndex], clearColor, 0, nullptr);
	{
		PreDraw();
		m_CommandList->SetPipelineState(m_pipelinestate.Get());

		m_CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		DrawImage();
	}
	SetResourceBarrier(m_CommandList, m_pColorBuffer[m_FrameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

	m_CommandList->Close();

	ID3D12CommandList* ppCmdLists[] = { m_CommandList.Get() };
	m_pQueue->ExecuteCommandLists(1, ppCmdLists);

	Present(1);

}

void RenderManager::DrawUI()
{
	auto hr = m_CommandAllocator[m_FrameIndex]->Reset();
	hr = m_CommandList->Reset(m_CommandAllocator[m_FrameIndex].Get(), nullptr);
}

void RenderManager::Update()
{

	if (input->InputA())
	{
		x += 0.01f;
	}

	x += input->InputPad_X();
	y += input->InputPad_Y();	
	
	std::cout << x << std::endl;
	std::cout << y << std::endl;

	m_CBV[m_FrameIndex][0].pBuffer->World = DirectX::XMMatrixTranslation(x,y,0);
	m_CBV[m_FrameIndex][1].pBuffer->World = DirectX::XMMatrixTranslation(-2, 0, 0);

	

}

void RenderManager::SetPipelineState()
{
	m_CommandList->SetPipelineState(m_pipelinestate.Get());
}

void RenderManager::Present(uint32_t interval)
{
	m_swapchain->Present(interval, 0);

	//シグナル処理
	const auto currentValue = m_FenceCounter[m_FrameIndex];
	m_pQueue->Signal(m_fence.Get(), currentValue);

	m_FrameIndex = m_swapchain->GetCurrentBackBufferIndex();

	if (m_fence->GetCompletedValue() < m_FenceCounter[m_FrameIndex])
	{
		m_fence->SetEventOnCompletion(m_FenceCounter[m_FrameIndex], m_FenceEvent);
		WaitForSingleObjectEx(m_FenceEvent, INFINITE, FALSE);
	}
	//次のフレームのフェンスカウンターを増やす
	m_FenceCounter[m_FrameIndex] = currentValue + 1;
}
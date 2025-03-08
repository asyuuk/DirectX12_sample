#pragma once
#include"pch.h"
#include"FIleUtil.h"
#include<list>
#include"Input.h"
#include<libpng16/png.h>

class Input;




namespace
{
	const auto ClassName = TEXT("sample");
	const wchar_t* path = L"E:/clip studio/pngフォルダ　自分用　高画質/02.png";
	const wchar_t* path1 = L"E:/clip studio/pngフォルダ　自分用　高画質/abya.png";
}

struct alignas(256) Transform
{
	DirectX::XMMATRIX World;
	DirectX::XMMATRIX View;
	DirectX::XMMATRIX Proj;

};



template<class T>
struct ConstantBufferView
{
	D3D12_CONSTANT_BUFFER_VIEW_DESC Desc;
	D3D12_CPU_DESCRIPTOR_HANDLE HandleCPU;
	D3D12_GPU_DESCRIPTOR_HANDLE HandleGPU;
	T* pBuffer;
};

struct Texture
{
	ComPtr<ID3D12Resource> pResource;
	D3D12_CPU_DESCRIPTOR_HANDLE CPUDescriptor;
	D3D12_GPU_DESCRIPTOR_HANDLE GPUDescriptor;
	DirectX::XMMATRIX WorldMatrix;
	D3D12_GPU_VIRTUAL_ADDRESS WorldMatrixAddress;
	float speed;
};



class RenderManager
{

public:
	RenderManager(uint32_t Height,uint32_t width);
	~RenderManager();
	void Run();

	bool InitApp();
	void TermApp();
	bool InitD3D();
	bool InitWindow();
	void TermWnd();
	bool LoadImages(const wchar_t* filename,int i);

	bool LoadDivImages(const wchar_t* filename, int Rows,int cols);

	bool DivVertexs(int rows,int cols);

	bool HeapRTV_SRV_UAV();	

	bool ConstantBufferViews();
	bool Load();

	bool VertexBuffer_05();
	bool VertexBuffer_10();

	bool RootSignature();
	bool Pipelinestate();
	void TermD3D();
	void WaitGPU();
	void Present(uint32_t interval);
	void Render();

	bool InitImage();

	bool InitD3D12();

	void PreDraw();

	void DrawImage();

	void Update();

	void DrawUI();

	bool DivImages();

	bool PngImages(const wchar_t* filename, int& width, int& height, png_bytep& data);

	bool CreateTextureFromPng(const wchar_t* filename,int i);


	void SetPipelineState();
	void SetResourceBarrier(ComPtr<ID3D12GraphicsCommandList>commandlist, ID3D12Resource* resource, D3D12_RESOURCE_STATES beforestate, D3D12_RESOURCE_STATES afterstate, unsigned int Subresource);

	void MainLoop();

	void OnTerm();

	void ViewPortScissor();

	unsigned int CreateShaderResourceView(ID3D12Resource* resource);

	bool Images();


public: 
	static const uint32_t FrameCount = 2;
	HINSTANCE m_hInst;
	HWND m_hWnd;
	uint32_t m_Width;
	uint32_t m_Height;

	ComPtr<ID3D12Device>m_device;
	ComPtr<ID3D12GraphicsCommandList>m_CommandList;
	ComPtr<ID3D12CommandQueue>m_pQueue;
	ComPtr<IDXGISwapChain3>m_swapchain;
	ComPtr<ID3D12Resource>m_pColorBuffer[FrameCount];
	ComPtr<ID3D12CommandAllocator> m_CommandAllocator[FrameCount];
	ComPtr<ID3D12DescriptorHeap> m_pHeapRTV;
	ComPtr<ID3D12Fence> m_fence;
	ComPtr<ID3D12DescriptorHeap> m_pHeapCBV_SRV_UAV;
	ComPtr<ID3D12Resource>m_pVB;
	ComPtr<ID3D12Resource>m_pIV;
	ComPtr<ID3D12Resource>m_pIB;
	ComPtr<ID3D12Resource>m_pCB[FrameCount * 2];
	ComPtr<ID3D12RootSignature>m_rootsignature;
	ComPtr<ID3D12PipelineState>m_pipelinestate;

	std::list<unsigned int> m_HeapSRVPool;

	static const unsigned int m_HeapSRVPoolMax = 10000;

	unsigned int m_HeapSRVIndex[2];

	HANDLE m_FenceEvent;
	uint64_t m_FenceCounter[FrameCount];
	uint32_t m_FrameIndex;
	D3D12_CPU_DESCRIPTOR_HANDLE m_HandleRTV[FrameCount];
	D3D12_VERTEX_BUFFER_VIEW	m_VBV;
	D3D12_INDEX_BUFFER_VIEW m_IBV;
	D3D12_VIEWPORT	m_Viewport;
	D3D12_RECT m_Scissor;
	ConstantBufferView<Transform> m_CBV[FrameCount*2][2];
	std::vector<Texture>textures;
	Texture texture;
	float m_Rotate;
	//プレイヤー敵などの情報：位置
	float x;//player_x;
	float y;//player_y;

	std::shared_ptr<Input>input;

	DirectX::XMFLOAT4* m_pRect;


	static const unsigned int SRVDescriptorMax = 10000;

	std::vector<DirectX::VertexPositionTexture> vertices;

	std::vector<uint32_t> indices;

	int m_CurrentFrame;
	int m_TotalFrames;
	int m_AnimationSpeed;
	int m_ElapsedTime;

	png_structp png_ptr=nullptr;
	png_infop info_ptr=nullptr;
	png_bytep* row_pointers=nullptr;
	
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
};


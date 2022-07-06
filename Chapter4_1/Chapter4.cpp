#include <Windows.h>
//#define _USE_MATH_DEFINES
//#include <math.h>
#include<tchar.h>
#include<d3d12.h> //Chapter3_2_2 
#include<dxgi1_6.h> //Chapter3_2_2 
#include<vector> //Chapter3_2_2 
#include<DirectXMath.h> //Chapter4_2_1
#include<d3dcompiler.h> //Chapter4_6_2
#pragma comment(lib, "d3dcompiler.lib")

//#define DEF_TEST

#ifdef _DEBUG
#include <iostream>
#endif
using namespace std;
using namespace DirectX;
// @brief �R���\�[����ʂɃt�H�[�}�b�g�t���������\��
// @param format�t�H�[�}�b�g�i%d�Ƃ�%f�Ƃ��́j
// @param �ϒ�����
// @remarks ���̊֐��̓f�o�b�O�p�ł��B�f�o�b�O���ɂ������삵�܂���
void DebugOutputFormatString(const char* format, ...)
{
#ifdef _DEBUG    
	va_list valist;
	va_start(valist, format);
	printf(format, valist);
	va_end(valist);
#endif 
}
//Chapter3_2_2 P65
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
//

LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	//Window���j�����ꂽ��Ă΂��
	if (msg == WM_DESTROY) {
		PostQuitMessage(0);//OS�ɑ΂��āu�������̃A�v���͏I���v�Ɠ`����
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

const unsigned int window_width = 1920;
const unsigned int window_height = 1080;

//Chapter3_2_2 P66
ID3D12Device* _dev = nullptr;
IDXGIFactory6* _dxgiFactory = nullptr;
ID3D12CommandAllocator* _cmdAllocator = nullptr;	//Chapter3_3_2 P72
ID3D12GraphicsCommandList* _cmdList = nullptr;			//Chapter3_3_2 P72
ID3D12CommandQueue* _cmdQueue = nullptr;	// Chapter3_3_3 P74
IDXGISwapChain4* _swapchain = nullptr;		// Chapter3_3_3

//Chapter3_4
void EnableDebugLayer() {
	ID3D12Debug* debugLayer = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugLayer)))) {
		debugLayer->EnableDebugLayer();
		debugLayer->Release();
	}
}
//



#ifdef _DEBUG 
int main()
{
#else
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
#endif

#ifdef DEF_TEST
	cout << "defined DEF_TEST" << endl;
#endif
	DebugOutputFormatString("Show window test.");
	HINSTANCE hInst = GetModuleHandle(nullptr);	//���ǉ�
	// �E�B���h�E�N���X�̐������o�^
	WNDCLASSEX w = {};
	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProcedure;			// �R�[���o�b�N�֐��̎w��
	w.lpszClassName = _T("DX12Sample");						// �A�v���P�[�V�����N���X���i�K���ł悢�j
	w.hInstance = GetModuleHandle(nullptr);					// �n���h���̎擾
	RegisterClassEx(&w);													// �A�v���P�[�V�����N���X�i�E�B���h�E�N���X�̎w���OS�ɓ`����j
	RECT wrc = { 0, 0, window_width, window_height };	// �E�B���h�E�T�C�Y�����߂�	

	// �֐����g���ăE�B���h�E�̃T�C�Y��␳����
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);
	// �E�B���h�E�I�u�W�F�N�g�̐���
	HWND hwnd = CreateWindow(w.lpszClassName, // �N���X���w��    
		_T("DX12�e�X�g"),      // �^�C�g���o�[�̕���    
		WS_OVERLAPPEDWINDOW,    // �^�C�g���o�[�Ƌ��E��������E�B���h�E    
		CW_USEDEFAULT,          // �\��x���W��OS�ɂ��C��    
		CW_USEDEFAULT,          // �\��y���W��OS�ɂ��C��    
		wrc.right - wrc.left,   // �E�B���h�E��    
		wrc.bottom - wrc.top,   // �E�B���h�E��    
		nullptr,                // �e�E�B���h�E�n���h��    
		nullptr,                // ���j���[�n���h��    
		w.hInstance,            // �Ăяo���A�v���P�[�V�����n���h��    
		nullptr);               // �ǉ��p�����[�^�[
		// �E�B���h�E�\��

		//Chapter3_4
#ifdef _DEBUG    //�f�o�b�O���C���[���I����
	EnableDebugLayer();
#endif
	//


//Chapter3_4 ��Chapter3_2_2 
// DirectX12�̓���
//  1.IDXGIFactory6�𐶐�
	HRESULT result = S_OK;
	if (FAILED(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&_dxgiFactory)))) {
		if (FAILED(CreateDXGIFactory2(0, IID_PPV_ARGS(&_dxgiFactory)))) {
			DebugOutputFormatString("FAILED CreateDXGIFactory2");
			return -1;
		}
	}
	//  2.VGA�A�_�v�^IDXGIAdapter�̔z���IDXGIFactory6������o��
	std::vector <IDXGIAdapter*> adapters;
	IDXGIAdapter* tmpAdapter = nullptr;
	for (int i = 0; _dxgiFactory->EnumAdapters(i, &tmpAdapter) != DXGI_ERROR_NOT_FOUND; ++i) {
		adapters.push_back(tmpAdapter);
	}

	//3.�g�������A�_�v�^��VGA�̃��[�J�[�őI��
	for (auto adpt : adapters) {
		DXGI_ADAPTER_DESC adesc = {};
		adpt->GetDesc(&adesc); // �A�_�v�^�[�̐����I�u�W�F�N�g�擾
		std::wstring strDesc = adesc.Description;    // �T�������A�_�v�^�[�̖��O���m�F
		if (strDesc.find(L"NVIDIA") != std::string::npos) {
			tmpAdapter = adpt;
			break;
		}
	}

	//4.ID3D12Device��I�񂾃A�_�v�^��p���ď���������������
	D3D_FEATURE_LEVEL levels[] =
	{
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};
	D3D_FEATURE_LEVEL featureLevel;
	for (auto l : levels) {
		if (D3D12CreateDevice(tmpAdapter, l, IID_PPV_ARGS(&_dev)) == S_OK) {
			featureLevel = l;
			break;
		}
	}

	//Chapter3_3_2  
	//�R�}���h����̏���
	// �R�}���h�A���P�[�^�[ID3D12CommandAllocator�𐶐�
	result = _dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_cmdAllocator));
	if (result != S_OK) {
		DebugOutputFormatString("FAILED CreateCommandAllocator");
		return -1;
	}
	// �R�}���h���X�gID3D12GraphicsCommandList�𐶐�
	result = _dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _cmdAllocator, nullptr, IID_PPV_ARGS(&_cmdList));
	if (result != S_OK) {
		DebugOutputFormatString("FAILED CreateCommandList");
		return -1;
	}
	//Chapter3_3_3 P74  
	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
	// �^�C���A�E�g�Ȃ�
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE; // �A�_�v�^�[��1�����g��Ȃ��Ƃ���0�ł悢
	cmdQueueDesc.NodeMask = 0; // �v���C�I���e�B�͓��Ɏw��Ȃ�
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	// �R�}���h���X�g�ƍ��킹��
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	// �L���[����
	result = _dev->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&_cmdQueue));

	//Chapter3_3_3 P79  
	//�X���b�v�`�F�[��
	DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
	swapchainDesc.Width = window_width;
	swapchainDesc.Height = window_height;
	swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchainDesc.Stereo = false;
	swapchainDesc.SampleDesc.Count = 1;
	swapchainDesc.SampleDesc.Quality = 0;
	swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
	swapchainDesc.BufferCount = 2;
	// �o�b�N�o�b�t�@�[�͐L�яk�݉\
	swapchainDesc.Scaling = DXGI_SCALING_STRETCH;
	// �t���b�v��͑��₩�ɔj��
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	// ���Ɏw��Ȃ�
	swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	// �E�B���h�E�̃t���X�N���[���؂�ւ��\
	swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	result = _dxgiFactory->CreateSwapChainForHwnd(
		_cmdQueue,
		hwnd,
		&swapchainDesc,
		nullptr,
		nullptr,
		(IDXGISwapChain1**)&_swapchain);
	if (result != S_OK) {
		DebugOutputFormatString("FAILED CreateSwapChainForHwnd");
		return -1;
	}
	// Chapter3_3_5
	// �f�B�X�N���v�^�[�q�[�v�̐���
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;//�����_�[�^�[�Q�b�g�r���[�Ȃ̂œ��RRTV
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = 2;//�\���̂Q��
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;//���Ɏw��Ȃ�
	ID3D12DescriptorHeap* rtvHeaps = nullptr;
	result = _dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&rtvHeaps));

	DXGI_SWAP_CHAIN_DESC swcDesc = {};
	result = _swapchain->GetDesc(&swcDesc);
	std::vector<ID3D12Resource*> _backBuffers(swcDesc.BufferCount);
	D3D12_CPU_DESCRIPTOR_HANDLE handle = rtvHeaps->GetCPUDescriptorHandleForHeapStart();
	for (size_t i = 0; i < swcDesc.BufferCount; ++i) {
		result = _swapchain->GetBuffer(static_cast<UINT>(i), IID_PPV_ARGS(&_backBuffers[i]));//�o�b�t�@�̈ʒu�̃n���h�������o��
		_dev->CreateRenderTargetView(_backBuffers[i], nullptr, handle); //RTV���f�B�X�N���v�^�[�q�[�v�ɍ쐬
		// �f�B�X�N���v�^�̐擪�A�h���X��RTV�̃T�C�Y���A���ւ��炸
		handle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}
	//

	// Chapter3_4_2 P92
	ID3D12Fence* _fence = nullptr;
	UINT64 _fenceVal = 0;
	result = _dev->CreateFence(_fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));



	ShowWindow(hwnd, SW_SHOW);

	//Chapter4_2_1
	XMFLOAT3 vertices[] =
	{
		{-1.0f,-1.0f,0.0f},  //����
		{-1.0f,1.0f,0.0f},  //����
		{1.0f,-1.0f,0.0f},  //�E��

	};

	//Chapter4_3_4
	D3D12_HEAP_PROPERTIES heapprop = {};

	heapprop.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC resdesc = {};

	resdesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resdesc.Width = sizeof(vertices);  //���_��񂪓��邾���̃T�C�Y
	resdesc.Height = 1;
	resdesc.DepthOrArraySize = 1;
	resdesc.MipLevels = 1;
	resdesc.Format = DXGI_FORMAT_UNKNOWN;
	resdesc.SampleDesc.Count = 1;
	resdesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	resdesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	ID3D12Resource* vertBuff = nullptr;

	result = _dev->CreateCommittedResource(
		&heapprop,
		D3D12_HEAP_FLAG_NONE,
		&resdesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertBuff)
	);

	XMFLOAT3* vertMap = nullptr;

	result = vertBuff->Map(0, nullptr, (void**)&vertMap);

	copy(begin(vertices), end(vertices), vertMap);

	vertBuff->Unmap(0, nullptr);

	//���_�o�b�t�@�[�r���[��p��
	D3D12_VERTEX_BUFFER_VIEW vbView = {};

	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();  //�o�b�t�@�[�̉��z�A�h���X
	vbView.SizeInBytes = sizeof(vertices);  //�S�o�C�g��
	vbView.StrideInBytes = sizeof(vertices[0]);  //1���_������̃o�C�g��

	//Chapter4_6_1
	ID3DBlob* _vsBlob = nullptr;
	ID3DBlob* _psBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;

	result = D3DCompileFromFile(
		L"BasicVertexShader.hlsl",  //�V�F�[�_�[��
		nullptr,  //define�͂Ȃ�
		D3D_COMPILE_STANDARD_FILE_INCLUDE,  //�C���N���[�h�̓f�t�H���g
		"BasicVS", "vs_5_0",  //�֐���BasicVS�A�ΏۃV�F�[�_�[��vs_5_0
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,  //�f�o�b�N�p����эœK���Ȃ�
		0,
		&_vsBlob, &errorBlob);  //�G���[����errorBlob�Ƀ��b�Z�[�W������

	result = D3DCompileFromFile(
		L"BasicPixelShader.hlsl",  //�V�F�[�_�[��
		nullptr,  //define�͂Ȃ�
		D3D_COMPILE_STANDARD_FILE_INCLUDE,  //�C���N���[�h�̓f�t�H���g
		"BasicPS", "vs_5_0",  //�֐���BasicVS�A�ΏۃV�F�[�_�[��vs_5_0
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,  //�f�o�b�N�p����эœK���Ȃ�
		0,
		&_vsBlob, &errorBlob);  //�G���[����errorBlob�Ƀ��b�Z�[�W������


	MSG	msg = {};
	float clearColor[] = { 1.0f, 1.0f, 0.0f, 1.0f }; //���F
	//UINT f = 0;


	while (true) {
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		//�A�v���P�[�V�������I���Ƃ���message��WM_QUIT�ɂȂ�    
		if (msg.message == WM_QUIT) {
			break;
		}


		//f++;
		//clearColor[0] = (sin((f % 220) / 190.0f * M_PI));
		//clearColor[1] = (sin((f % 340) / 280.0f * M_PI));
		//clearColor[2] = (sin((f % 520) / 360.0f * M_PI));




		// Chapter3_3_6
		// �X���b�v�`�F�[���𓮍�
		auto bbIdx = _swapchain->GetCurrentBackBufferIndex();
		//// Chapter3_4_3�@ ���\�[�X�o���A
		D3D12_RESOURCE_BARRIER BarrierDesc = {};
		BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		BarrierDesc.Transition.pResource = _backBuffers[bbIdx];
		BarrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		////

		auto rtvH = rtvHeaps->GetCPUDescriptorHandleForHeapStart();
		rtvH.ptr += bbIdx * _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		_cmdList->OMSetRenderTargets(1, &rtvH, true, nullptr);
		_cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);

		//// Chapter3_4_3�@ ���\�[�X�o���A
		BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		_cmdList->ResourceBarrier(1, &BarrierDesc);
		////

		// ���߂̃N���[�Y
		_cmdList->Close();
		//�R�}���h���X�g�̎��s
		ID3D12CommandList* cmdlists[] = { _cmdList };
		_cmdQueue->ExecuteCommandLists(1, cmdlists);

		//// Chapter3_4_2 P93
		_cmdQueue->Signal(_fence, ++_fenceVal);
		if (_fence->GetCompletedValue() != _fenceVal) {
			auto event = CreateEvent(nullptr, false, false, nullptr);
			_fence->SetEventOnCompletion(_fenceVal, event);
			WaitForSingleObject(event, INFINITE);
			CloseHandle(event);
		}
		////

		_cmdAllocator->Reset();//�L���[���N���A
		_cmdList->Reset(_cmdAllocator, nullptr);//�ĂуR�}���h���X�g�����߂鏀��
		//�t���b�v
		_swapchain->Present(1, 0);
		//

	}
	//�����N���X�͎g��Ȃ��̂œo�^��������
	UnregisterClass(w.lpszClassName, w.hInstance);

	//getchar();
	return 0;
}
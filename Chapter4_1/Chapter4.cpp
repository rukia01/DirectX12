
#include <Windows.h>
#include<tchar.h>
#include<d3d12.h> //Chapter3_2_2 
#include<dxgi1_6.h>//Chapter3_2_2 
#include<vector>//Chapter3_2_2 
#include <DirectXMath.h>//Chapter4_2_1 P103
#include<d3dcompiler.h>							//Chapter4_6_2 P118
#pragma comment(lib, "d3dcompiler.lib")		//Chapter4_6_2 P118

using namespace DirectX;//Chapter4_2_1 P104

//#define DEF_TEST

#ifdef _DEBUG
#include <iostream>
#endif
using namespace std;
// @brief �R���\�[����ʂɃt�H�[�}�b�g�t���������\��
// @param format�t�H�[�}�b�g�i%d�Ƃ�%f�Ƃ��́j
// @param �ϒ�����
// @remarks ���̊֐��̓f�o�b�O�p�ł��B�f�o�b�O���ɂ������삵�܂���
void DebugOutputFormatString(const char* format, ...)
{
#ifdef _DEBUG    
	va_list valist;
	va_start(valist, format);
	vprintf(format, valist);
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

	//Chapter4_2_1 P105
	//XMFLOAT3 vertices[] = {
	//	{-0.5f, -1.0f, 0.0f} ,	//����    
	//	{-1.0f,  1.0f, 0.0f} ,	//����    
	//	{ 1.0f, -1.0f, 0.0f} , 	//�E��
	//};

	XMFLOAT3 vertices[] = {
		{-0.4f, -0.7f, 0.0f} , // ����
		{-0.4f, 0.7f, 0.0f} , // ����
		{0.4f, -0.7f, 0.0f} , // �E��
		{0.4f, 0.7f, 0.0f},  //�E��
	};


	//Chapter4_3_4 P112
	D3D12_HEAP_PROPERTIES heapprop = {};
	heapprop.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC resdesc = {};
	resdesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resdesc.Width = sizeof(vertices);
	resdesc.Height = 1;
	resdesc.DepthOrArraySize = 1;
	resdesc.MipLevels = 1;
	resdesc.Format = DXGI_FORMAT_UNKNOWN;
	resdesc.SampleDesc.Count = 1;
	resdesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	resdesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	//UPLOAD(�m�ۂ͉\)
	ID3D12Resource* vertBuff = nullptr;
	result = _dev->CreateCommittedResource(
		&heapprop,
		D3D12_HEAP_FLAG_NONE,
		&resdesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertBuff));

	//Chapter4_4_1 P113
	XMFLOAT3* vertMap = nullptr;
	result = vertBuff->Map(0, nullptr, (void**)&vertMap);
	std::copy(std::begin(vertices), std::end(vertices), vertMap);
	vertBuff->Unmap(0, nullptr);

	//Chapter4_4_2 P114
	// ���_�o�b�t�@�r���[��p��
	D3D12_VERTEX_BUFFER_VIEW vbView = {};
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress(); // �o�b�t�@�[�̉��z�A�h���X
	vbView.SizeInBytes = sizeof(vertices);      // �S�o�C�g��
	vbView.StrideInBytes = sizeof(vertices[0]); // 1���_������̃o�C�g��

	unsigned short indices[] = { 0,1,2, 2,1,3 };

	ID3D12Resource* idxBuff = nullptr;
	//�ݒ�́A�o�b�t�@�̃T�C�Y�ȊO���_�o�b�t�@�̐ݒ���g���܂킵��
	//OK���Ǝv���܂��B
	resdesc.Width = sizeof(indices);
	result = _dev->CreateCommittedResource(
		&heapprop,
		D3D12_HEAP_FLAG_NONE,
		&resdesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&idxBuff));

	//������o�b�t�@�ɃC���f�b�N�X�f�[�^���R�s�[
	unsigned short* mappedIdx = nullptr;
	idxBuff->Map(0, nullptr, (void**)&mappedIdx);
	std::copy(std::begin(indices), std::end(indices), mappedIdx);
	idxBuff->Unmap(0, nullptr);

	//�C���f�b�N�X�o�b�t�@�r���[���쐬
	D3D12_INDEX_BUFFER_VIEW ibView = {};
	ibView.BufferLocation = idxBuff->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R16_UINT;
	ibView.SizeInBytes = sizeof(indices);


	//Chapter4_6_1 P118
	ID3DBlob* _vsBlob = nullptr;
	ID3DBlob* _psBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;

	result = D3DCompileFromFile(
		L"BasicVertexShader.hlsl", // �V�F�[�_�[��
		nullptr, // define �͂Ȃ�
		D3D_COMPILE_STANDARD_FILE_INCLUDE, // �C���N���[�h�̓f�t�H���g
		"BasicVS", "vs_5_0", // �֐��� BasicVS�A�ΏۃV�F�[�_�[�� vs_5_0
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // �f�o�b�O�p����эœK���Ȃ�
		0,
		&_vsBlob, &errorBlob); // �G���[���� errorBlob �Ƀ��b�Z�[�W������
	if (FAILED(result))
	{
		if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
		{
			::OutputDebugStringA(" �t�@�C������������܂��� ");
			return 0; // exit() �ȂǂɓK�X�u��������ق����悢
		}
		else
		{
			std::string errstr;
			errstr.resize(errorBlob->GetBufferSize());
			std::copy_n((char*)errorBlob->GetBufferPointer(),
				errorBlob->GetBufferSize(),
				errstr.begin());
			errstr += "\n";
			::OutputDebugStringA(errstr.c_str());
		}
	}

	result = D3DCompileFromFile(
		L"BasicPixelShader.hlsl", // �V�F�[�_�[��
		nullptr, // define �͂Ȃ�
		D3D_COMPILE_STANDARD_FILE_INCLUDE, // �C���N���[�h�̓f�t�H���g
		"BasicPS", "ps_5_0", // �֐��� BasicPS�A�ΏۃV�F�[�_�[�� ps_5_0
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // �f�o�b�O�p����эœK���Ȃ�
		0,
		&_psBlob, &errorBlob); // �G���[���� errorBlob �Ƀ��b�Z�[�W������
	if (FAILED(result))
	{
		if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
		{
			::OutputDebugStringA(" �t�@�C������������܂��� ");
			return 0; // exit() �ȂǂɓK�X�u��������ق����悢
		}
		else
		{
			std::string errstr;
			errstr.resize(errorBlob->GetBufferSize());
			std::copy_n((char*)errorBlob->GetBufferPointer(),
				errorBlob->GetBufferSize(),
				errstr.begin());
			errstr += "\n";
			::OutputDebugStringA(errstr.c_str());
		}
	}

	// Chapter4_7 P127
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{
			"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
	};
	//

	// Chapter4_8_3 P130
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline = {};
	gpipeline.pRootSignature = nullptr; // ���ƂŐݒ肷��
	gpipeline.VS.pShaderBytecode = _vsBlob->GetBufferPointer();
	gpipeline.VS.BytecodeLength = _vsBlob->GetBufferSize();
	gpipeline.PS.pShaderBytecode = _psBlob->GetBufferPointer();
	gpipeline.PS.BytecodeLength = _psBlob->GetBufferSize();

	// Chapter4_8_4 P130
	// �f�t�H���g�̃T���v���}�X�N��\���萔�i0xffffffff�j
	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	gpipeline.RasterizerState.MultisampleEnable = false;
	gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;  // �J�����O���Ȃ�
	gpipeline.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID; // ���g��h��Ԃ�
	gpipeline.RasterizerState.DepthClipEnable = true; // �[�x�����̃N���b�s���O�͗L����
	//�c��
	gpipeline.RasterizerState.FrontCounterClockwise = false;
	gpipeline.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	gpipeline.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	gpipeline.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	gpipeline.RasterizerState.AntialiasedLineEnable = false;
	gpipeline.RasterizerState.ForcedSampleCount = 0;
	gpipeline.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	// Chapter4_8_5 P133
	gpipeline.BlendState.AlphaToCoverageEnable = false;
	gpipeline.BlendState.IndependentBlendEnable = false;
	D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = {};
	renderTargetBlendDesc.BlendEnable = false;
	renderTargetBlendDesc.LogicOpEnable = false;
	renderTargetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	gpipeline.BlendState.RenderTarget[0] = renderTargetBlendDesc;

	gpipeline.DepthStencilState.DepthEnable = false;
	gpipeline.DepthStencilState.StencilEnable = false;

	// Chapher4_8_6 P137
	gpipeline.InputLayout.pInputElementDescs = inputLayout; // ���C�A�E�g�擪�A�h���X
	gpipeline.InputLayout.NumElements = _countof(inputLayout); // ���C�A�E�g�z��̗v�f��
	// Chapher4_8_6 P138
	gpipeline.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;//�X�g���b�v���̃J�b�g�Ȃ�
	gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;//�O�p�`�ō\��


	// Chapher4_8_7 P139
	gpipeline.NumRenderTargets = 1; // ���� 1 �̂�
	gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; // 0 �` 1 �ɐ��K�����ꂽ RGBA

	// Chapter4_8_8
	gpipeline.SampleDesc.Count = 1; // �T���v�����O�� 1 �s�N�Z���ɂ� 1
	gpipeline.SampleDesc.Quality = 0; // �N�I���e�B�͍Œ�

	// Chapter4_9_2 P141
	ID3D12RootSignature* rootsignature = nullptr;
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	ID3DBlob* rootSigBlob = nullptr;
	result = D3D12SerializeRootSignature(
		&rootSignatureDesc, // ���[�g�V�O�l�`���ݒ�
		D3D_ROOT_SIGNATURE_VERSION_1_0, // ���[�g�V�O�l�`���o�[�W����
		&rootSigBlob, // �V�F�[�_�[��������Ƃ��Ɠ���
		&errorBlob); // �G���[����������

	// Chapter4_9_2 P142
	result = _dev->CreateRootSignature(
		0, // nodemask�B0 �ł悢
		rootSigBlob->GetBufferPointer(), // �V�F�[�_�[�̂Ƃ��Ɠ��l
		rootSigBlob->GetBufferSize(), // �V�F�[�_�[�̂Ƃ��Ɠ��l
		IID_PPV_ARGS(&rootsignature));
	rootSigBlob->Release(); // �s�v�ɂȂ����̂ŉ��
	gpipeline.pRootSignature = rootsignature;

	// Chapter4_8_9 P139
	ID3D12PipelineState* _pipelinestate = nullptr;
	result = _dev->CreateGraphicsPipelineState(
		&gpipeline, IID_PPV_ARGS(&_pipelinestate));

	// Chapter4_10_1 P143
	D3D12_VIEWPORT viewport = {};
	viewport.Width = window_width; // �o�͐�̕��i�s�N�Z�����j
	viewport.Height = window_height; // �o�͐�̍����i�s�N�Z�����j
	viewport.TopLeftX = 0; // �o�͐�̍�����W X
	viewport.TopLeftY = 0; // �o�͐�̍�����W Y
	viewport.MaxDepth = 1.0f; // �[�x�ő�l
	viewport.MinDepth = 0.0f; // �[�x�ŏ��l
	// Chapter4_10_2 P143
	D3D12_RECT scissorrect = {};
	scissorrect.top = 0; // �؂蔲������W
	scissorrect.left = 0; // �؂蔲�������W
	scissorrect.right = scissorrect.left + window_width; // �؂蔲���E���W
	scissorrect.bottom = scissorrect.top + window_height; // �؂蔲�������W

	MSG	msg = {};
	float clearColor[] = { 1.0f, 1.0f, 0.0f, 1.0f }; //���F

	while (true) {
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		//�A�v���P�[�V�������I���Ƃ���message��WM_QUIT�ɂȂ�    
		if (msg.message == WM_QUIT) {
			break;
		}

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
		_cmdList->ResourceBarrier(1, &BarrierDesc);
		////

		// Chapter4_10_3 P144
		_cmdList->SetPipelineState(_pipelinestate);

		auto rtvH = rtvHeaps->GetCPUDescriptorHandleForHeapStart();
		rtvH.ptr += bbIdx * _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		_cmdList->OMSetRenderTargets(1, &rtvH, true, nullptr);
		_cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);


		// Chapter4_10_3 P145 ����
		_cmdList->RSSetViewports(1, &viewport);
		_cmdList->RSSetScissorRects(1, &scissorrect);
		_cmdList->SetGraphicsRootSignature(rootsignature);
		_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		_cmdList->IASetVertexBuffers(0, 1, &vbView);
		_cmdList->IASetIndexBuffer(&ibView);
		//_cmdList->DrawInstanced(4, 1, 0, 0);
		_cmdList->DrawIndexedInstanced(6, 1, 0, 0, 0);

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
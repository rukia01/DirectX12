#include <Windows.h>
#include <tchar.h>
#ifdef _DEBUG
#include <iostream>
#endif
#include<d3d12.h>
#include<dxgi1_6.h>
#include<vector>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

using namespace std;

// @brief コンソール画面にフォーマット付き文字列を表示
// @param format フォーマット（%dとか%fとかの）
// @param 可変長引数
// @remarks この関数はデバック用です。デバック時にしか動きません。
void DebugOutputFormatString(const char* format, ...)
{
#ifdef _DEBUG
	va_list valist;
	va_start(valist, format);
	printf(format, valist);
	va_end(valist);
#endif
}

LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	//ウィンドウが破棄されたら呼ばれる
	if (msg == WM_DESTROY)
	{
		PostQuitMessage(0);  //OSに対して「もうこのアプリは終わる」と伝える
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);  //既存の処理を行う
}

const unsigned int window_width = 1280;
const unsigned int window_height = 720;

ID3D12Device* _dev = nullptr;
IDXGIFactory6* _dxgiFactory = nullptr;
ID3D12CommandAllocator* _cmdAllocator = nullptr;
ID3D12GraphicsCommandList* _cmdList = nullptr;
ID3D12CommandQueue* _cmdQueue = nullptr;
IDXGISwapChain4* _swapchain = nullptr;

float clearColor[] = { 1.0f,1.0f,0.0f,1.0f };  //黄色

#ifdef _DEBUG
int main()
{
#else
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
#endif
	DebugOutputFormatString("Show Window Test.");
	HINSTANCE hlnst = GetModuleHandle(nullptr);  //※追加
	//ウィンドウクラスの生成＆登録
	WNDCLASSEX w = {};

	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProcedure;  //コールバック関数の指定
	w.lpszClassName = _T("DX12Sample");  //アプリケーションクラス名（適当でよい）
	w.hInstance = GetModuleHandle(nullptr);  //ハンドルの取得

	RegisterClassEx(&w);  //アプリケーションクラス（ウィンドウクラスの指定をOSに伝える）

	RECT wrc = { 0,0,window_width,window_height };  //ウィンドウサイズを決める

	//関数を使ってウィンドウのサイズを補正する
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	//ウィンドウオブジェクトの生成
	HWND hwnd = CreateWindow(w.lpszClassName,  //クラス名指定
		_T("DX12テスト"),  //タイトルバーの文字

		WS_OVERLAPPEDWINDOW,  //タイトルバーと境界線があるウィンドウ
		CW_USEDEFAULT,  //表示x座標はOSにお任せ
		CW_USEDEFAULT,  //表示y座標はOSにお任せ
		wrc.right - wrc.left,  //ウィンドウ幅
		wrc.bottom - wrc.top,  //ウィンドウ高
		nullptr,  //親ウィンドウハンドル
		nullptr,  //メニューハンドル
		w.hInstance,  //呼び出しアプリケーションハンドル
		nullptr);  //追加パラメーター

	//DirectX12の導入
	//1.IDXGIFactory6を生成
	auto result = CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory));
	if (result != S_OK)
	{
		DebugOutputFormatString("FAILED CreateDXGIFactory1");
		return -1;
	}

	//2.VGAアダプタIDXGIAdapterの配列をIDXGIFactoryから取り出す

	vector<IDXGIAdapter*> adapters;
	IDXGIAdapter* tmpAdapter = nullptr;
	for (int i = 0; _dxgiFactory->EnumAdapters(i, &tmpAdapter) != DXGI_ERROR_NOT_FOUND; ++i)
	{
		adapters.push_back(tmpAdapter);
	}

	//3.使いたいアダプタをVGAのメーカーで選ぶ
	for (auto adpt : adapters)
	{
		DXGI_ADAPTER_DESC adesc = {};
		adpt->GetDesc(&adesc);  //アダプターの説明オブジェクト取得
		wstring strDesc = adesc.Description;  //探したいアダプターの名前を確認
		if (strDesc.find(L"NVIDIA") != std::string::npos)
		{
			tmpAdapter = adpt;
			break;
		}
	}

	//4.ID3D12Deviceを選んだアダプタを用いて初期化し生成する
	D3D_FEATURE_LEVEL levels[] =
	{
		   D3D_FEATURE_LEVEL_12_1,
		   D3D_FEATURE_LEVEL_12_0,
		   D3D_FEATURE_LEVEL_11_1,
		   D3D_FEATURE_LEVEL_11_0,
	};
	D3D_FEATURE_LEVEL featureLevel;
	for (auto l : levels)
	{
		if (D3D12CreateDevice(tmpAdapter, l, IID_PPV_ARGS(&_dev)) == S_OK)
		{
			featureLevel = l;
			break;
		}
	}

	//コマンド周りの準備
	//コマンドアロケーターID3D12CommandAllocatorを生成
	result = _dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_cmdAllocator));
	if (result != S_OK)
	{
		DebugOutputFormatString("FAILED CreateCommandAllocator");
		return -1;
	}
	//コマンドリストID3D12GraphicsCommandListを生成
	result = _dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _cmdAllocator, nullptr, IID_PPV_ARGS(&_cmdList));
	if (result != S_OK)
	{
		DebugOutputFormatString("FAILED CreateCommandList");
		return -1;
	}

	//コマンドキューD3D12_COMMAND_QUEUE_DESCに関する設定を作成
	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;  //タイムアウトなし
	cmdQueueDesc.NodeMask = 0;
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;  //プライオリティ特になし
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;  //ここはコマンドリストと合わせてください
	//コマンドキューID3D12CommandQueueを生成
	result = _dev->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&_cmdQueue));  //コマンドキュー生成
	if (result != S_OK)
	{
		DebugOutputFormatString("FAILED CreateCommandQueue");
		return -1;
	}

	//スワップチェーン

	DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};

	swapchainDesc.Width = window_width;
	swapchainDesc.Height = window_height;
	swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchainDesc.Stereo = false;
	swapchainDesc.SampleDesc.Count = 1;
	swapchainDesc.SampleDesc.Quality = 0;
	swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
	swapchainDesc.BufferCount = 2;

	//バックバッファーは伸び縮み可能
	swapchainDesc.Scaling = DXGI_SCALING_STRETCH;

	//フリップ後は速やかに破棄
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	//特に指定なし
	swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;

	//ウィンドウ⇔フルスクリーン切り替え可能
	swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	result = _dxgiFactory->CreateSwapChainForHwnd(
		_cmdQueue,
		hwnd,
		&swapchainDesc,
		nullptr,
		nullptr,
		(IDXGISwapChain1**)&_swapchain);
	if (result != S_OK)
	{
		DebugOutputFormatString("FAILED CreateSwapChainForHwnd");
		return -1;
	}

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;  //レンダラーターゲットビューなので楊戩RTV
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = 2;  //表裏の２つ
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;  //特に指定なし
	ID3D12DescriptorHeap* rtvHeaps = nullptr;
	result = _dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&rtvHeaps));

	//ディスクリプターヒープ内にレンダーターゲットビューを作成
	DXGI_SWAP_CHAIN_DESC swcDesc = {};
	result = _swapchain->GetDesc(&swcDesc);
	if (result != S_OK)
	{
		DebugOutputFormatString("FAILED IDXGISwapChain4->GetDesc");
		return -1;
	}
	vector<ID3D12Resource*> _backBuffers(swcDesc.BufferCount);
	D3D12_CPU_DESCRIPTOR_HANDLE handle = rtvHeaps->GetCPUDescriptorHandleForHeapStart();
	for (size_t i = 0; i < swcDesc.BufferCount; ++i)
	{
		result = _swapchain->GetBuffer(static_cast<UINT>(i), IID_PPV_ARGS(&_backBuffers[i]));
		_dev->CreateRenderTargetView(_backBuffers[i], nullptr, handle);
		handle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

	//ウィンドウ表示
	ShowWindow(hwnd, SW_SHOW);

	MSG msg = {};

	while (true)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		//アプリケーションが終わるときにmessageがWM_QUITになる
		if (msg.message == WM_QUIT)
		{
			break;
		}

		//スワップチェーンを動作
		auto bbldx = _swapchain->GetCurrentBackBufferIndex();
		auto rtvH = rtvHeaps->GetCPUDescriptorHandleForHeapStart();
		rtvH.ptr += bbldx * _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		_cmdList->OMSetRenderTargets(1, &rtvH, true, nullptr);
		_cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);
		//命令のクローズ
		_cmdList->Close();
		//コマンドリストの実行
		ID3D12CommandList* cmdlists[] = { _cmdList };
		_cmdQueue->ExecuteCommandLists(1, cmdlists);
		_cmdAllocator->Reset();  //キューをクリア
		_cmdList->Reset(_cmdAllocator, nullptr);  //再びコマンドリストをためる準備
		//フリップ
		_swapchain->Present(1, 0);

	}

	//もうクラスは使わないので登録解除する
	UnregisterClass(w.lpszClassName, w.hInstance);
	//getchar();
	return 0;
}
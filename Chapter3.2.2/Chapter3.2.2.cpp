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
IDXGISwapChain4* _swapchain = nullptr;

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
	}

	//もうクラスは使わないので登録解除する
	UnregisterClass(w.lpszClassName, w.hInstance);
	//getchar();
	return 0;
}
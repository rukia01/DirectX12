#include <Windows.h>
#ifdef _DEBUG
#include <iostream>
#endif

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

#ifdef _DEBUG
int main() 
{
#else
int WINAPI WinMain(HINSTANCE,HINSTANCE,LPSTR,int)
{
#endif
	DebugOutputFormatString("Show Window Test.");
	getchar();
	return 0;
}


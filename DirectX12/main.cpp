#include <Windows.h>
#ifdef _DEBUG
#include <iostream>
#endif

using namespace std;

// @brief �R���\�[����ʂɃt�H�[�}�b�g�t���������\��
// @param format �t�H�[�}�b�g�i%d�Ƃ�%f�Ƃ��́j
// @param �ϒ�����
// @remarks ���̊֐��̓f�o�b�N�p�ł��B�f�o�b�N���ɂ��������܂���B
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


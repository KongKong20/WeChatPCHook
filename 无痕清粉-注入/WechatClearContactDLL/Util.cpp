#include "stdafx.h"
#include <string>
#include "Util.h"


DWORD GetWeChatWinBaseAddress()
{
	DWORD baseAddress = 0;

	baseAddress = (DWORD)GetModuleHandle(L"WeChatWin.dll");

	return baseAddress;
}

wstring GetWStringByAddress(DWORD memoryAddress)
{
	wstring message;

	DWORD msgLength = *(DWORD*)(memoryAddress + 4);
	if (msgLength > 0) {
		WCHAR* msg = new WCHAR[msgLength + 1]{ 0 };

		try {
			wmemcpy_s(msg, msgLength + 1, (WCHAR*)(*(DWORD*)memoryAddress), msgLength + 1);
			message = msg;
		}
		catch(...)
		{

		}
		
		delete[]msg;
	}
	
	return  message;
}


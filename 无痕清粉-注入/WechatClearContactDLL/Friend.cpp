#include "stdafx.h"
#include "Friend.h"
#include "Util.h"

void DelFriend(wchar_t* wxid)
{
	DWORD delFriendCallAddr = GetWeChatWinBaseAddress() + OFFSET_FRIEND_DEL_FRIEND;

	struct WxStringFill2
	{
		const wchar_t* buffer;
		int size;
		int capacity;
		int fill[2];
	};

	wstring wsWxId = wxid;
	WxStringFill2 wxWxId = { 0 };
	wxWxId.buffer = wsWxId.c_str();
	wxWxId.size = wsWxId.size();
	wxWxId.capacity = wsWxId.capacity();

	__asm {
		mov ecx, 0;
		push ecx;
		lea edi, wxWxId;
		push edi;
		call delFriendCallAddr;
	}
}

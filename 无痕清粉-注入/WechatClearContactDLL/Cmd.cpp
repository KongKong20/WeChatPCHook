#include "stdafx.h"
#include "Cmd.h"
#include "Contacts.h"
#include "VerifyUser.h"
#include "Friend.h"

extern int robotId;
extern HWND hCurWnd;

void SendCmdToMaster(ULONG_PTR dwData, DWORD cbData, PVOID lpData)
{
	HWND hMaster = FindWindow(NULL, TEXT(MASTER_NAME));
	if (hMaster == NULL)
	{
		MessageBoxA(NULL, "未查找到Master", "错误", MB_OK);
		return;
	}

	COPYDATASTRUCT msg;
	msg.dwData = dwData;
	msg.lpData = lpData;
	msg.cbData = cbData;
	
	//发送消息给控制端
	SendMessage(hMaster, WM_COPYDATA, (WPARAM)hCurWnd, (LPARAM)&msg);
}

void DealCmdFromMaster(ULONG_PTR dwData, DWORD cbData, PVOID lpData)
{
	int messageType = dwData;
	switch (messageType) 
	{
	case CT_CONTACT_CHECK:
	{
		ContactInfoStruct contactInfo = *(ContactInfoStruct*)lpData;
		StartVerifyUser(contactInfo.wcWxId);
	}
	break;

	case CT_FRIEND_DEL:
	{
		FriendOperStruct friendOperMessage = *(FriendOperStruct*)lpData;
		DelFriend(friendOperMessage.wcWxId);
		SendCmdToMaster(dwData, cbData, lpData);
	}
	break;

	default:
		break;
	}
}


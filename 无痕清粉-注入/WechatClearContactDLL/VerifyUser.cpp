#include "stdafx.h"
#include "VerifyUser.h"
#include "Contacts.h"
#include "Cmd.h"
#include <string>
using namespace std;

DWORD baseAddress = (DWORD)GetModuleHandle(L"WeChatWin.dll");

#define OFFSET_CONTACTS_VERIFY_USER_CALL_1	0x052550
#define OFFSET_CONTACTS_VERIFY_USER_CALL_2 	0x06E910
#define OFFSET_CONTACTS_VERIFY_USER_CALL_3 	0x4DEDB0
#define OFFSET_CONTACTS_VERIFY_USER_CALL_4	0x4DEDF0
#define OFFSET_CONTACTS_VERIFY_USER_CALL_5 	0x04E720
#define OFFSET_CONTACTS_VERIFY_USER_CALL	0x2AAF70
DWORD verifyUserCall1 = baseAddress + OFFSET_CONTACTS_VERIFY_USER_CALL_1;
DWORD verifyUserCall2 = baseAddress + OFFSET_CONTACTS_VERIFY_USER_CALL_2;
DWORD verifyUserCall3 = baseAddress + OFFSET_CONTACTS_VERIFY_USER_CALL_3;
DWORD verifyUserCall4 = baseAddress + OFFSET_CONTACTS_VERIFY_USER_CALL_4;
DWORD verifyUserCall5 = baseAddress + OFFSET_CONTACTS_VERIFY_USER_CALL_5;
DWORD verifyUserCall = baseAddress + OFFSET_CONTACTS_VERIFY_USER_CALL;

#define OFFSET_CONTACTS_VERIFY_USER_HOOK_ADDRESS	0x1891C0
DWORD hookAddress = baseAddress + OFFSET_CONTACTS_VERIFY_USER_HOOK_ADDRESS;
DWORD jumpBackAddress = hookAddress + 5;

struct WxString
{
	wchar_t* ptr;
	int size;
	int capacity;
	int fill[2];
};

static wchar_t curWxId[80] = { 0 };

VOID StartVerifyUser(wchar_t* pWxid)
{
	if (wcslen(curWxId) > 0) return;
	wcscpy_s(curWxId, pWxid);

	WxString wxWxid = { 0 };
	WxString wxNULL = { 0 };
	wxWxid.ptr = pWxid;
	wxWxid.size = wcslen(pWxid);
	wxWxid.capacity = wcslen(pWxid) * 2;

	wchar_t* pVerifyContent = NULL;

	__asm
	{
		pushfd;
		pushad;

		sub esp, 18h;
		mov ecx, esp;
		lea eax, wxNULL;
		push eax;
		call verifyUserCall1;

		sub esp, 18h;
		mov ecx, esp;
		lea eax, wxNULL;
		push eax;
		call verifyUserCall2;

		push 6;

		sub esp, 14h;
		mov ecx, esp;
		push -0x1;
		lea eax, pVerifyContent;
		push eax;
		call verifyUserCall3;

		push 1; 

		sub esp, 14h;
		mov ecx, esp;
		lea eax, wxWxid;
		push eax;
		call verifyUserCall4;

		call verifyUserCall5;
		mov ecx, eax;
		call verifyUserCall;

		popad;
		popfd;
	}
}

void dealVerifyUserResult(DWORD result)
{
	if (wcslen(curWxId) <= 0) return;

	ContactStruct contact = GetWxNameByWxId(curWxId);
	wmemset(curWxId, 0x0, sizeof(curWxId) / sizeof(wchar_t));

	if (wcslen(contact.wcWxName) <= 0)
	{
		return;
	}

	wstring wsResult = L"";
	switch (result)
	{
	case 0xA7:
	{
		wsResult = L"É¾³ý";
	}
	break;
	case 0xA8:
	{
		wsResult = L"ºÃÓÑ";
	}
	break;
	case 0xAC:
	{
		wsResult = L"À­ºÚ";
	}
	break;

	default:
		break;
	}

	ContactInfoStruct sendInfo = { 0 };
	swprintf_s(sendInfo.wcWxId, L"%s", contact.wcWxId);
	swprintf_s(sendInfo.wcWxName, L"%s", contact.wcWxName);
	swprintf_s(sendInfo.wcCheckResult, L"%s", wsResult.c_str());
	SendCmdToMaster(CT_CONTACT_CHECK, sizeof(ContactInfoStruct), &sendInfo);
}

__declspec(naked) void doHookVerifyUserResult()
{
	__asm
	{
		push ebp;
		mov ebp, esp;
		push - 0x1;

		pushfd;
		pushad;

		push [ebp + 0x8];
		call dealVerifyUserResult;
		add esp, 0x4;

		popad;
		popfd;

		jmp jumpBackAddress;
	}
}

static CHAR oldCode[5] = { 0 };

VOID HookVerifyUserResult()
{
	BYTE hookCode[5] = { 0xE9 };
	*(DWORD*)&hookCode[1] = (DWORD)doHookVerifyUserResult - hookAddress - 5;

	ReadProcessMemory(GetCurrentProcess(), (LPVOID)hookAddress, oldCode, 5, 0);

	WriteProcessMemory(GetCurrentProcess(), (LPVOID)hookAddress, hookCode, 5, 0);
}

VOID UnHookVerifyUserResult()
{
	if (oldCode[0] == 0) return;

	WriteProcessMemory(GetCurrentProcess(), (LPVOID)hookAddress, oldCode, 5, 0);
	memset(oldCode, 0, 5);
}

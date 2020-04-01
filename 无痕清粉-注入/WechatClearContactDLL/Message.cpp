#include "stdafx.h"
#include "Message.h"
#include "Cmd.h"
#include "Util.h"
#include "Contacts.h"
#include <string>
using namespace std;

static wchar_t curWxId[80] = { 0 };
void SendMessageCard(const wchar_t* recieverWxId, const wchar_t* senderWxId, const wchar_t* NickName)
{
	wcscpy_s(curWxId, recieverWxId);

	struct wxString
	{
		const wchar_t * pStr;
		int strLen;
		int strMaxLen;
		char fillbuff[0x8];
	};

	wxString pWxid = { 0 };
	wxString pXml = { 0 };
	wchar_t xml[0x2000] = { 0 };
	pWxid.pStr = recieverWxId;
	pWxid.strLen = wcslen(recieverWxId);
	pWxid.strMaxLen = wcslen(recieverWxId) * 2;

	swprintf_s(xml, L"<?xml version=\"1.0\"?><msg bigheadimgurl=\"\" smallheadimgurl=\"\" username=\"%s\" nickname=\"%s\" fullpy=\"?\" shortpy=\"\" alias=\"%s\" imagestatus=\"3\" scene=\"17\" province=\"北京\" city=\"中国\" sign=\"\" sex=\"2\" certflag=\"0\" certinfo=\"\" brandIconUrl=\"\" brandHomeUrl=\"\" brandSubscriptConfigUrl= \"\" brandFlags=\"0\" regionCode=\"CN_BeiJing_BeiJing\" />", senderWxId, NickName, senderWxId);
	pXml.pStr = xml;
	pXml.strLen = wcslen(xml);
	pXml.strMaxLen = wcslen(xml) * 2;

	char buffer[0x20C] = { 0 };
	DWORD callAddress = (DWORD)GetModuleHandle(L"WeChatWin.dll") + OFFSET_MESSAGE_SEND_CARD_CALL;

	__asm {
		push 0x2A
		lea eax, pXml
		lea edx, pWxid
		push 0x0
		push eax
		lea ecx, buffer
		call callAddress
		add esp, 0xC
	}
}

DWORD recievedHookAddress = (DWORD)GetModuleHandle(L"WeChatWin.dll") + OFFSET_MESSAGE_RECEIVE_ADDRESS_HOOK;
DWORD recievedHookRetAddress = recievedHookAddress + 7;
CHAR originalRecieveCode[7] = { 0 };

void HookRecievedMessage()
{
	//组装数据
	BYTE bJmpCode[7] = { 0 };
	bJmpCode[0] = 0xE9;
	*(DWORD*)&bJmpCode[1] = (DWORD)ListenRecievedMessage - recievedHookAddress - 5;

	//保存当前位置的指令,在unhook的时候使用。
	ReadProcessMemory(GetCurrentProcess(), (LPVOID)recievedHookAddress, originalRecieveCode, 7, 0);

	//覆盖指令 B9 E8CF895C //mov ecx,0x5C89CFE8
	WriteProcessMemory(GetCurrentProcess(), (LPVOID)recievedHookAddress, bJmpCode, 7, 0);
}

void UnHookRecievedMessage()
{
	if (originalRecieveCode[0] == 0) return;

	WriteProcessMemory(GetCurrentProcess(), (LPVOID)recievedHookAddress, originalRecieveCode, 5, 0);
	memset(originalRecieveCode, 0, 7);
}

__declspec(naked) void ListenRecievedMessage()
{
	//保存现场
	__asm
	{
		pushad
		pushfd
	}

	DealRecievedMessage();

	//恢复现场
	__asm
	{
		popfd
		popad

		mov byte ptr ss : [ebp - 0x4], 0x1
		lea eax, dword ptr ss : [ebp - 0x24]
		jmp recievedHookRetAddress
	}
}

void DealRecievedMessage()
{
	if (wcslen(curWxId) <= 0) return;

	DWORD msgAddress = (DWORD)GetModuleHandle(L"WeChatWin.dll") + OFFSET_MESSAGE_RECEIVE_ADDRESS_DATA;
	if (msgAddress <= 0) return;

	msgAddress = *(DWORD*)msgAddress + 0x4;
	if (msgAddress <= 0) return;

	msgAddress = *(DWORD*)msgAddress;
	if (msgAddress <= 0) return;

	DWORD msgTypeAddress = msgAddress - 0x1B0;
	DWORD showerWxIdAddress = msgAddress - 0x1A0;
	DWORD messageDataAddress = msgAddress - 0x178;
	DWORD senderTypeAddress = msgAddress - 0x1AC;

	DWORD msgType = *(DWORD*)msgTypeAddress;
	DWORD senderType = *(DWORD*)senderTypeAddress;
	if (senderType > 3 or msgType <= 0) return;


	wstring showerWxId = GetWStringByAddress(showerWxIdAddress);
	wstring messageData = GetWStringByAddress(messageDataAddress);

	if (msgType == 0x2710 && 0 == wcscmp(showerWxId.c_str(), curWxId))
	{
		wstring wsResult;

		ContactStruct contact = GetWxNameByWxId(curWxId);
		wmemset(curWxId, 0x0, sizeof(curWxId) / sizeof(wchar_t));
		if (wcslen(contact.wcWxName) <= 0)
		{
			return;
		}

		if (messageData.find(L"消息已发出，但被对方拒收了") != messageData.npos)
		{
			wsResult = L"拉黑";

		}
		if (messageData.find(L"你还不是他（她）朋友") != messageData.npos)
		{
			wsResult = L"删除";
		}

		ContactInfoStruct sendInfo = { 0 };
		swprintf_s(sendInfo.wcWxId, L"%s", contact.wcWxId);
		swprintf_s(sendInfo.wcWxName, L"%s", contact.wcWxName);
		swprintf_s(sendInfo.wcCheckResult, L"%s", wsResult.c_str());
		SendCmdToMaster(CT_CONTACT_CHECK, sizeof(ContactInfoStruct), &sendInfo);
	}
}


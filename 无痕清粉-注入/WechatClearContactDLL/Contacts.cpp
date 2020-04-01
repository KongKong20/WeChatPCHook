#include "stdafx.h"
#include "Contacts.h"
#include "Util.h"
#include "Cmd.h"
#include <vector>
#include <string>
using namespace std;

vector<ContactStruct> contactList;

bool isFriend(wchar_t* UserName, int Type, int VerifyFlag)
{
	// 陌生人
	if (Type == 0) return false;

	// 群成员
	if (Type == 4) return false;

	// 黑名单
	if (Type == 10) return false;
	if (Type == 11) return false;

	// 公众号
	if (VerifyFlag > 0) return false;

	// 群聊
	if (Type == 2) return false;
	if (wcsstr(UserName, L"@chatroom") != NULL) return false;

	// 官方
	if (Type == 33) return false;
	if (wcsstr(UserName, L"tmessage") != NULL) return false;
	if (wcsstr(UserName, L"floatbottle") != NULL) return false;
	if (wcsstr(UserName, L"medianote") != NULL) return false;
	if (wcsstr(UserName, L"fmessage") != NULL) return false;
	if (wcsstr(UserName, L"fliehelper") != NULL) return false;

	return true;
}

void GetContactsList()
{
	contactList.clear();

	DWORD baseAddress = (DWORD)GetModuleHandle(TEXT("WeChatWin.dll"));
	DWORD Address1 = baseAddress + OFFSET_CONTACT_GLOBAL;
	DWORD Address2 = *(DWORD*)(Address1);
	DWORD Address3 = *(DWORD*)(Address2 + 0x28 + 0x84);

	vector<DWORD> nodeAddressList;
	nodeAddressList.push_back(Address3);

	DWORD nodeAddress1 = *(DWORD*)(Address3 + 0x0);
	DWORD nodeAddress2 = *(DWORD*)(Address3 + 0x4);
	DWORD nodeAddress3 = *(DWORD*)(Address3 + 0x8);
	if (find(nodeAddressList.begin(), nodeAddressList.end(), nodeAddress1) == nodeAddressList.end())
		nodeAddressList.push_back(nodeAddress1);
	if (find(nodeAddressList.begin(), nodeAddressList.end(), nodeAddress2) == nodeAddressList.end())
		nodeAddressList.push_back(nodeAddress2);
	if (find(nodeAddressList.begin(), nodeAddressList.end(), nodeAddress3) == nodeAddressList.end())
		nodeAddressList.push_back(nodeAddress3);

	DWORD index = 1;
	while (index < nodeAddressList.size())
	{
		DWORD nodeAddress = nodeAddressList[index++];
		DWORD checkNullResult = *(DWORD*)(nodeAddress + 0xD);
		if (checkNullResult == 0) {
			continue;
		}

		ContactStruct contact;
		swprintf_s(contact.wcWxId, L"%s", (WCHAR*)(*(DWORD*)(nodeAddress + 0x30)));
		swprintf_s(contact.wcWxName, L"%s", (WCHAR*)(*(DWORD*)(nodeAddress + 0x8C)));
		contact.type		= *(DWORD*)(nodeAddress + 0x70);
		contact.verifyFlag	= *(DWORD*)(nodeAddress + 0x74);

		if (isFriend(contact.wcWxId, contact.type, contact.verifyFlag))
		{
			contactList.push_back(contact);
		}

		DWORD nodeAddress1 = *(DWORD*)(nodeAddress + 0x0);
		DWORD nodeAddress2 = *(DWORD*)(nodeAddress + 0x4);
		DWORD nodeAddress3 = *(DWORD*)(nodeAddress + 0x8);
		if (find(nodeAddressList.begin(), nodeAddressList.end(), nodeAddress1) == nodeAddressList.end())
			nodeAddressList.push_back(nodeAddress1);
		if (find(nodeAddressList.begin(), nodeAddressList.end(), nodeAddress2) == nodeAddressList.end())
			nodeAddressList.push_back(nodeAddress2);
		if (find(nodeAddressList.begin(), nodeAddressList.end(), nodeAddress3) == nodeAddressList.end())
			nodeAddressList.push_back(nodeAddress3);
	}

	for (vector<ContactStruct>::iterator iter = contactList.begin();
		iter != contactList.end();
		iter++)
	{
		ContactStruct contact = *iter;
		ContactInfoStruct sendInfo = { 0 };
		swprintf_s(sendInfo.wcWxId, L"%s", contact.wcWxId);
		swprintf_s(sendInfo.wcWxName, L"%s", contact.wcWxName);
		swprintf_s(sendInfo.wcCheckResult, L"%s", L"等待");
		SendCmdToMaster(CT_CONTACT_LIST, sizeof(ContactInfoStruct), &sendInfo);
	}
}

ContactStruct GetWxNameByWxId(wchar_t* pWxId)
{
	for (vector<ContactStruct>::iterator iter = contactList.begin();
		iter != contactList.end();
		iter++)
	{
		ContactStruct contact = *iter;
		if (0 == wcscmp(contact.wcWxId, pWxId)) return contact;
	}

	return ContactStruct();
}

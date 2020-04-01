#pragma once
#include <string>
using namespace std;

#pragma once
#include <string>
using namespace std;

#define SLAVE_NAME				"Slave"
#define MASTER_NAME				"Master"

#define CT_CONTACT_LIST					1
#define CT_CONTACT_CHECK				2
#define CT_FRIEND_DEL					3

struct ContactInfoStruct
{
	wchar_t wcWxId[80];
	wchar_t wcWxName[80];
	wchar_t wcCheckResult[80];
};

struct FriendOperStruct
{
	wchar_t wcWxId[80];
	wchar_t param[50];
};

void SendCmdToSlave(ULONG_PTR dwData, DWORD cbData, PVOID lpData);
void DealCmdFromSlave(ULONG_PTR dwData, DWORD cbData, PVOID lpData);


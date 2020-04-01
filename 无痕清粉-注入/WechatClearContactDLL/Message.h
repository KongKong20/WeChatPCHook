#pragma once
#define OFFSET_MESSAGE_SEND_CARD_CALL		0x02EB4E0	//发送名片
#define OFFSET_MESSAGE_RECEIVE_ADDRESS_HOOK	0x030F337	//接收消息
#define OFFSET_MESSAGE_RECEIVE_ADDRESS_DATA	0x127F72C	//接收消息

void SendMessageCard(const wchar_t* recieverWxId, const wchar_t* senderWxId, const wchar_t* NickName);

void HookRecievedMessage();
void ListenRecievedMessage();
void DealRecievedMessage();
void UnHookRecievedMessage();

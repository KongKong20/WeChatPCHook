#pragma once
#include "stdafx.h"

void InitMsgWnd(HMODULE hModule);			//初始化窗口
void RegisterMsgWnd(HMODULE hModule);		//注册窗口
LRESULT CALLBACK WndProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);	//窗口回调

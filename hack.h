#pragma once
#include "stdafx.h"

class hack {
public:
	HANDLE process_h;
	DWORD pid;
	HWND cs_hwnd;
	bool initialise() {
		cs_hwnd = FindWindowA(NULL, "Counter-Strike: Global Offensive");
		if (!cs_hwnd) {
			std::cout << "Are you running CSGO?\n";
			return false;
		}
		GetWindowThreadProcessId(cs_hwnd, &pid);
		if (pid < 0) {
			return false;
		}
		process_h = OpenProcess(PROCESS_ALL_ACCESS, 0, pid);
		if (!process_h || process_h == INVALID_HANDLE_VALUE) {
			std::cout << "Try running as admin\n";
			return false;
		}
		return true;
	}
	DWORD_PTR dwGetModuleBaseAddress(DWORD dwProcID, TCHAR *szModuleName)
	{
		MODULEENTRY32 lpModuleEntry = { 0 };
		HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwProcID);
		if (!hSnapShot || hSnapShot == INVALID_HANDLE_VALUE)
		{
			return NULL;
		}
		lpModuleEntry.dwSize = sizeof(lpModuleEntry);
		BOOL bModule = Module32First(hSnapShot, &lpModuleEntry);
		while (bModule)
		{
			do
			{
				Module32Next(hSnapShot, &lpModuleEntry); //It always skip the first module in CSGO theres no problem, but it's bad practice.
			} while (strcmp(lpModuleEntry.szModule, szModuleName));
			CloseHandle(hSnapShot);
			return reinterpret_cast<DWORD>(lpModuleEntry.modBaseAddr);

		}
		CloseHandle(hSnapShot);
	}	

};

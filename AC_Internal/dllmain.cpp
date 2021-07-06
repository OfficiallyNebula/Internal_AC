// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include <iostream>
#include "mem.h"

DWORD WINAPI HackThread(HMODULE hModule)
{
	//Create Console
	AllocConsole();
	FILE* f;
	freopen_s(&f, "CONOUT$", "w", stdout);

	std::cout << "Nebula's Trainer (Kinda)\n" << 
		"NUMPAD 1) Godmode\n" << 
		"NUMPAD 2) Infinite Ammo\n" << 
		"NUMPAD 3) No Recoil\n";

	uintptr_t moduleBase = (uintptr_t)GetModuleHandle(L"ac_client.exe");

	//calling it with NULL also gives you the address of the .exe module
	moduleBase = (uintptr_t)GetModuleHandle(NULL);

	bool bHealth = false, bAmmo = false, bRecoil = false;

	while (true)
	{
		if (GetAsyncKeyState(VK_END) & 1)
		{
			break;
		}

		if (GetAsyncKeyState(VK_NUMPAD1) & 1)
		{
			bHealth = !bHealth;
			if (bHealth) 
			{
				std::cout << "GodMode Enabled";
			}
			else {
				std::cout << "GodMode Disabled";
			}
		}
		

		if (GetAsyncKeyState(VK_NUMPAD2) & 1)
		{
			bAmmo = !bAmmo;
		}

		//no recoil NOP
		if (GetAsyncKeyState(VK_NUMPAD3) & 1)
		{
			bRecoil = !bRecoil;

			if (bRecoil)
			{
				mem::Nop((BYTE*)(moduleBase + 0x63786), 10);
				std::cout << "No Recoil Enabled\n";
			}

			else
			{
				//50 8D 4C 24 1C 51 8B CE FF D2 the original stack setup and call
				mem::Patch((BYTE*)(moduleBase + 0x63786), (BYTE*)"\x50\x8D\x4C\x24\x1C\x51\x8B\xCE\xFF\xD2", 10);
				std::cout << "No Recoil Disabled\n";
			}
		}

		//need to use uintptr_t for pointer arithmetic later
		uintptr_t* localPlayerPtr = (uintptr_t*)(moduleBase + 0x10F4F4);

		//continuous writes / freeze

		if (localPlayerPtr)
		{
			if (bHealth)
			{

				//*localPlayerPtr = derference the pointer, to get the localPlayerAddr
				// add 0xF8 to get health address
				//cast to an int pointer, this pointer now points to the health address
				//derference it and assign the value 1337 to the health variable it points to
				*(int*)(*localPlayerPtr + 0xF8) = 1337;
			}

			if (!bHealth)
			{
				*(int*)(*localPlayerPtr + 0xF8) = 100;
			}

			if (bAmmo)
			{
				//We aren't external now, we can now efficiently calculate all pointers dynamically
				//before we only resolved pointers when needed for efficiency reasons
				//we are executing internally, we can calculate everything when needed
				uintptr_t ammoAddr = mem::FindDMAAddy(moduleBase + 0x10F4F4, { 0x374, 0x14, 0x0 });
				int* ammo = (int*)ammoAddr;
				*ammo = 1337;

				//or just
				*(int*)mem::FindDMAAddy(moduleBase + 0x10F4F4, { 0x374, 0x14, 0x0 }) = 1337;
			}

		}
		Sleep(5);
	}

	fclose(f);
	FreeConsole();
	FreeLibraryAndExitThread(hModule, 0);
	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)HackThread, hModule, 0, nullptr));
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
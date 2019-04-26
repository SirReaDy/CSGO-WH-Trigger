#include "stdafx.h"
//////////DECLARATIONS//////////
hack hacks;
Offsets off;
int shot = 5;
int nShot = 4;
bool shoot = false;

//////////END DECLARATIONS//////


 /////////// TEMPLATES ////////////////////
template<typename TYPE>
TYPE ReadMemory(HANDLE proc, DWORD address) {
	TYPE buffer;
	ReadProcessMemory(proc, (LPCVOID)address, &buffer, sizeof(buffer), 0);
	return buffer;
}

template<typename TYPE>
void WriteMemory(HANDLE proc, DWORD address, TYPE dataToWrite) {
	TYPE buffer = dataToWrite;
	WriteProcessMemory(proc, (LPVOID)address, &buffer, sizeof(buffer), 0);
}
///////////// TEMPLATES_END ////////////////

class Information {
private:

	struct MyPlayer {
		int iCrosshairID;
		int iTeam;
		DWORD localplayer;
		int iFlags;
	}MyPlayer;

	struct PlayerList {
		DWORD dwCurrentEntity;
		int iTeam;
		bool dormant;
		int GlowIndex;
	}PlayerList[32];

public:
	struct glow_s
	{
		DWORD dwBase;
		float r;
		float g;
		float b;
		float a;
		uint8_t unk1[16];
		bool m_bRenderWhenOccluded;//rwo
		bool m_bRenderWhenUnoccluded;//rwuo
		bool m_bFullBloom;
		uint8_t unk2[10];
	};

	void ActivateGlow(Information::glow_s glowObject, int i, DWORD dwGlowPointer,float In_r,float In_g,float In_b) {
		glowObject.r = In_r;
		glowObject.b = In_b;
		glowObject.g = In_g;
		glowObject.a = 1.0f;
		glowObject.m_bRenderWhenOccluded = true;
		glowObject.m_bRenderWhenUnoccluded = false;
		WriteMemory(hacks.process_h, dwGlowPointer + i * 0x38, glowObject);
	}

	void MY_ReadInformation(DWORD client) //Reads our player information
	{
		MyPlayer.localplayer = ReadMemory<DWORD>(hacks.process_h, client + off.local_player); //LocalPlayer
		MyPlayer.iCrosshairID = ReadMemory<int>(hacks.process_h, MyPlayer.localplayer + off.crosshairID);//CrosshairID
		MyPlayer.iTeam = ReadMemory<int>(hacks.process_h, MyPlayer.localplayer+0xF0); //Team
		MyPlayer.iFlags = ReadMemory<int>(hacks.process_h, MyPlayer.localplayer + 0x100);//Flags as in-air,crouched,going crouch
	}

	bool Player_ReadInformation(DWORD client,int i) {//Reads our player information
		PlayerList[i].dwCurrentEntity = ReadMemory<DWORD>(hacks.process_h,(client +off.entity_list)+(i*0x10));
		if (PlayerList[i].dwCurrentEntity == NULL) return false;//If there are no more players return
		PlayerList[i].iTeam = ReadMemory<int>(hacks.process_h, PlayerList[i].dwCurrentEntity + 0xF0);
		PlayerList[i].dormant = ReadMemory<bool>(hacks.process_h, PlayerList[i].dwCurrentEntity + off.dormant);
		return true;
	}

	void trigger(DWORD client) {
		if (MyPlayer.iFlags == 256) {
			return;
		}
		if (!shoot) //Shoot starts false
		{
			WriteMemory(hacks.process_h, client + off.forceattack, nShot);
			shoot = !shoot;
			return;
		}
		if (MyPlayer.iCrosshairID == 0) return;

		if (PlayerList[MyPlayer.iCrosshairID - 1].iTeam == MyPlayer.iTeam) return;

		if (shoot)
		{
			WriteMemory(hacks.process_h, client + off.forceattack, shot);
			shoot = !shoot;
			return;
		}
	}
	void glow(DWORD client_dll,int i) {
		DWORD dwGlowPointer = ReadMemory<DWORD>(hacks.process_h,client_dll + off.glowObject); //The Adress of the glow object
		int iObjectCount = ReadMemory<DWORD>(hacks.process_h, client_dll + off.glowObject + 0x4); //Object count
		if (dwGlowPointer != NULL && iObjectCount > 0) {
			for (int t = 0; t < iObjectCount; t++)
			{
				Information::glow_s glowObject = ReadMemory<Information::glow_s>(hacks.process_h, dwGlowPointer+t*0x38);
				if (glowObject.dwBase == NULL)
				{
					continue;
				}
				if (glowObject.dwBase == PlayerList[i].dwCurrentEntity) //If it's a player
				{


					if (PlayerList[i].iTeam == MyPlayer.iTeam  && PlayerList[i].dormant == 0)
					{
						ActivateGlow(glowObject,t,dwGlowPointer,0.0f,0.0f,95.0f); //r,g,b
					}
					if (PlayerList[i].iTeam != MyPlayer.iTeam  && PlayerList[i].dormant == 0) {
						ActivateGlow(glowObject, t, dwGlowPointer, 95.0f, 0.0f, 0.0f); //r,g,b
					}


				}
				
			}
		}
	}
};
Information info;

int main() {
	std::cout << "*********************Simple external****************************\n";
	std::cout << "***************Press F1 for triggerbot*************************\n";
	std::cout << "***************Press F2 for glow*******************************\n";
	std::cout << "***************************************************************\n";
	std::cout << "***************************************************************\n";
	if (hacks.initialise())
	{

		bool trigger = false;
		bool glow = false;
		DWORD dwClient = hacks.dwGetModuleBaseAddress(hacks.pid, "client.dll");
		while (1) {
			if (GetKeyState(VK_F1)) {
				trigger = !trigger;
			}
			info.MY_ReadInformation(dwClient); //Reads our information

			for (int i = 0; i < 32; i++) 
			{

				if (!info.Player_ReadInformation(dwClient, i)) //If it returned NULL then break the FOR loop
				{
					break;
				}
				if (GetKeyState(VK_F2)) {
					glow = !glow;
				}
				if (glow)
				{
					info.glow(dwClient, i);
				}

			}

			if (trigger)
			{
				info.trigger(dwClient);
			}

			Sleep(1);
				  }	 
		
	}

	else {
		std::cout << "Ops! Something went wrong pal\n";
	}
}
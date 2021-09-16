#include <iostream>
#include <vector>
#include <windows.h>
#include <TlHelp32.h>
#include <cmath>

#define PI 3.14159265358979323846

struct vect3 { float x, y, z; };
struct entity {
	uintptr_t addr = 0x0;
	uintptr_t pos[3] = { };
	uintptr_t head[2] = {};
	uintptr_t team = 0x0;
	uintptr_t attack = 0x0;
	uintptr_t health = 0x0;
	vect3* coord = new vect3();
};

float pX = 2.0f * cos(0.0f); // POV-Direction Vector (2.0 radius is arbitrary)
float pY = 2.0f * sin(0.0f);

unsigned int amount = 14;

DWORD getProcessId(const wchar_t* procName) {
	DWORD procId = 0; //Process Id

	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); //Snapshot of process's in memory
	if (hSnap != INVALID_HANDLE_VALUE) {
		PROCESSENTRY32 procEntry;
		procEntry.dwSize = sizeof(procEntry);

		if (Process32First(hSnap, &procEntry)) {
			do {
				if (!_wcsicmp(procEntry.szExeFile, procName)) {
					procId = procEntry.th32ProcessID;
					break;
				}
			} while (Process32Next(hSnap, &procEntry));
		}
	}
	CloseHandle(hSnap);
	return procId;
}
uintptr_t getModuleBaseAddress(DWORD procId, const wchar_t* modName) {
	uintptr_t moduleBaseAddress = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);

	if (hSnap != INVALID_HANDLE_VALUE) {
		MODULEENTRY32 moduleEntry;
		moduleEntry.dwSize = sizeof(moduleEntry);
		
		if (Module32First(hSnap, &moduleEntry)) {
			do {
				if (!_wcsicmp(moduleEntry.szModule, modName)) {
					moduleBaseAddress = (uintptr_t)moduleEntry.modBaseAddr;
					break;
				}
			} while (Module32Next(hSnap, &moduleEntry));
		}
	}
	CloseHandle(hSnap);
	return moduleBaseAddress;
}
uintptr_t findDVMA(HANDLE hProc, uintptr_t modBaseAddress, std::vector<unsigned int> offsets) {
	uintptr_t healthAddr = modBaseAddress;

	for (unsigned int i = 0; i < offsets.size(); i++) {
		ReadProcessMemory(hProc, (BYTE*)healthAddr, &healthAddr, sizeof(healthAddr), 0);
		healthAddr += offsets[i];
	}
	return healthAddr;
}

std::vector<entity> getEntityAddresses(HANDLE hProc, uintptr_t entityBasePtr, unsigned int n) {		//maybe vector with entity pointers
	std::vector<entity> entities;
	uintptr_t currentEntityOffset = entityBasePtr;

	for (unsigned int i = 0; i < n; i++) {
		currentEntityOffset = i * 0x4;
		entities.push_back(entity());
		entities[i].addr = findDVMA(hProc, entityBasePtr, { currentEntityOffset });
	}
	return entities;
}
void setEntityAddrByOffset(HANDLE hProc, std::vector<entity>& ent, std::vector<unsigned int> offsets) {
	for (unsigned int i = 0; i < ent.size(); i++) {
		ent[i].pos[0] = findDVMA(hProc, ent[i].addr, { offsets[0] });
		ent[i].pos[1] = findDVMA(hProc, ent[i].addr, { offsets[1] });
		ent[i].pos[2] = findDVMA(hProc, ent[i].addr, { offsets[2] });
	}
}

float calcDistance(vect3* localPos, vect3* enemyPos) {
	float dX = enemyPos->x - localPos->x;
	float dY = enemyPos->y - localPos->y;
	return sqrt(dX * dX + dY * dY);
}
std::vector<float> calcHeadRot(vect3* localPos, vect3* enemyPos) {	//aimbot
	std::vector<float> resultRotations;
	//Calculate X-Rotation:
	bool left = false, front = false;
	float rotX = 0.0f;

	float pToEX = enemyPos->x - localPos->x;    //Vector from Player to Enemy
	float pToEY = enemyPos->y - localPos->y;
	front = pToEX < 0 ? true : false;

	float dEnemy = sqrt((pToEX * pToEX) + (pToEY * pToEY));

	if (pToEY == 0) {
		if (!front) { rotX = 180.0f; }
		else { rotX = 0.0f; }
	}
	else {
		left = pToEY < 0 ? true : false;
		//Calculate Angle between POV-Vector and Enemy From Player Vector
		//Formula -> ((povVectorPlayer*vectorPlayerToEnemy)/ sqrt(povVectorPlayer.x^2 + povVectorPlayer.y^2) * sqrt(vectorPlayerToEnemy.x^2 + vectorPlayerToEnemy.y^2))
		float dPlayer = sqrt((pX * pX) + (pY * pY));
		rotX = acos(((pX * pToEX) + (pY * pToEY)) / (dPlayer * dEnemy));
	}
	//Convert to Degrees
	rotX *= (float)(180.0f / PI);
	rotX = left ? fmod(rotX - 180.0f, 180.0f) : fmod(rotX - 180.0f, -180.0f);	//Change to diagonal angle due to inverse in movement ingame
	rotX = !left ? rotX * -1.0f : rotX;
	resultRotations.push_back(rotX);

	//Calculate Z-Rotation:
	float dZ = enemyPos->z - localPos->z;
	float rotZ = atan(dZ / dEnemy);		//(Difference | length in Z) / (Difference | length in X and Y) 
	//Convert to Degrees
	rotZ *= (float)(180 / PI);
	resultRotations.push_back(rotZ);

	return resultRotations;
}



int main(){
	bool bHealth = false, bAmmo = false, bAimbot = false, bNoclip = false;

	std::cout << "Searching Process ID..." << std::endl;
	DWORD procId = getProcessId(L"ac_client.exe");	//process id
	std::cout << "Process found!\n" << std::endl;

	std::cout << "********************************" << std::endl;

	std::cout << "\nSearching Module..." << std::endl;
	uintptr_t modBaseAddress = getModuleBaseAddress(procId, L"ac_client.exe");  //module base address
	std::cout << "Module found wiht Module-Base-Address: " << std::hex << modBaseAddress  << std::endl;

	HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, NULL, procId);	//open a handle of processid
	std::cout << "\nProcess opened \"ALL ACCESS\" Permissions \n" << std::endl;

	std::cout << "********************************\n" << std::endl;

	if (hProc) {
		std::cout << "Finding current Addresses..." << std::endl;

		std::vector<unsigned int> offsets = { 0xF8 };
		uintptr_t dynamicPlayerPtr = modBaseAddress + 0x10F4F4;



		//Health
		uintptr_t healthAddr = findDVMA(hProc, dynamicPlayerPtr, offsets);	//finding current health address
		int healthValue = 0;
		ReadProcessMemory(hProc, (BYTE*)healthAddr, &healthValue, sizeof(healthValue), nullptr);	//outputting current health
		std::cout << "Current Health: " << std::dec << healthValue << "\n" << std::endl;



		//Ammo
		std::vector<unsigned int> offsetsAmmo = { 0x374, 0x14, 0x0 };
		int ammoValue = 0;
		uintptr_t ammoAddr = 0;
		
		int localNoclipOn = 11;
		int localNoclipOff = 0;
		int localInvisOn = 5;
		int localInvisOff = 0;

		//Local Player
		struct entity localPlayer;
		uintptr_t localPlayerAddr = 0;
		ReadProcessMemory(hProc, (BYTE*)dynamicPlayerPtr, &localPlayerAddr, sizeof(localPlayerAddr), nullptr);
		uintptr_t localTeamPtr = localPlayerAddr + 0x32C;
		uintptr_t localNoclip = localPlayerAddr + 0x82;
		uintptr_t localInvis = localPlayerAddr + 0x338;

		ReadProcessMemory(hProc, (BYTE*)localTeamPtr, &localPlayer.team, sizeof(localPlayer.team), nullptr);	//very unnecessary
		localPlayer.attack = findDVMA(hProc, dynamicPlayerPtr, { 0x224 });
		localPlayer.pos[0] = findDVMA(hProc, dynamicPlayerPtr, { 0x38 });
		localPlayer.pos[1] = findDVMA(hProc, dynamicPlayerPtr, { 0x34 });
		localPlayer.pos[2] = findDVMA(hProc, dynamicPlayerPtr, { 0x3c });
		localPlayer.head[0] = findDVMA(hProc, dynamicPlayerPtr, { 0x40 });
		localPlayer.head[1] = findDVMA(hProc, dynamicPlayerPtr, { 0x44 });



		//Enemy Entity Pos
		uintptr_t dynamicEntityPtr = modBaseAddress + 0x10f4f8;
		
		std::vector<entity> entityAddr = getEntityAddresses(hProc, dynamicEntityPtr, amount);
		setEntityAddrByOffset(hProc, entityAddr, { 0x38, 0x34, 0x3C });
		
		//unsigned int entTeam = 0;
		/*
		for (unsigned int i = 0; i < amount-1; i++) {
			entityAddr[i].team = findDVMA(hProc, entityAddr[i].addr, { 0x32C });
			ReadProcessMemory(hProc, (BYTE*)entityAddr[i].team, &entTeam, sizeof(entTeam), nullptr);

			if (entTeam == localPlayer.team) {
				entityAddr.erase(entityAddr.begin()+i);
				continue;
			}
			std::cout << "Entity Team: " << std::dec << entTeam << std::endl;
			std::cout << "Local Team: " << std::dec << localPlayer.team << std::endl;
		}
		*/
		//unsigned int entHealth = 0;
		for (unsigned int i = 0; i < amount - 1; i++) {
			entityAddr[i].health = findDVMA(hProc, entityAddr[i].addr, { 0xf8 });
			//ReadProcessMemory(hProc, (BYTE*)entityAddr[i].health, &entHealth, sizeof(entHealth), nullptr);
		}

		//[TO-DO] add to player struct
		DWORD dwExit = 0;
		int newHealth = 1337;
		int newAmmo = 1337;
		unsigned int localShoot = 0;
		unsigned int bAttackIdle = 0;
		WriteProcessMemory(hProc, (BYTE*)localPlayer.attack, &localShoot, sizeof(localShoot), nullptr);
		ReadProcessMemory(hProc, (BYTE*)localPlayer.attack, &bAttackIdle, sizeof(bAttackIdle), nullptr);
		unsigned int bAttackRelative = 0;

		std::vector<float> rot;

		float distance;
		float min;
		unsigned int index;
		unsigned int entHealth;

		std::cout << "********************************" << std::endl;
		std::cout << "\nFully Compiled.\nHack ready!\n" << std::endl;
		std::cout << "\n********************************\n" << std::endl;
		std::cout << "Feature History:\n" << std::endl;


		while (GetExitCodeProcess(hProc, &dwExit) && dwExit == STILL_ACTIVE) {
			if(GetAsyncKeyState(VK_NUMPAD1) & 1){
				bHealth = !bHealth;
				if (bHealth) {
					WriteProcessMemory(hProc, (BYTE*)healthAddr, &newHealth, sizeof(newHealth), nullptr);
					std::cout << "\nHEALTH RESTORED" << std::endl;
				}
			}
			if (GetAsyncKeyState(VK_NUMPAD2) & 1) {
				bAmmo = !bAmmo;
				if (bAmmo) {
					ammoAddr = findDVMA(hProc, dynamicPlayerPtr, offsetsAmmo);
					WriteProcessMemory(hProc, (BYTE*)ammoAddr, &newAmmo, sizeof(newAmmo), nullptr);
					std::cout << "\AMMO REFILLED" << std::endl;
				}
			}
			if (GetAsyncKeyState(VK_NUMPAD3) & 1) {
				bAimbot = !bAimbot;

				std::cout << "\nAimbot [ACTIVE]" << std::endl;

				while (bAimbot) {
					while (bAimbot) {
						if (GetAsyncKeyState(VK_NUMPAD3) & 1) {
							bAimbot = !bAimbot;
							std::cout << "AIMBOT [DISABLED]\n" << std::endl;
							break;
						}
						ReadProcessMemory(hProc, (BYTE*)localPlayer.attack, &bAttackRelative, sizeof(bAttackRelative), nullptr);
						if (bAttackRelative - bAttackIdle == 1) { break; }
						//std::cout << std::dec << bAttackRelative - bAttackIdle << std::endl;
						Sleep(10);
					}
					while (bAttackRelative - bAttackIdle == 1 && bAimbot) {
						ReadProcessMemory(hProc, (BYTE*)localPlayer.attack, &bAttackRelative, sizeof(bAttackRelative), nullptr);
						//Get Current Player Position
						ReadProcessMemory(hProc, (BYTE*)localPlayer.pos[0], &localPlayer.coord->x, sizeof(localPlayer.coord->x), nullptr);
						ReadProcessMemory(hProc, (BYTE*)localPlayer.pos[1], &localPlayer.coord->y, sizeof(localPlayer.coord->y), nullptr);
						ReadProcessMemory(hProc, (BYTE*)localPlayer.pos[2], &localPlayer.coord->z, sizeof(localPlayer.coord->z), nullptr);

						//Get Entity with least distance to Player
						distance = 0.0f;
						min = 10000.0f;
						index = 0;
						for (unsigned int i = 0; i < entityAddr.size(); i++) {
							ReadProcessMemory(hProc, (BYTE*)entityAddr[i].pos[0], &entityAddr[i].coord->x, sizeof(entityAddr[i].coord->x), nullptr);
							ReadProcessMemory(hProc, (BYTE*)entityAddr[i].pos[1], &entityAddr[i].coord->y, sizeof(entityAddr[i].coord->y), nullptr);
							ReadProcessMemory(hProc, (BYTE*)entityAddr[i].pos[2], &entityAddr[i].coord->z, sizeof(entityAddr[i].coord->z), nullptr);
							distance = calcDistance(localPlayer.coord, entityAddr[i].coord);

							ReadProcessMemory(hProc, (BYTE*)entityAddr[i].health, &entHealth, sizeof(entHealth), nullptr);	//check if still alive
							if (distance < min && entHealth <= 100) {		//Health value increases to MAX_VALUE when entity dies
								min = distance;
								index = i;
							}
						}
						rot = calcHeadRot(localPlayer.coord, entityAddr[index].coord);
						//update head angle 
						WriteProcessMemory(hProc, (BYTE*)localPlayer.head[0], &rot[0], sizeof(rot[0]), nullptr);
						WriteProcessMemory(hProc, (BYTE*)localPlayer.head[1], &rot[1], sizeof(rot[1]), nullptr);
						Sleep(50);
					}
				}
			}
			if (GetAsyncKeyState(VK_NUMPAD4) & 1) {
				bNoclip = !bNoclip;
				

				if (bNoclip) {
					WriteProcessMemory(hProc, (BYTE*)localInvis, &localInvisOn, sizeof(localInvisOn), nullptr);
					WriteProcessMemory(hProc, (BYTE*)localNoclip, &localNoclipOn, sizeof(localNoclipOn), nullptr);
					std::cout << "\nNOCLIP [ENABLED]" << std::endl;
				}
				else {
					WriteProcessMemory(hProc, (BYTE*)localInvis, &localInvisOff, sizeof(localInvisOff), nullptr);
					WriteProcessMemory(hProc, (BYTE*)localNoclip, &localNoclipOff, sizeof(localNoclipOff), nullptr);
					std::cout << "NOCLIP [DISABLED]" << std::endl;
				}
			}
			//[IDEA] Portal Gun: Use Trace Line Function to detect collision and teleport to collision coordinates
			if (GetAsyncKeyState(VK_NUMPAD5) & 1) {
				for (unsigned int i = 0; i < entityAddr.size(); i++) {
					ReadProcessMemory(hProc, (BYTE*)entityAddr[i].pos[0], &entityAddr[i].coord->x, sizeof(entityAddr[i].coord->x), nullptr);
					ReadProcessMemory(hProc, (BYTE*)entityAddr[i].pos[1], &entityAddr[i].coord->y, sizeof(entityAddr[i].coord->y), nullptr);
					ReadProcessMemory(hProc, (BYTE*)entityAddr[i].pos[2], &entityAddr[i].coord->z, sizeof(entityAddr[i].coord->z), nullptr);
					std::cout << std::dec << entityAddr[i].coord->x << std::endl;
				}
				ReadProcessMemory(hProc, (BYTE*)localPlayer.attack, &bAttackRelative, sizeof(bAttackRelative), nullptr);
				std::cout << std::dec << bAttackRelative - bAttackIdle << std::endl;
				std::cout << std::hex << localPlayer.attack << std::endl;
			}
			if (GetAsyncKeyState(VK_NUMPAD8) & 1) {
				return 0;
			}
			Sleep(25);
		}
	}
	else {
		std::cout << "Process could not be opened\nPress any button to exit" << std::endl;
		(void)getchar();
		return 0;
	}

	(void)getchar();
	return 0;
}
#pragma once
#include <Windows.h>
#include "geom.h"

enum gameModes {
	TEAMDEATHMATCH = 0,
	COOPEDIT,
	DEATHMATCH,
	SURVIVOR,
	TEAMSURVIVOR,
	CTF,
	PISTOLFRENY,
	BOTTEAMDEATHMATCH,
	BOTDEATHMATCH,
	LASTSWISSSTANDING,
	ONESHOTONEKILL,
	TEAMONESHOTONEKILL,
	BOTONESHOTONEKILL,
	HUNTTHEFLAG,
	TEAMKEEPTHEFLAG,
	KEEPTHEFLAG,
	TEAMPF,
	TEAMLSS,
	BOTPISTOLFRENZY,
	BOtlSS,
	BOTTEAMSURVIVOR,
	BOTTEAMONESHOTONEKILL,
	NUM
};

class ent;

class weapon {
public:
	char pad_0x0000[0x4];
	__int32 ID;
	ent* owner;
	uintptr_t* guninfo;
	int* ammo2;
	int* ammo;
	int* gunWait;
	int shots;
	int breload;
};

class ent {
public:
	DWORD vTable;
	Vector3 head;
	char _0x0010[36];
	Vector3 pos;
	Vector3 angle;
	char _0x004c[37];
	BYTE bScoping;
	char _0x0072[134];
	__int32 health;
	__int32 armor;
	char _0x0100[292];
	BYTE bAttacking;
	char name[16];
	char _0x0235[247];
	BYTE team;
	char _0x032D[11];
	BYTE state;
	char _0x0339[59];
	weapon* weapon;
	char _0x0378[520];
};

struct entList {
	ent* ents[31];
};
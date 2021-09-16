#pragma once
#include "assaultcube.h"
#include "OpenGL.h"
#include "geom.h"

//Screen Scaling
const int VIRTUAL_SCREEN_WIDTH = 800;
const int GAME_UNIT_MAGIC = 400;	// Game Unit

const float PLAYER_HEIGHT = 5.25f;
const float PLAYER_WIDTH = 2.0f;
const float EYE_HEIGHT = 4.5f;

const float PLAYER_ASPECT_RATIO = PLAYER_HEIGHT / PLAYER_WIDTH;

const int ESP_FONT_HEIGHT = 15;
const int ESP_FONT_WIDTH = 9;


class ESP {
public:
	int* gameMode = (int*)(0x50F49C);
	int* numOfPlayers = (int*)(0x50F500);
	float* matrix = (float*)(0x501AE8);
	ent* localPlayer = *(ent**)0x50F4F4;
	entList* entlist = *(entList**)0x50F4F8;

	int viewport[4];

	Vector3 direct;
	//please dont hate on me
	ent localCopyEntry = *localPlayer;
	ent localCopyExit = *localPlayer;

	ent* entryPortal = &localCopyEntry;
	ent* exitPortal = &localCopyExit;
	bool bEntry = true;
	bool bExit = false;

	bool entry = false;
	bool exit = false;
	
	bool isTeamGame();
	bool isEnemy(ent* e);
	bool isValidEnt(ent* ent);

	void DrawESPBox(ent* p, Vector3 screen, GLText::Font& font);
	void checkTeleport(ent* p1, ent* p2);
	void Draw(GLText::Font& font, int dist);
};


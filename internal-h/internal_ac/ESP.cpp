#include "pch.h"
#include <cmath>
#include "ESP.h"

bool ESP::isTeamGame()
{
    if ((*gameMode == 0 || *gameMode == 4 || *gameMode == 5 || *gameMode == 7 || *gameMode == 11 || *gameMode == 13 ||
        *gameMode == 14 || *gameMode == 16 || *gameMode == 17 || *gameMode == 20 || *gameMode == 21))
        return true;
    else return false;
}

bool ESP::isEnemy(ent* e)
{
    if (localPlayer->team == e->team)
        return false;
    else return true;
}

bool ESP::isValidEnt(ent* ent)
{
    if (ent)
    {
        if (ent->vTable == 0x4E4A98 || ent->vTable == 0x4E4AC0)
            return true;
    }
    return false;
}

void ESP::DrawESPBox(ent* e, Vector3 screen, GLText::Font& font)
{
    const GLubyte* color = nullptr;

    if (isTeamGame() && !isEnemy(e))
        color = rgb::green;
    else color = rgb::red;

    float dist = localPlayer->pos.Distance(e->pos);

    // Scaling ESP-Box
    float scale = (GAME_UNIT_MAGIC / dist) * (viewport[2] / VIRTUAL_SCREEN_WIDTH);
    float x = screen.x - scale;
    float y = screen.y - scale * PLAYER_ASPECT_RATIO;
    float width = scale * 2;
    float height = scale * PLAYER_ASPECT_RATIO * 2;

    GL::DrawOutline(x, y, width, height, 2.0f, color);

    float textX = font.centerText(x, width, strlen(e->name) * ESP_FONT_WIDTH);
    float textY = y - ESP_FONT_HEIGHT / 2;
    font.Print(textX, textY, color, "%s", e->name);
}

void ESP::checkTeleport(ent* p1, ent* p2) {
    float radius = 5;
    if (entry && exit) {
        if (p1->pos.Distance(p2->pos) > 10) {       // Minimum Portal Distance
            if (localPlayer->pos.Distance(p1->pos) < radius) {
                localPlayer->pos = p2->pos;         // Teleport to Exit Portal
            }
            else if (localPlayer->pos.Distance(p2->pos) < radius) {
                localPlayer->pos = p1->pos;         // Teleport to Entry Portal
            }
        }
        //else std::cout << "Increase the distance between the Portals" << std::endl;
    }
}

void ESP::Draw(GLText::Font& font, int dist) {  // Main Function
    glGetIntegerv(GL_VIEWPORT, viewport);
    Vector3 screenCoords;

    for (int i = 0; i < (*numOfPlayers); i++) {
        if (entlist && entlist->ents && isValidEnt(entlist->ents[i])) {   // If Entity is "valid" draw it on the screen
            ent* e = entlist->ents[i];
            Vector3 center = e->head;
            center.z = center.z - EYE_HEIGHT + PLAYER_HEIGHT / 2;

            //Vector3 screenCoords;                                      [!!!] COULD BE WRONG

            if (WorldToScreen(center, screenCoords, matrix, viewport[2], viewport[3])) {
                DrawESPBox(e, screenCoords, font);
            }
        }
    }


    //Code Portal here
    // Angles are weird and I dont have the patience like I had for the aimbot because always have to restart
    /*
    if (bEntry) {
        entryPortal->head.x = localPlayer->head.x + (dist * cos(localPlayer->angle.x)); //
        entryPortal->head.y = localPlayer->head.y + (dist * sin(localPlayer->angle.x));
        entryPortal->head.z = localPlayer->head.z + (dist * sin(localPlayer->angle.y)); // Angle Z

        bEntry = !bEntry;
        entry = true;
    }
    if (entry) {
        Vector3 centEntryPort = entryPortal->head;
        centEntryPort.z = centEntryPort.z - EYE_HEIGHT + PLAYER_HEIGHT / 2;

        if (WorldToScreen(centEntryPort, screenCoords, matrix, viewport[2], viewport[3])) {
            DrawESPBox(entryPortal, screenCoords, font);
        }
    }


    if (bExit) {
        exitPortal->head.x = localPlayer->head.x - (dist * cos(localPlayer->angle.x));
        exitPortal->head.y = localPlayer->head.y + (dist * sin(localPlayer->angle.x));
        exitPortal->head.z = localPlayer->head.z + (dist * sin(localPlayer->angle.y));
        bExit = !bExit;
        exit = true;
    }
    if (exit) {
        Vector3 centExitPort = exitPortal->head;
        centExitPort.z = centExitPort.z - EYE_HEIGHT + PLAYER_HEIGHT / 2;

        if (WorldToScreen(centExitPort, screenCoords, matrix, viewport[2], viewport[3])) {
            DrawESPBox(exitPortal, screenCoords, font);
        }
    }
    checkTeleport(entryPortal, exitPortal);
    */
}
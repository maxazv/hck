#include "pch.h"
#include "mem.h"
#include "hook.h"
#include "OpenGL.h"
#include "ESP.h"

bool hooked = false;

typedef BOOL(__stdcall* _glSwapBuffers)(HDC hDc);

_glSwapBuffers glSwapBuffersGateWay;

GLText::Font glFont;
const int FONT_HEIGHT = 15;
const int FONT_WIDTH = 9;

ESP esp;

int distance = 20;

void Draw() {
    HDC currentHDC = wglGetCurrentDC();

    if (!glFont.bBuilt || currentHDC != glFont.hdc)
    {
        glFont.Build(FONT_HEIGHT);
    }

    GL::SetupOrtho();

    esp.Draw(glFont, distance);

    GL::RestoreGL();

    /*
    GL::DrawOutline(300, 300, 200, 200, 2.0f, rgb::red);

    float textPointX = glFont.centerText(300, 200, strlen(text) * FONT_WIDTH);
    float textPointY = 300 - FONT_HEIGHT / 2;

    glFont.Print(textPointX, textPointY, rgb::green, "%s", text);

    vec3 insideTextPoint = glFont.centerText(300, 300 + 100, 200, 200, strlen(text_2) * FONT_WIDTH, FONT_HEIGHT);
    glFont.Print(insideTextPoint.x, insideTextPoint.y, rgb::green, "%s", text_2);
    */  //inside SetupOrtho
}
int c = 0;
BOOL __stdcall hookglSwapBuffers(HDC hDc){
    /*
    if (c >= 1) {
        esp.bEntry = false;
    }
    */
    if (GetAsyncKeyState(VK_NUMPAD1) & 1) {
        esp.bEntry = true;
    }
    if (GetAsyncKeyState(VK_NUMPAD2) & 1) {
        esp.bExit = true;
    }
    if (GetAsyncKeyState(VK_NUMPAD3) & 1) {
        esp.entry = false;
        esp.exit = false;
    }
    if (GetAsyncKeyState(VK_NUMPAD4) & 1) {
        distance -= 5;
    }
    if (GetAsyncKeyState(VK_NUMPAD5) & 1) {
        distance += 5;
    }

    Draw();
    ++c;
    return glSwapBuffersGateWay(hDc);      //function gateway
}


DWORD WINAPI MainThread(HMODULE hModule) {

    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);
    std::cout << "Injected successfully\n" << std::endl;
    std::cout << "\n****************************************************\n" << std::endl;
    std::cout << "\nTo Set Entrance Portal: Press [NUMPAD 1]" << std::endl;
    std::cout << "\nTo Set Exit Portal: Press [NUMPAD 2]" << std::endl;
    std::cout << "\nTo Reset All Portals: Press [NUMPAD 3]" << std::endl;
    std::cout << "\nTo Decrease Portal-Set-Distance: Press [NUMPAD 4]" << std::endl;
    std::cout << "\nTo Increase Portal-Set-Distance: Press [NUMPAD 5]" << std::endl;

    uintptr_t moduleBase = (uintptr_t)GetModuleHandle(L"ac_client.exe");
    //or
    moduleBase = (uintptr_t)GetModuleHandle(NULL);

    /*
    glSwapBuffersGateWay = (_glSwapBuffers)GetProcAddress(GetModuleHandle(L"opengl32.dll"), "wglSwapBuffers");     //obtain function pointer
    glSwapBuffersGateWay = (_glSwapBuffers)mem::TrampHook32((BYTE*)glSwapBuffersGateWay, (BYTE*)hookglSwapBuffers, 5); //setting function pointer to gateway
                                                                                                                       //look "how to call a game function"
    */

    Hook SwapBuffersHook((BYTE*)GetProcAddress(GetModuleHandle(L"opengl32.dll"), "wglSwapBuffers"), (BYTE*)hookglSwapBuffers, (BYTE*)&glSwapBuffersGateWay, 5);
    SwapBuffersHook.Enable();

    while (true) {
        if (GetAsyncKeyState(VK_INSERT) & 1) {
            break;
        }
    }

    SwapBuffersHook.Disable();
    /*
    while (true) {
        std::cout << hooked << std::endl;
    }
    */

    if (f) fclose(f);
    //FreeConsole();
    //FreeLibraryAndExitThread(hModule, 0);
    return 0;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)MainThread, hModule, 0, nullptr));
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}


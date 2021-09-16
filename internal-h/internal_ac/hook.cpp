#include "pch.h"
#include "hook.h"

bool Detour32(BYTE* src, BYTE* dst, const uintptr_t len) {
	if (len < 5) return false;

	DWORD curProtection;
	VirtualProtect(src, len, PAGE_EXECUTE_READWRITE, &curProtection);
	uintptr_t relativeAddress = dst - src - 5;

	*src = 0xE9;
	*(uintptr_t*)(src + 1) = relativeAddress;

	VirtualProtect(src, len, curProtection, &curProtection);
	return true;
}

BYTE* TrampHook32(BYTE* src, BYTE* dst, const uintptr_t len) {
	if (len < 5) return 0;

	BYTE* gateway = (BYTE*)VirtualAlloc(0, len, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);	//allocating memory for gateway
	memcpy_s(gateway, len, src, len);	//copying stolen bytes into gateway
	uintptr_t gatewayRelativeAddr = src - gateway - 5;

	*(gateway + len) = 0xE9;
	*(uintptr_t*)((uintptr_t)gateway + len + 1) = gatewayRelativeAddr;

	Detour32(src, dst, len);
	return gateway;
}


Hook::Hook(BYTE* src, BYTE* dst, BYTE* ptrToGateWayPtr, uintptr_t len) {
	this->src = src;
	this->dst = dst;
	this->len = len;
	this->ptrToGatewayPtr = ptrToGateWayPtr;
}
Hook::Hook(const char* exportName, const char* modName, BYTE* dst, BYTE* ptrToGateWayPtr, uintptr_t len) {
	HMODULE hMod = GetModuleHandleA(modName);

	this->src = (BYTE*)GetProcAddress(hMod, exportName);
	this->dst = dst;
	this->len = len;
	this->ptrToGatewayPtr = ptrToGateWayPtr;
}
void Hook::Enable() {
	memcpy(originalBytes, src, len);
	*(uintptr_t*)this->ptrToGatewayPtr = (uintptr_t)TrampHook32(src, dst, len);		// Setting Function Pointer
	bStatus = true;
}
void Hook::Disable() {
	mem::Patch(src, originalBytes, len);
	bStatus = false;
}
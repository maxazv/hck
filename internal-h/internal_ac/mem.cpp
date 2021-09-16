#include "pch.h"
#include "mem.h"

bool mem::Detour32(BYTE* src, BYTE* dst, const uintptr_t len) {
	if (len < 5) return false;

	DWORD curProtection;
	VirtualProtect(src, len, PAGE_EXECUTE_READWRITE, &curProtection);
	uintptr_t relativeAddress = dst - src - 5;

	*src = 0xE9;
	*(uintptr_t*)(src + 1) = relativeAddress;

	VirtualProtect(src, len, curProtection, &curProtection);
	return true;
}

BYTE* mem::TrampHook32(BYTE* src, BYTE* dst, const uintptr_t len) {
	if (len < 5) return 0;

	BYTE* gateway = (BYTE*)VirtualAlloc(0, len, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);	//allocating memory for gateway
	memcpy_s(gateway, len, src, len);	//copying stolen bytes into gateway
	uintptr_t gatewayRelativeAddr = src - gateway - 5;

	*(gateway + len) = 0xE9;
	*(uintptr_t*)((uintptr_t)gateway + len + 1) = gatewayRelativeAddr;

	Detour32(src, dst, len);
	return gateway;
}

void mem::Patch(BYTE* dst, BYTE* src, unsigned int size) {
	DWORD oldprotect;
	VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &oldprotect);
	memcpy(dst, src, size);
	VirtualProtect(dst, size, oldprotect, &oldprotect);
}
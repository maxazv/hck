#pragma once
#include "pch.h"
#include <vector>
#include <windows.h>
#include <iostream>
#include <TlHelp32.h>

namespace mem {
	void Patch(BYTE* dst, BYTE* src, unsigned int size);

	bool Detour32(BYTE* src, BYTE* dst, const uintptr_t len);
	BYTE* TrampHook32(BYTE* src, BYTE* dst, const uintptr_t len);
}
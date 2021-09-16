#pragma once
#pragma comment(lib, "OpenGL32.lib")
#include <Windows.h>
#include <stdio.h>
#include <gl\Gl.h>
#include "mem.h"
#include "geom.h"

namespace rgb {
	const GLubyte red[3] = { 255, 0, 0 };
	const GLubyte green[3] = { 0, 255, 0 };
	const GLubyte gray[3] = { 55, 55, 55 };
	const GLubyte lightgray[3] = { 192, 192, 192 };
	const GLubyte black[3] = { 0, 0, 0 };
}

namespace GL {
	void SetupOrtho();
	void RestoreGL();

	void DrawFilledRect(float x, float y, float width, float height, const GLubyte color[3]);
	void DrawOutline(float x, float y, float width, float height, float lineWidth, const GLubyte color[3]);
}

namespace GLText {
	class Font {
	public:
		bool bBuilt = false;
		unsigned int base = 0;
		HDC hdc = nullptr;
		int height = 0;
		int width = 0;
		
		void Build(int height);
		void Print(float x, float y, const unsigned char color[3], const char* format, ...);

		Vector3 centerText(float x, float y, float width, float height, float textWidth, float textHeight);
		float centerText(float x, float width, float textWidth);

	};
}
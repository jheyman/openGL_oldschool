#ifndef GL_IO_H
#define GL_IO_H

#include "gl_defs.h"

bool LoadTGATexture(TextureImage *texture, const char *filename);
bool UpdateTexture(const TextureImage *texture);
unsigned int CreateBitmapFontBig(char *fontName, int fontSize, bool fat);
unsigned int CreateBitmapFontInfo(char *fontName, int fontSize);
void ClearFont(unsigned int base);
void EnterOrthoMode();
void ExitOrthoMode();
void OutputTextInfo(char* text, unsigned int x, unsigned int y, float color_r=1.0f, float color_g=1.0f, float color_b=1.0f);
void OutputTextBig(char* text, float color_r=1.0f, float color_g=1.0f, float color_b=1.0f, bool fat=true);

#endif
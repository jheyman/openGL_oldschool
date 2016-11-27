#ifndef GL_DEFS_H
#define GL_DEFS_H

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <gl/gl.h>
#include <gl/glu.h>
//#include <gl/glaux.h>
#include "bass.h"

#define X_RESOLUTION 800
#define Y_RESOLUTION 600
#define BITS_RESOLUTION 32

#define MAX_NB_RASTERS  50
#define FLASH_DURATION 2.0 // seconds

#define PI 3.1415927

#define BITMAP_SCROLLER_LETTER_SPACING 15 //in pixels
#define NBBITMAPSCROLLINGLETTERS 40

#define TROISD_VERTICAL_SCROLLER_LETTER_SPACING 1.0
#define TROISD_HORIZONTAL_SCROLLER_LETTER_SPACING 0.85
#define NBTROISDSCROLLINGLETTERS 30
#define FONTFILE "newbaveuse.ttf"
#define FONTNAME "NewBaveuse"

#define NB_BUBBLES				30
#define WATERLEVEL_START		-30
#define WATERLEVEL_STOP			Y_RESOLUTION
#define WATERLEVEL_RISESPEED	10
#define REACHEDMAXLEVELTIME		(Y_RESOLUTION/2 - WATERLEVEL_START)/WATERLEVEL_RISESPEED

#define SINELOGO_SUBTILES_X   16
#define SINELOGO_SUBTILES_Y   16

#define NB_STARS 250
#define STAR_XMIN -35
#define STAR_XMAX 35
#define STAR_YMIN -25
#define STAR_YMAX 15
#define STAR_ZMIN -50
#define STAR_ZMAX 0
#define STAR_V 15

#define TUNNEL_RADIUS       6
#define TUNNEL_MINZ         -10
#define TUNNEL_MAXZ         60
#define TUNNEL_CIRCUMPIECES 25
#define TUNNEL_DEPTH_PIECES 25

typedef struct
{
	GLfloat	x;
	GLfloat	y;
	GLfloat z;
	GLfloat v;
} Star;

typedef struct
{
	GLfloat	x;
	GLfloat	y;
} SineLogoTile;


// Phase 1 : warning text, siren sound
//	silence, then two times the siren, then silence again
// Phase 2 : simple rasters, flat 3D text
// Phase 3 : colored simple rasters, sine bitmap scroller
// Phase 4 : 3d colored rasters, bouncing 3D scroller
// Phase 5 : 3d colored rasters, sine 3D scroller
// Phase 6 : 3D colored ball-textured rasters, rotating 3D scroller
// Phase 7 : rising water, the end text
// Phase 8 : final fade out

#define SIREN_START 3
#define SIREN_STOP 10.6

#define NB_TRANSITIONS 8

enum {	TEXTURE_RASTER_LINEAR,
		TEXTURE_RASTER_PLAIN,
		TEXTURE_SCROLLER,
		TEXTURE_BACKGROUND,
		TEXTURE_BITMAPFONT,
		TEXTURE_BUBBLE,
		TEXTURE_UNDERWATER,
		TEXTURE_WATER_SURFACE,
		TEXTURE_WARNING_TEXT1,
		TEXTURE_WARNING_TEXT2,
		TEXTURE_WARNING_TEXT3,
		TEXTURE_WARNING_MASK,
		TEXTURE_SINELOGO,
		TEXTURE_TROMBINE,
		TEXTURE_STAR,
		NB_TEXTURES };

typedef struct
{
	GLubyte	*imageData;
	GLuint	bpp;
	GLuint	width;
	GLuint	height;
	GLuint	ID;
} TextureImage;

typedef struct 
{
	float x;
	float y;
	int letter_index;
} ScrollingLetter;

typedef struct
{
	GLfloat rotate;
	GLfloat thickness;
	GLfloat width;
	GLfloat	x;
	GLfloat	y;
	GLfloat color_r;
	GLfloat color_g;
	GLfloat color_b;
	GLfloat color_a;
} Raster;

typedef struct
{
	GLfloat	x;
	GLfloat	y;
	GLfloat vx;
	GLfloat vy;
	GLuint size;
} Bubble;

#endif
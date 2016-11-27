#include "gl_defs.h"
#include "gl_io.h"
#include "gl_main.h"

// Disable spurious VC6 warnings about floating point truncation
#ifdef  _MSC_VER
#pragma warning (disable:4305 4244 4018)
#endif//_MSC_VER

TextureImage textures[NB_TEXTURES];
Raster rasters[MAX_NB_RASTERS];
Star stars[NB_STARS];

ScrollingLetter bitmapScrollingLetters[NBBITMAPSCROLLINGLETTERS];
ScrollingLetter troisdScrollingLetters[NBTROISDSCROLLINGLETTERS];

Bubble bubbles[NB_BUBBLES];

SineLogoTile sineLogoTiles[SINELOGO_SUBTILES_X+1][SINELOGO_SUBTILES_Y+1];

// List of transition times from phase N to N+1
// WARNING : transitionTimes[0] must be 0, and transitionTimes[NB_TRANSITIONS+1] must be "infinite".
int transitionTimes[NB_TRANSITIONS+2] = {0, SIREN_STOP + 1, 34, 47, 66, 87, 103, 139, 142 + REACHEDMAXLEVELTIME + 5, 999999};
bool transitionTriggered[NB_TRANSITIONS+1];

extern HDC g_hDC;
extern HSTREAM strWarning;
extern HSTREAM strMusic;
extern char ErrorMessage[256];
extern unsigned int listBaseTextInfo;
extern unsigned int listBaseTextBigFat;
extern unsigned int listBaseTextBigThin;
extern GLuint fps;
extern bool showInfo;
extern bool showAxes;
extern double startTime;
extern double currentTimeCounterInSeconds;
extern double oldTimeCounterInSeconds;
extern GLfloat fadeFactor;
extern double finalFadeStartTime;
extern GLfloat fadeSpeed;
extern bool exiting;
extern float angleX; 
extern float angleY; 
extern float Zdistance;

extern GLYPHMETRICSFLOAT agmf[256]; 

GLfloat glfLightAmbient[]=	{ 0.01f, 0.01f, 0.01f, 1.0f };
GLfloat glfLightDiffuse[]=	{ 0.5f, 0.5f, 0.5f, 1.0f };
GLfloat glfLightSpecular[]=	{ 0.5f, 0.5f, 0.5f, 1.0f };
GLfloat glfLight_position[] = {0.0f, 0.0f, 3.0f, 1.0f};

//	Ascii codes    Sp  !   "   #   $   % & ' ( ) *   + ,   - . / 0   1   2   3   4   5   6   7   8   9   : ; < = > ?   @ A  B  C  D  E   F   G   H   I   J   K   L   M   N   O   P   Q   R   S   T   U   V   W   X   Y   Z  
int startList[] = {622,651,661,676,702,0,0,0,0,0,952,0,968,0,0,0,723,747,759,783,805,830,854,878,903,928,0,0,0,0,0,985,0,0, 26,50,72,96, 117,138,162,186,195,213,239,259,289,315,339,363,389,413,436,459,482,507,546,572,598};
int widthList[] = { 28,  7, 12, 23, 19,0,0,0,0,0, 14,0,  7,0,0,0, 22,  9, 22, 19, 22, 22, 22, 22, 22, 22,0,0,0,0,0,20, 0,25,23,21,23,20,  20, 23, 23,  8, 17, 25, 19, 29, 25, 23, 23, 25, 23, 22, 22, 22, 24, 38, 25, 25, 22};

float angle;

float theEnd_angleX = 0.0f;
float theEnd_angleY = 0.0f;
float theEnd_angleZ = 0.0f;
float theEnd_dive = -0.32f;

//initialize parameters for first sequence
int _nbRasters = MAX_NB_RASTERS / 3;
int _rasterTextureID;
bool verticalRaster = false;
bool rasterEnableBlend;

float flashFadeFactor=0.0f;
float virtualFlashFadeFactor=0.0f;
float finalFadeFactor=0.0;
bool flashing = false;
int flash_direction=1;
float scroller_movey = 3.0f;
float scroller_movex = 0.0f;
float test=0.0f;
bool scroller_isFat = false;
double sceneTime;
float scroller_stop = -5.0f;
float scroller_sineAmplitudeX = 0.0f;
float scroller_sineAmplitudeY = 0.0f;
float scroller_sineAmplitudeZ = 0.0f;
float scroller_angleX = 0.0f;
float scroller_angleY = 0.0f;
bool scroller_twistX = false;
bool scroller_twistY = false;
float scroller_speed;
float background_texture_shiftX = 0.0f;
float background_texture_shiftY = 0.0f;
float background_angle = 0.0f;
float warningMaskShift = 0.0;
int warningTextTexture;
int backGroundTexture;
float backGroundTileFactor = 0.0;
float backGroundColorFactor = 0.0;

char scroller_text[2048];

#define PHRASE1 "     ALLRIGHT, OLDSCHOOL EFFECTS COMING UP, AS AN EXCUSE TO EXPERIMENT WITH OPENGL AND BRING BACK OLD MEMORIES                "

#define PHRASE2 "        REMEMBER WHEN SINE SCROLLER WHERE A THING ? PEPPERIDGE FARM REMEMBERS...                         "

#define PHRASE3 "     ONCE UPON A TIME, IT WAS NOT A PIECE OF CAKE TO RENDER PSEUDO-3D SHAPES WHEN RUNNING OFF A FEW MHZ of CPU!                            "

#define PHRASE4 "SINE FUNCTION IS OUR FRIEND, LET'S TAKE A MOMENT TO COMMEMORATE THE INFINITE NUMBER OF TIMES IT WAS ABUSED IN OLDSKOOL DEMOS...                            "

#define PHRASE5 "            THIS IS HARDLY READABLE BUT WHO CARES, IT SPINS AND IT IS FUN TO CODE!                   "

#define PHRASE6 "        TWIIIIST SCROLLER BABY ! BACK IN THE DAYS ROTOZOOM REQUIRED CUNNING TRICKS TO ADDRESS MEMORY, NOW IT IS A MATTER OF SETTING A COUPLE OF FACTORS BEFORE RENDERING...                      " 

TextureImage texture;
int waterLevel;

bool troisd_horizontalScroller = false;
bool troisd_verticalScroller = false;
bool bitmap_scroller = false;
bool showBubbles = false;
bool showBackground = false;
bool showRasters = false;
bool showWarning = true;
bool showSineLogo = true;
bool showStarfield = false;
bool showTunnel = false;

bool LoadTextures()
{
	if (!LoadTGATexture(&textures[TEXTURE_SCROLLER],"Data/scroller.tga") ||
		!LoadTGATexture(&textures[TEXTURE_RASTER_LINEAR],"Data/raster.tga") || 
		!LoadTGATexture(&textures[TEXTURE_RASTER_PLAIN],"Data/raster_plain.tga") || 
		!LoadTGATexture(&textures[TEXTURE_BACKGROUND],"Data/background.tga") || 
		!LoadTGATexture(&textures[TEXTURE_BITMAPFONT],"Data/fonte.tga") || 
		!LoadTGATexture(&textures[TEXTURE_WARNING_TEXT1],"Data/warning_text1.tga") ||
		!LoadTGATexture(&textures[TEXTURE_WARNING_TEXT2],"Data/warning_text2.tga") ||
		!LoadTGATexture(&textures[TEXTURE_WARNING_TEXT3],"Data/warning_text3.tga") ||
		!LoadTGATexture(&textures[TEXTURE_WARNING_MASK],"Data/warning_mask.tga") ||
		!LoadTGATexture(&textures[TEXTURE_SINELOGO],"Data/sinelogo.tga") ||
		!LoadTGATexture(&textures[TEXTURE_TROMBINE],"Data/trombine.tga") ||
		!LoadTGATexture(&textures[TEXTURE_STAR],"Data/star.tga") ||
		!LoadTGATexture(&textures[TEXTURE_BUBBLE],"Data/bubble.tga") )
	{
		strcpy(ErrorMessage, "Unable to load texture!");
		return false;
	}
	else
	{
		return true;
	}
}

void CleanTextures()
{
	for (int n = 0 ; n < NB_TEXTURES ; n++)
	{
		if (textures[n].ID != 0)
			glDeleteTextures(1, &textures[n].ID);
	}

	if (texture.imageData != NULL) delete [] texture.imageData;
}

void CleanUp()
{
	ClearFont(listBaseTextInfo);
	ClearFont(listBaseTextBigFat);
	ClearFont(listBaseTextBigThin);
	CleanTextures();
	
	if (RemoveFontResource(FONTFILE)==0)
	{
		char temp[32];
		sprintf(temp, "%d", GetLastError());
		MessageBox(NULL, "RemoveFontResource failed", temp, MB_OK);	
	}
	else
		SendMessage(HWND_BROADCAST,WM_FONTCHANGE,0,0);

	// For some reason, the deletion of the font file fails with "access denied" error...
	// I don't have the courage to investigate...

	/*
	char Fullpath[MAX_PATH+125];
	GetWindowsDirectory(Fullpath, MAX_PATH);
	strcat(Fullpath, "\\Fonts");

	char commandText[128];
	sprintf(commandText, "del %s\\%s", FONTFILE, Fullpath);
	system(commandText);
	*/
}

void initializeWaterTexture()
{
	int i, j, index;

	texture.height = 16;
	texture.width = 64;
	texture.bpp = 32;
	texture.imageData = new GLubyte[texture.height*texture.width*texture.bpp/8];

	glGenTextures(1, &textures[TEXTURE_UNDERWATER].ID);
	glGenTextures(1, &textures[TEXTURE_WATER_SURFACE].ID);

	texture.ID = textures[TEXTURE_UNDERWATER].ID;

	for (j=0 ; j < texture.width ; j++) 
	{
		index = j*texture.bpp/8;
		for (i=0 ; i < texture.height ; i++) 
		{
			int colorR, colorG, colorB, alpha;
			colorR = 0;
			colorG = 35;
			colorB = 200;
			alpha = 127;
			
			texture.imageData[index] = colorR;
			texture.imageData[index+1] = colorG;
			texture.imageData[index+2] = colorB;
			texture.imageData[index+3] = alpha;
			
			index +=texture.width*texture.bpp/8;
		}
	}

	UpdateTexture(&texture);
}

bool Initialize()
{
	bool success = true;
	int i;
		
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);		
	glFrontFace(GL_CCW);						
	glShadeModel(GL_SMOOTH);					
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glEnable(GL_DEPTH_TEST);	
	
	glLightfv(GL_LIGHT0, GL_AMBIENT, glfLightAmbient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, glfLightDiffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, glfLightSpecular);
	glLightfv(GL_LIGHT0, GL_POSITION, glfLight_position);

	glDisable(GL_LIGHTING);	
	glEnable(GL_CULL_FACE);	
	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);

	// init bubbles locations/speeds
	for (i=0; i < NB_BUBBLES ; i++)
	{
		bubbles[i].x = (float)rand()* X_RESOLUTION / RAND_MAX;
		bubbles[i].y = -350.0 - (float)rand()* 150 / RAND_MAX;
		bubbles[i].vx = 0.0;
		bubbles[i].vy = 75.0;
		bubbles[i].size = 5+(int)(15*(float)rand()/RAND_MAX);
	}

	for (i=0; i < NB_STARS ; i++)
	{
		stars[i].x = STAR_XMIN + (float)rand()*(STAR_XMAX-STAR_XMIN)/RAND_MAX;
		stars[i].y = STAR_YMIN + (float)rand()*(STAR_YMAX-STAR_YMIN)/RAND_MAX;
		stars[i].z = STAR_ZMIN + (float)rand()*(STAR_ZMAX-STAR_ZMIN)/RAND_MAX;
	}

	char Fullpath[MAX_PATH+125];
	GetWindowsDirectory(Fullpath, MAX_PATH);
	strcat(Fullpath, "\\Fonts");

	char commandText[128];
	sprintf(commandText, "copy %s %s", FONTFILE, Fullpath);
	system(commandText);

	if (AddFontResource(FONTFILE)==0)
	{
		int err=GetLastError();
		char temp[64];
		sprintf(temp,"%d", err);
		MessageBox(NULL, "AddFontResource failed", temp, MB_OK);	
		return false;
	}
	else
	{
		SendMessage(HWND_BROADCAST,WM_FONTCHANGE,0,0);
	}

	listBaseTextBigFat = CreateBitmapFontBig(FONTNAME, 16, true);
	listBaseTextBigThin = CreateBitmapFontBig(FONTNAME, 16, false);
	listBaseTextInfo = CreateBitmapFontInfo("Arial", 20);

	for (i=0; i < NB_TRANSITIONS ; i++)
		transitionTriggered[i] = false;

	// Force enable Vertical Sync :
	typedef BOOL (WINAPI *SwapProc)(int);
	
	// Grab the PROC address of the vsync extension
	PROC procSwapInterval = wglGetProcAddress("wglSwapIntervalEXT");
	
	// Does the extension exist?
	if (procSwapInterval) 
	{
		// Create a function pointer to the extension
		SwapProc pS = (SwapProc)procSwapInterval;
		// Enable or Disable vertical sync
		pS(1);
	}

	success = LoadTextures();
	
	initializeWaterTexture();

	return success;
}

void Animate(double t)
{
	int i,j, index, threshold, phase;	
	
	double delta_t = t - oldTimeCounterInSeconds;
	if (delta_t < 0) delta_t = 0;
	
	oldTimeCounterInSeconds = t;
	
	// Determine phase we are currently in.
	// Scan transition times from lower to higher.
	for (int k=1; k <= NB_TRANSITIONS+1  ; k++)
	{
		if (t < transitionTimes[k])
		{
			phase = k;
			sceneTime = t - transitionTimes[k-1];
			break;
		}
	}

	angle = 50*sceneTime;	
	int shift = 100*sceneTime;

	if (t>transitionTimes[8] && transitionTriggered[8] == false) 
	{
		exiting = true;
		finalFadeStartTime = currentTimeCounterInSeconds;
		fadeFactor = 0.0f;
		fadeSpeed = 0.25;
		transitionTriggered[8] = true;
	}
 	else if (t>transitionTimes[7] && transitionTriggered[7] == false) 
	{
		showSineLogo = false;
		showStarfield = false;
		showBubbles = true;
		showBackground = false;
		showRasters = false;
		showTunnel = false;
		bitmap_scroller = false;
		troisd_horizontalScroller = false;
		troisd_verticalScroller = false;
		transitionTriggered[7] = true;
	}
	else if (t>(transitionTimes[7]-FLASH_DURATION/2) && transitionTriggered[7] == false) 
	{
		flashing = true;
	} 
 	else if (t>transitionTimes[6] && transitionTriggered[6] == false) 
	{
		showBubbles = false;
		showSineLogo = true;
		showStarfield = false;
		showBackground = true;
		showTunnel = false;
		backGroundTexture = TEXTURE_TROMBINE;
		backGroundColorFactor = 0.5;
		showRasters = false;
		bitmap_scroller = false;
		troisd_horizontalScroller = false;
		troisd_verticalScroller = true;

		scroller_isFat = true;
		scroller_movey = 0.0f;
		scroller_movex = 3.0f;
		scroller_stop = 5.0f;
		scroller_sineAmplitudeX = 1.0f;
		scroller_sineAmplitudeY = 0.0f;
		scroller_sineAmplitudeZ = 0.0f;
		scroller_twistX= false;
		scroller_twistY = false;
		scroller_angleX = 0.0f;
		scroller_angleY = 0.0f;
		scroller_speed = 5.0f;
		verticalRaster = false;

		// (re)initialize 3D scrolling letters
		for (i=0; i < NBTROISDSCROLLINGLETTERS; i++)
		{
			troisdScrollingLetters[i].x = 0.0f;
			troisdScrollingLetters[i].y = -5 - TROISD_VERTICAL_SCROLLER_LETTER_SPACING*i;
			troisdScrollingLetters[i].letter_index = i;
		}
		
		strcpy(scroller_text, PHRASE6);

		transitionTriggered[6] = true;
	}
	else if (t>(transitionTimes[6]-FLASH_DURATION/2) && transitionTriggered[6] == false) 
	{
		flashing = true;
	} 
	else if (t>transitionTimes[5] && transitionTriggered[5] == false) 
	{
		// (re)initialize 3D scrolling letters
		for (i=0; i < NBTROISDSCROLLINGLETTERS; i++)
		{
			troisdScrollingLetters[i].x = 20 + TROISD_HORIZONTAL_SCROLLER_LETTER_SPACING*i;
			troisdScrollingLetters[i].y = 0.0f;
			troisdScrollingLetters[i].letter_index = i;
		}

		bitmap_scroller = false;
		showBackground = true;
		showStarfield = false;
		showTunnel = false;
		troisd_horizontalScroller = true;
		troisd_verticalScroller = false;
		scroller_isFat = true;
		scroller_movey = 0.0f;
		scroller_stop = -5.0f;
		scroller_sineAmplitudeY = 0.0f;
		scroller_sineAmplitudeZ = 0.0f;
		scroller_twistX= true;
		scroller_twistY = false;
		scroller_angleX = -34.0f;
		scroller_angleY = 28.0f;
		scroller_speed = 6.0f;
		rasterEnableBlend = true;
		_nbRasters = MAX_NB_RASTERS;
		verticalRaster = true;
		_rasterTextureID = TEXTURE_RASTER_LINEAR;
		strcpy(scroller_text, PHRASE5);
		
		transitionTriggered[5] = true;
	}
	else if (t>(transitionTimes[5]-FLASH_DURATION/2) && transitionTriggered[5] == false) 
	{
		flashing = true;
	} 
	else if (t>transitionTimes[4] && transitionTriggered[4] == false) 
	{
		// (re)initialize 3D scrolling letters
		for (i=0; i < NBTROISDSCROLLINGLETTERS; i++)
		{
			troisdScrollingLetters[i].x = 15 + TROISD_HORIZONTAL_SCROLLER_LETTER_SPACING*i;
			troisdScrollingLetters[i].y = 0.0f;
			troisdScrollingLetters[i].letter_index = i;
		}

		bitmap_scroller = false;
		showBackground = true;
		showStarfield = false;
		showRasters = true;
		showTunnel = false;
		troisd_horizontalScroller = true;
		troisd_verticalScroller = false;
		scroller_isFat = true;
		scroller_movey = 0.0;
		scroller_stop = -5.0f;
		scroller_sineAmplitudeY = 1.0f;
		scroller_sineAmplitudeZ = 0.0f;
		scroller_twistX= false;
		scroller_twistY = false;
		scroller_angleX = 28.0f;
		scroller_angleY = 37.0f;
		scroller_speed = 6.0f;
		rasterEnableBlend = true;
		_nbRasters = MAX_NB_RASTERS;
		_rasterTextureID = TEXTURE_RASTER_LINEAR;
		strcpy(scroller_text, PHRASE4);

		transitionTriggered[4] = true;
	} 
	else if (t>(transitionTimes[4]-FLASH_DURATION/2) && transitionTriggered[4] == false) 
	{
		flashing = true;
	}
	else if (t>transitionTimes[3] && transitionTriggered[3] == false) 
	{
		// (re)initialize 3D scrolling letters
		for (i=0; i < NBTROISDSCROLLINGLETTERS; i++)
		{
			troisdScrollingLetters[i].x = 20 + TROISD_HORIZONTAL_SCROLLER_LETTER_SPACING*i;
			troisdScrollingLetters[i].y = 0.0f;
			troisdScrollingLetters[i].letter_index = i;
		}

		bitmap_scroller = false;
		showBackground = false;
		showStarfield = false;
		showRasters = false;
		showTunnel = true;
		troisd_horizontalScroller = true;
		troisd_verticalScroller = false;
		scroller_isFat = true;
		scroller_movey = 0.0f;
		scroller_stop = -5.0f;
		scroller_sineAmplitudeY = 0.0f;
		scroller_sineAmplitudeZ = 0.0f;
		scroller_twistX = false;
		scroller_twistY = false;
		scroller_angleX = 30.0f;
		scroller_angleY = 42.0f;
		scroller_speed = 6.0f;
		rasterEnableBlend = true;
		_nbRasters = MAX_NB_RASTERS / 2;
		_rasterTextureID = TEXTURE_RASTER_LINEAR;
		strcpy(scroller_text, PHRASE3);

		transitionTriggered[3] = true;
	} 
	else if (t>(transitionTimes[3]-FLASH_DURATION/2) && transitionTriggered[3] == false) 
	{
		flashing = true;
	}
	else if (t>transitionTimes[2] && transitionTriggered[2] == false) 
	{
		// initialize bitmap scrolling text
		strcpy(scroller_text, PHRASE2);
		int total_width = 0;
		for (i=0; i < NBBITMAPSCROLLINGLETTERS; i++)
		{
			bitmapScrollingLetters[i].x = X_RESOLUTION+100+total_width;
			bitmapScrollingLetters[i].y = 0.0f;
			bitmapScrollingLetters[i].letter_index = i;
			total_width += widthList[scroller_text[i]-32] + BITMAP_SCROLLER_LETTER_SPACING;
		}

		bitmap_scroller = true;
		showBackground = true;
		showStarfield = false;
		showTunnel = false;
		troisd_horizontalScroller = false;
		troisd_verticalScroller = false;
		scroller_isFat = false;
		scroller_movey = -2.3f;
		scroller_stop = -5.0f;
		scroller_sineAmplitudeY = 0.75f;
		scroller_sineAmplitudeZ = 0.0f;
		scroller_twistX = false;
		scroller_twistY = false;
		scroller_angleX = 0.0f;
		scroller_angleY = 0.0f;
		scroller_speed = 5.0f;
		rasterEnableBlend = true;
		_nbRasters = MAX_NB_RASTERS / 3;
		_rasterTextureID = TEXTURE_RASTER_LINEAR;
		
		transitionTriggered[2] = true;
	}
	else if (t>(transitionTimes[2]-FLASH_DURATION/2) && transitionTriggered[2] == false) 
	{
		flashing = true;
	}
	else if (t>transitionTimes[1] && transitionTriggered[1] == false) 
	{
		// initialize 3D scrolling letters
		for (i=0; i < NBTROISDSCROLLINGLETTERS; i++)
		{
			troisdScrollingLetters[i].x = 15 + TROISD_HORIZONTAL_SCROLLER_LETTER_SPACING*i;
			troisdScrollingLetters[i].y = 0.0f;
			troisdScrollingLetters[i].letter_index = i;
		}

		showWarning = false;
		troisd_horizontalScroller = true;
		troisd_verticalScroller = false;
		bitmap_scroller = false;
		scroller_speed = 5.0f;
		showRasters = true;
		showTunnel = false;
		rasterEnableBlend = false;
		_rasterTextureID = TEXTURE_RASTER_PLAIN;
		showBackground = false;
		showStarfield = true;
		backGroundTexture = TEXTURE_BACKGROUND;
		backGroundTileFactor = 8.0;
		backGroundColorFactor = 0.1;
		BASS_MusicPlay(strMusic);
		strcpy(scroller_text, PHRASE1);
		transitionTriggered[1] = true;
	}
	else if (t>(transitionTimes[1]-FLASH_DURATION/2) && transitionTriggered[1] == false) 
	{
		flashing = true;
	}
	else if (t>SIREN_START && transitionTriggered[0] == false)
	{
		BASS_StreamPlay(strWarning, FALSE, BASS_SAMPLE_LOOP);
		warningTextTexture = TEXTURE_WARNING_TEXT1;
		transitionTriggered[0] = true;
	}

	if (flashing == true)
	{
		// Ok, we will gradually move from alpha=0 to a max alpha, which can be greater
		// than one (meaning that after alpha reaches one we will stay white for a bit more
		// time). Need to increment alpha taking into account the round trip (going from 0 to
		// max alpha, THEN back to 0. Hence the x2 factor in the value in the formula.
		virtualFlashFadeFactor += flash_direction*2.4*delta_t/FLASH_DURATION;
		
		if (virtualFlashFadeFactor > 1.2) /* max alpha */
		{
			//max alpha (time...) reached, clamp to alpha=1 and reverse flash direction
			flashFadeFactor = 1.0;
			flash_direction = -1;
		}
		else if (virtualFlashFadeFactor > 1.0) 
		{
			// just clamp alpha to 1, we have not reached the peak (=max time...) yet
			flashFadeFactor = 1.0;
		}
		else if (virtualFlashFadeFactor < 0.0) 
		{
			// clamp to 0, reset direction and stop flashing mode.
			flashFadeFactor = 0.0;
			flash_direction = 1;
			flashing = false;
		}
		else
			// just update new alpha factor for fadein/fadeout
			flashFadeFactor = virtualFlashFadeFactor;
	}

	static double keepT=0.0f;
	static bool warningStopped=false;
	static int warningCount=0;

	switch(phase)
	{
	case 1:
		if (sceneTime>SIREN_START && sceneTime <SIREN_STOP && (sceneTime - keepT) > 1.0) // move mask across warning text
		{
			warningMaskShift -= delta_t;
		}

		if (sceneTime >= SIREN_STOP && warningStopped == false)
		{
			BASS_ChannelStop(strWarning);
			warningStopped = true;
		}

		if (warningMaskShift <= -1) 
		{
			keepT = sceneTime;
			warningMaskShift = 0;
			
			warningCount++;
			if (warningCount == 2)
				warningTextTexture = TEXTURE_WARNING_TEXT2;
			else if (warningCount == 3) 
				warningTextTexture = TEXTURE_WARNING_TEXT3;
		}

		break;

	case 2: // simple horizontal rasters; unique shade of blue color

		for (i=0; i < NB_STARS ; i++)
		{
			stars[i].x += sin(0.6*sceneTime)*STAR_V*delta_t;
			if (stars[i].x > STAR_XMAX) stars[i].x = STAR_XMIN;
			if (stars[i].x < STAR_XMIN) stars[i].x = STAR_XMAX;

			stars[i].z += 1.5*STAR_V*delta_t;
			if (stars[i].z > STAR_ZMAX) stars[i].z = STAR_ZMIN;

			stars[i].y += cos(0.5*sceneTime)*STAR_V*delta_t;
			if (stars[i].y > STAR_YMAX) stars[i].y = STAR_YMIN;
			if (stars[i].y < STAR_YMIN) stars[i].y = STAR_YMAX;
		}

		for (i=0; i < _nbRasters ; i++)
		{
			rasters[i].thickness = 32;
			rasters[i].width = X_RESOLUTION;
			rasters[i].rotate = 0;
			rasters[i].y = (Y_RESOLUTION-128)/2 + (Y_RESOLUTION-128)/2 * sin(2*(sceneTime+0.05*i));
			rasters[i].x = X_RESOLUTION/2;
			rasters[i].color_r = 0.0f;
			rasters[i].color_g = 0.0f;
			rasters[i].color_b = 0.5+0.5*(float)i/_nbRasters;
			rasters[i].color_a = 1.0f;
		}
		break;
	case 3: // more colorful !
		background_angle += 0.05*delta_t;

		//move bitmap letters
		for (i=0; i < NBBITMAPSCROLLINGLETTERS; i++)
		{
			bitmapScrollingLetters[i].x -= 300*delta_t;
			bitmapScrollingLetters[i].y = (Y_RESOLUTION/6-30)+(Y_RESOLUTION/6-30)*sin(5*sceneTime+0.004*bitmapScrollingLetters[i].x);
			if (bitmapScrollingLetters[i].x < -100) 
			{
				//find the coordinates of the letter to the far right, and move the letter after that
				int max_x = 0, max_index = 0;
				for (int j=0; j < NBBITMAPSCROLLINGLETTERS; j++)
				{
					max_x = bitmapScrollingLetters[j].x > max_x ? bitmapScrollingLetters[j].x : max_x;
					max_index = bitmapScrollingLetters[j].x == max_x ? j : max_index;
				}
				
				int previous_index = (i > 0) ? i-1: NBBITMAPSCROLLINGLETTERS-1;
				bitmapScrollingLetters[i].x = bitmapScrollingLetters[previous_index].x + widthList[scroller_text[bitmapScrollingLetters[previous_index].letter_index]-32] + BITMAP_SCROLLER_LETTER_SPACING;
				
				// current letter has disappeared from the screen : pick the next letter to be scrolled
				bitmapScrollingLetters[i].letter_index += NBBITMAPSCROLLINGLETTERS;
				int delta = strlen(scroller_text) - bitmapScrollingLetters[i].letter_index;
				if (delta<=0) bitmapScrollingLetters[i].letter_index = -delta;
			}
		}
		
		// move rasters
		for (i=0; i < _nbRasters ; i++)
		{
			rasters[i].thickness = 32;
			rasters[i].width = X_RESOLUTION;
			rasters[i].rotate = 0;
			rasters[i].y = 2*Y_RESOLUTION/3 + Y_RESOLUTION/3 * sin(2*(sceneTime+0.05*i));
			rasters[i].x = X_RESOLUTION/2;
			rasters[i].color_r = cos(3*sceneTime)+(1-cos(3*sceneTime))*(float)i/_nbRasters;
			rasters[i].color_g = 1-(float)i/_nbRasters;
			rasters[i].color_b = sin(2*sceneTime)+(1-sin(2*sceneTime))*(float)i/_nbRasters;
			rasters[i].color_a = 1.0f;
		}
		break;
	case 4: // let's twist a bit!
		background_angle += 0.05*delta_t;
		
		//move rasters
		for (i=0; i < _nbRasters ; i++)
		{
			rasters[i].thickness = 32;
			rasters[i].width = X_RESOLUTION;
			rasters[i].rotate = 20*sin(sceneTime+0.025*i);
			rasters[i].y = Y_RESOLUTION/2 + Y_RESOLUTION/2 * sin(2*(sceneTime+0.05*i));
			rasters[i].x = X_RESOLUTION/2;
			rasters[i].color_r = cos(3*sceneTime)+(1-cos(3*sceneTime))*(float)i/_nbRasters;
			rasters[i].color_g = 1-(float)i/_nbRasters;
			rasters[i].color_b = sin(2*sceneTime)+(1-sin(2*sceneTime))*(float)i/_nbRasters;
			rasters[i].color_a = 1.0f;
		}
		scroller_movey = 1.5*fabs(sin(3*sceneTime));

		break;
	case 5: // final raster mayhem
		background_angle += 0.05*delta_t;
	
		//move rasters
		for (i=0; i < _nbRasters ; i++)
		{
			rasters[i].thickness = 32;
			rasters[i].width = X_RESOLUTION/2 + X_RESOLUTION/4 * sin(0.25*sin(0.15*sceneTime)*(sceneTime+0.5*i));
			rasters[i].rotate = 70*sin(sceneTime+0.05*i);
			rasters[i].y = Y_RESOLUTION/2 + Y_RESOLUTION/4 * sin(2*(sceneTime+0.05*i));
			rasters[i].x = X_RESOLUTION/2;
			rasters[i].color_r = cos(3*sceneTime)+(1-cos(3*sceneTime))*(float)i/_nbRasters;
			rasters[i].color_g = 1-(float)i/_nbRasters;
			rasters[i].color_b = sin(2*sceneTime)+(1-sin(2*sceneTime))*(float)i/_nbRasters;
			rasters[i].color_a = 1.0f;
		}
		break;
	case 6: // change texture !
		background_angle += 0.05*delta_t;
		
		for (i=0; i < _nbRasters ; i++)
		{
			rasters[i].thickness = Y_RESOLUTION;
			rasters[i].width = 32;
			rasters[i].rotate = 0;
			rasters[i].y = Y_RESOLUTION*(1 - (float)i/_nbRasters);
			rasters[i].x = X_RESOLUTION/2+ 1.5*(X_RESOLUTION/4)*sin(2*sceneTime+0.075*i)*sin(0.6*sceneTime+0.03*i);
			rasters[i].color_r = cos(3*sceneTime)+(1-cos(3*sceneTime))*(float)i/_nbRasters;
			rasters[i].color_g = 1-(float)i/_nbRasters;
			rasters[i].color_b = sin(2*sceneTime)+(1-sin(2*sceneTime))*(float)i/_nbRasters;
			rasters[i].color_a = 1.0f;
		}
		break;

	case 7: 

		background_angle = 360*sin(0.001*sceneTime);
		backGroundTileFactor = 0.5+ 1.5*fabs(sin(0.5*sceneTime));

		background_texture_shiftX += 0.5*delta_t;
		//if (background_texture_shiftX > backGroundTileFactor) background_texture_shiftX = 0.0f;

		background_texture_shiftY += 1.5*delta_t;
		//if (background_texture_shiftY > backGroundTileFactor) background_texture_shiftY = 0.0f;

		for (i=0; i <= SINELOGO_SUBTILES_X ; i++)
		{
			for (j=0; j <= SINELOGO_SUBTILES_Y ; j++)
			{
				sineLogoTiles[i][j].x = (X_RESOLUTION/3-128)+i*256/SINELOGO_SUBTILES_X+50*sin(3*t+4.0*j/SINELOGO_SUBTILES_Y);
				sineLogoTiles[i][j].y = (Y_RESOLUTION/2-128)+j*256/SINELOGO_SUBTILES_Y+50*sin(3*t+4.0*i/SINELOGO_SUBTILES_X);
			}
		}
		break;

	case 8:
	case 9:

		if ((t-transitionTimes[7]) > REACHEDMAXLEVELTIME)
		{
			theEnd_angleX -= 0.25*delta_t;
			theEnd_angleY += 0.25*delta_t;
			theEnd_angleZ -= 0.5*delta_t;
			theEnd_dive -= 0.1*delta_t;
		}
		else if ((t-transitionTimes[7]) > REACHEDMAXLEVELTIME+1)
		{
			theEnd_angleX -= 0.5*delta_t;
			theEnd_angleY += 0.5*delta_t;
			theEnd_angleZ -= 0.75*delta_t;
			theEnd_dive -= 0.15*delta_t;
		}
		else if ((t-transitionTimes[7]) > REACHEDMAXLEVELTIME+2)
		{
			theEnd_angleX -= 1.0*delta_t;
			theEnd_angleY += 1.0*delta_t;
			theEnd_angleZ -= 1.0*delta_t;
			theEnd_dive -= 0.2*delta_t;
		}

		waterLevel = WATERLEVEL_START + WATERLEVEL_RISESPEED*(t-transitionTimes[7]) ;
		
		if ( waterLevel >= WATERLEVEL_STOP)
			waterLevel = WATERLEVEL_STOP;
		
		texture.ID = textures[TEXTURE_WATER_SURFACE].ID;
		
		for (j=0 ; j < texture.width ; j++) 
		{
			index = j*texture.bpp/8;
			for (i=0 ; i < texture.height ; i++) 
			{
				int colorR, colorG, colorB, alpha;
				threshold = texture.height/2 + 3*sin((((j+shift)%texture.width)*2*PI)/texture.width) *sin(((3*j)%texture.width)*2*PI/texture.width);
				if (i > threshold) 
				{ 
					colorR = 0;
					colorG = 0;
					colorB = 0;
					alpha = 0;
				}
				else 
				{
					colorR = 0;
					colorG = 35;
					colorB = 200;
					alpha = 127;
				}
				
				texture.imageData[index] = colorR;
				texture.imageData[index+1] = colorG;
				texture.imageData[index+2] = colorB;
				texture.imageData[index+3] = alpha;
				
				index +=texture.width*texture.bpp/8;
			}
		}
		
		UpdateTexture(&texture);
		
		// Move bubbles
		
		for (i=0; i < NB_BUBBLES ; i++)
		{
			bubbles[i].y += bubbles[i].vy*delta_t;
			if ( (bubbles[i].y >= (waterLevel - bubbles[i].size)) ||
				(bubbles[i].y >= waterLevel))
				bubbles[i].y = -50;
			bubbles[i].x += bubbles[i].vx*delta_t;
			bubbles[i].vx = 30*sin(3*(sceneTime+10*i));
		}
		
		break;

	default:
		break;
	}

	// Move 3D Letters
	if (troisd_horizontalScroller)
	{
		for (i=0; i < NBTROISDSCROLLINGLETTERS; i++)
		{
			troisdScrollingLetters[i].x -= scroller_speed*delta_t;
			if (troisdScrollingLetters[i].x < scroller_stop) 
			{
				//find the coordinates of the letter to the far right, and move the letter after that
				
				int max_x = 0, max_index = 0;
				for (int j=0; j < NBTROISDSCROLLINGLETTERS; j++)
				{
					max_x = troisdScrollingLetters[j].x > max_x ? troisdScrollingLetters[j].x : max_x;
					max_index = troisdScrollingLetters[j].x == max_x ? j : max_index;
				}
				
				int previous_index = (i > 0) ? i-1: NBTROISDSCROLLINGLETTERS-1;
				troisdScrollingLetters[i].x = troisdScrollingLetters[previous_index].x + TROISD_HORIZONTAL_SCROLLER_LETTER_SPACING;
				
				// current letter has disappeared from the screen : pick the next letter to be scrolled
				troisdScrollingLetters[i].letter_index += NBTROISDSCROLLINGLETTERS;
				int delta = strlen(scroller_text) - troisdScrollingLetters[i].letter_index;
				if (delta<=0) troisdScrollingLetters[i].letter_index = -delta;
			}
		}
	}

	// Move 3D Letters
	if (troisd_verticalScroller)
	{
		for (i=0; i < NBTROISDSCROLLINGLETTERS; i++)
		{
			troisdScrollingLetters[i].y += scroller_speed*delta_t;
			if (troisdScrollingLetters[i].y > scroller_stop) 
			{
				//find the coordinates of the letter to the far right, and move the letter after that
				
				int max_y = 0, max_index = 0;
				for (int j=0; j < NBTROISDSCROLLINGLETTERS; j++)
				{
					max_y = troisdScrollingLetters[j].y > max_y ? troisdScrollingLetters[j].y : max_y;
					max_index = troisdScrollingLetters[j].y == max_y ? j : max_index;
				}
				
				int previous_index = (i > 0) ? i-1: NBTROISDSCROLLINGLETTERS-1;
				troisdScrollingLetters[i].y = troisdScrollingLetters[previous_index].y - TROISD_VERTICAL_SCROLLER_LETTER_SPACING;
				
				// current letter has disappeared from the screen : pick the next letter to be scrolled
				troisdScrollingLetters[i].letter_index += NBTROISDSCROLLINGLETTERS;
				int delta = strlen(scroller_text) - troisdScrollingLetters[i].letter_index;
				if (delta<=0) troisdScrollingLetters[i].letter_index = -delta;
			}
		}
	}

}

void rotate(float x_in, float y_in, float &x_out, float &y_out, float angle)
{
	x_out = x_in*cos(angle) + y_in*sin(angle);
	y_out = x_in*sin(angle) - y_in*cos(angle);
}

void Render()
{
	int i,j;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	if (showStarfield)
	{
		glTranslatef(0.0,0.0,Zdistance);
		glPushMatrix();
		glPushAttrib(GL_ALL_ATTRIB_BITS);

		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_TEXTURE_2D);
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);	
		
		glBindTexture(GL_TEXTURE_2D,textures[TEXTURE_STAR].ID);

		for (i=0; i < NB_STARS ; i++)
		{
			float fade = 0.75*(stars[i].z - STAR_ZMIN)/(STAR_ZMAX - STAR_ZMIN);
			glColor3f(fade,fade,fade);

			glBegin(GL_QUADS);
			glTexCoord2f(0.0f,0.0f);	
			glVertex3f(stars[i].x, stars[i].y, stars[i].z);
			glTexCoord2f(1.0f,0.0f);
			glVertex3f(stars[i].x+1, stars[i].y, stars[i].z);
			glTexCoord2f(1.0f,1.0f);
			glVertex3f(stars[i].x+1, stars[i].y+1, stars[i].z);
			glTexCoord2f(0.0f,1.0f);
			glVertex3f(stars[i].x, stars[i].y+1, stars[i].z);
			glEnd();
		}
		
		glPopAttrib();
		glPopMatrix();
	}

	// "THE END" screen
	if(showBubbles)
	{
		glEnable(GL_LIGHTING);	
		glPushMatrix();
		glTranslatef(-2.9,theEnd_dive,Zdistance);
		glRotatef(theEnd_angleX, 1.0,0.0,0.0);
		glRotatef(theEnd_angleY, 0.0,1.0,0.0);
		glRotatef(theEnd_angleZ, 0.0,0.0,1.0);
		glColor3f(0.0,0.0,1.0);
		OutputTextBig("T H E     E N D", 0.0,0.0,0.75, true);

		glDisable(GL_LIGHTING);	
		glPopMatrix();
	}

	EnterOrthoMode();

	glPushAttrib(GL_ALL_ATTRIB_BITS);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);	
	
	if (showBackground)
	{
		glBindTexture(GL_TEXTURE_2D,textures[backGroundTexture].ID);
		
		glColor4f(backGroundColorFactor, backGroundColorFactor, backGroundColorFactor, 1.0);
		glPushMatrix();
		
		float x1,y1,x2,y2,x3,y3,x4,y4;
		rotate(-backGroundTileFactor/2, -backGroundTileFactor/2, x1, y1, background_angle);
		rotate(backGroundTileFactor/2, -backGroundTileFactor/2, x2, y2, background_angle);
		rotate(backGroundTileFactor/2, backGroundTileFactor/2, x3, y3, background_angle);
		rotate(-backGroundTileFactor/2, backGroundTileFactor/2, x4, y4, background_angle);
		
		glBegin(GL_QUADS);
		glTexCoord2f(background_texture_shiftX + x1,background_texture_shiftY + y1);	
		glVertex2d(0, 0);
		glTexCoord2f(background_texture_shiftX + x2,background_texture_shiftY + y2);
		glVertex2d(X_RESOLUTION, 0);
		glTexCoord2f(background_texture_shiftX + x3,background_texture_shiftY + y3);
		glVertex2d(X_RESOLUTION, Y_RESOLUTION);
		glTexCoord2f(background_texture_shiftX + x4,background_texture_shiftY + y4);
		glVertex2d(0,Y_RESOLUTION);
		glEnd();
		
		glPopMatrix();
	}

	if (showSineLogo)
	{
		glBindTexture(GL_TEXTURE_2D,textures[TEXTURE_SINELOGO].ID);
		glColor4f(1.0, 1.0, 1.0, 1.0);
		
		for (i=0; i < SINELOGO_SUBTILES_X ; i++)
		{
			for (j=0; j < SINELOGO_SUBTILES_Y ; j++)
			{
				glBegin(GL_QUADS);
				glTexCoord2f((float)(i)/SINELOGO_SUBTILES_X,-(float)(j)/SINELOGO_SUBTILES_Y);	
				glVertex2d(sineLogoTiles[i][j].x, sineLogoTiles[i][j].y);
				glTexCoord2f((float)(i+1)/SINELOGO_SUBTILES_X, -(float)(j)/SINELOGO_SUBTILES_Y);
				glVertex2d(sineLogoTiles[i+1][j].x, sineLogoTiles[i+1][j].y);
				glTexCoord2f((float)(i+1)/SINELOGO_SUBTILES_X, -(float)(j+1)/SINELOGO_SUBTILES_Y);
				glVertex2d(sineLogoTiles[i+1][j+1].x, sineLogoTiles[i+1][j+1].y);
				glTexCoord2f((float)(i)/SINELOGO_SUBTILES_X, -(float)(j+1)/SINELOGO_SUBTILES_Y);
				glVertex2d(sineLogoTiles[i][j+1].x, sineLogoTiles[i][j+1].y);
				glEnd();

			}
		}
	}

	if (showWarning)
	{
		glBindTexture(GL_TEXTURE_2D,textures[warningTextTexture].ID);
		glColor4f(1.0, 1.0, 1.0, 1.0);

		glBegin(GL_QUADS);
		glTexCoord2f(0.0,1.0);	
		glVertex2d(0, 0.33*Y_RESOLUTION);
		glTexCoord2f(1.0,1.0);
		glVertex2d(X_RESOLUTION, 0.33*Y_RESOLUTION);
		glTexCoord2f(1.0,0.0);
		glVertex2d(X_RESOLUTION, 0.66*Y_RESOLUTION);
		glTexCoord2f(0.0,0.0);
		glVertex2d(0,0.66*Y_RESOLUTION);
		glEnd();

		glBindTexture(GL_TEXTURE_2D,textures[TEXTURE_WARNING_MASK].ID);
		glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);
		glColor4f(0.0, 0.0, 0.0, 1.0);
		
		// Display mask : initially start texture coord after vertical raster end
		// to hide the warning text completely. Then use warningMaskShift to scroll
		// the visible part across the text.
		// start coord = 165(end_of_raster)/512(total_width) = 0.322
		glBegin(GL_QUADS);
		glTexCoord2f(0.322+warningMaskShift,1.0);	
		glVertex2d(0,0.33*Y_RESOLUTION);
		glTexCoord2f(1.0+warningMaskShift,1.0);
		glVertex2d(X_RESOLUTION,0.33*Y_RESOLUTION);
		glTexCoord2f(1.0+warningMaskShift,0.0);
		glVertex2d(X_RESOLUTION,0.66*Y_RESOLUTION);
		glTexCoord2f(0.322+warningMaskShift,0.0);
		glVertex2d(0,0.66*Y_RESOLUTION);
		glEnd();

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	if (showRasters)
	{
		glBindTexture(GL_TEXTURE_2D,textures[_rasterTextureID].ID);
		
		// BIG TEST
		if (!rasterEnableBlend) 
			glDisable(GL_BLEND);
		else
			glEnable(GL_BLEND);

		for (i=0; i < _nbRasters ; i++)
		{
			glColor4f(rasters[i].color_r, rasters[i].color_g, rasters[i].color_b, rasters[i].color_a);
			glPushMatrix();
			glTranslatef(0.0, rasters[i].y, 0.0);
			glRotatef(rasters[i].rotate,0.0,0.0,1.0);
			
			if (!verticalRaster)
			{
				glBegin(GL_QUADS);
				glTexCoord2f(0.0f,0.0f);	
				glVertex2d((int)(rasters[i].x-0.5*rasters[i].width), (int)(-0.5*rasters[i].thickness));
				glTexCoord2f(16.0f,0.0f);
				glVertex2d((int)(rasters[i].x+0.5*rasters[i].width), (int)(-0.5*rasters[i].thickness));
				glTexCoord2f(16.0f,1.0f);
				glVertex2d((int)(rasters[i].x+0.5*rasters[i].width), (int)(0.5*rasters[i].thickness));
				glTexCoord2f(0.0f,1.0f);
				glVertex2d((int)(rasters[i].x-0.5*rasters[i].width), (int)(0.5*rasters[i].thickness));
				glEnd();
			}
			else
			{
				glBegin(GL_QUADS);
				glTexCoord2f(16.0f,0.0f);	
				glVertex2d((int)(rasters[i].x-0.5*rasters[i].width), (int)(-0.5*rasters[i].thickness));
				glTexCoord2f(16.0f,1.0f);
				glVertex2d((int)(rasters[i].x+0.5*rasters[i].width), (int)(-0.5*rasters[i].thickness));
				glTexCoord2f(0.0f,1.0f);
				glVertex2d((int)(rasters[i].x+0.5*rasters[i].width), (int)(0.5*rasters[i].thickness));
				glTexCoord2f(0.0f,0.0f);
				glVertex2d((int)(rasters[i].x-0.5*rasters[i].width), (int)(0.5*rasters[i].thickness));
				glEnd();			
			}
			glPopMatrix();
		}
	}

	if (bitmap_scroller)
	{
		glBindTexture(GL_TEXTURE_2D,textures[TEXTURE_BITMAPFONT].ID);
		
		for (i=0; i < NBBITMAPSCROLLINGLETTERS ; i++)
		{
			glColor4f(1.0, 1.0, 1.0, 1.0);
			
			float start_texture = (float)startList[scroller_text[bitmapScrollingLetters[i].letter_index] - 32]/1024;
			int width = widthList[scroller_text[bitmapScrollingLetters[i].letter_index] - 32];
			float stop_texture = (float)(startList[scroller_text[bitmapScrollingLetters[i].letter_index] - 32] + width)/1024;

			glPushMatrix();
			glTranslated((int)bitmapScrollingLetters[i].x,bitmapScrollingLetters[i].y,0);
			glBegin(GL_QUADS);
				glTexCoord2f(1.001*start_texture,0.99f);	
				glVertex2d(0, 0);
				glTexCoord2f(1.001*start_texture,0.01f);
				glVertex2d(0, 40);
				glTexCoord2f(0.999*stop_texture,0.01f);
				glVertex2d(width, 40);
				glTexCoord2f(0.999*stop_texture,0.99f);
				glVertex2d(width, 0);
			glEnd();
			glPopMatrix();
		}
	}

	if(showBubbles)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_TEXTURE_2D);
		glDisable(GL_DEPTH_TEST);	
		
		glColor4f(1.0,1.0,1.0,1.0);
		
		// bubbles...
		glBindTexture(GL_TEXTURE_2D,textures[TEXTURE_BUBBLE].ID);
		
		for (i=0; i < NB_BUBBLES ; i++)
		{
			glBegin(GL_QUADS);
			glTexCoord2f(0.0f,0.0f);	
			glVertex2d((int)bubbles[i].x, (int)bubbles[i].y);
			glTexCoord2f(0.0f,1.0f);
			glVertex2d((int)bubbles[i].x + bubbles[i].size, (int)bubbles[i].y);
			glTexCoord2f(1.0f,1.0f);
			glVertex2d((int)bubbles[i].x + bubbles[i].size, (int)bubbles[i].y + bubbles[i].size);
			glTexCoord2f(1.0f,0.0f);
			glVertex2d((int)bubbles[i].x, (int)bubbles[i].y + bubbles[i].size);
			glEnd();
		}
		
		// water surface...
		glBindTexture(GL_TEXTURE_2D,textures[TEXTURE_WATER_SURFACE].ID);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f,0.0f);	
		glVertex2d(0, waterLevel - 8);
		glTexCoord2f(16.0f,0.0f);
		glVertex2d(X_RESOLUTION-1, waterLevel - 8);
		glTexCoord2f(16.0f,1.0f);
		glVertex2d(X_RESOLUTION-1, waterLevel + 8);
		glTexCoord2f(0.0f,1.0f);
		glVertex2d(0,waterLevel + 8);
		glEnd();
		
		// underwater 
		glBindTexture(GL_TEXTURE_2D,textures[TEXTURE_UNDERWATER].ID);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f,0.0f);	
		glVertex2d(0, 0);
		glTexCoord2f(1.0f,0.0f);
		glVertex2d(X_RESOLUTION-1, 0);
		glTexCoord2f(1.0f,1.0f);
		glVertex2d(X_RESOLUTION-1, waterLevel - 8);
		glTexCoord2f(0.0f,1.0f);
		glVertex2d(0,waterLevel - 8);
		glEnd();	
	}

	glPopAttrib();
	ExitOrthoMode();
	
	if(showTunnel)
	{
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		
		glEnable(GL_LIGHTING);	
		glShadeModel(GL_FLAT);					
		glDisable(GL_BLEND);
		glDisable(GL_CULL_FACE);	

		glTranslatef(0.0,0.0,Zdistance);
		glRotatef(scroller_angleX,1.0,0.0,0.0);
		glRotatef(scroller_angleY+80,0.0,1.0,0.0);

		float P1x, P1y, P1z;
		float P2x, P2y, P2z;		
		float P3x, P3y, P3z;
		float P4x, P4y, P4z;

		for (int z=0; z<TUNNEL_DEPTH_PIECES; z++)
		{
			for (int k=0.5*TUNNEL_CIRCUMPIECES; k<TUNNEL_CIRCUMPIECES; k++)
			{
				float red = 0.2 + 0.8*sin(PI*(float)z/TUNNEL_DEPTH_PIECES)*cos(0.3*sceneTime);
				float green = 0.3 + 0.7*sin(PI*(float)k/TUNNEL_CIRCUMPIECES)*sin(0.1*sceneTime);
				float blue = 0.1 + 0.6*sin(0.5*sceneTime)*sin(0.5*sceneTime);
				glColor3f(red,green,blue);

				P1x = TUNNEL_RADIUS*(2.5+0.5*cos(2*sceneTime+0.5*z))*cos((float)2*PI*k/TUNNEL_CIRCUMPIECES)+3*cos(sceneTime)+4;
				P1y = TUNNEL_RADIUS*(2.5+0.5*cos(2*sceneTime+0.5*z))*sin((float)2*PI*k/TUNNEL_CIRCUMPIECES)+3*sin(sceneTime)+6;
				P1z = TUNNEL_MINZ + (float)z*(TUNNEL_MAXZ-TUNNEL_MINZ)/TUNNEL_DEPTH_PIECES;
				
				P2x = TUNNEL_RADIUS*(2.5+0.5*cos(2*sceneTime+0.5*(z+1)))*cos((float)2*PI*k/TUNNEL_CIRCUMPIECES)+3*cos(sceneTime)+4;
				P2y = TUNNEL_RADIUS*(2.5+0.5*cos(2*sceneTime+0.5*(z+1)))*sin((float)2*PI*k/TUNNEL_CIRCUMPIECES)+3*sin(sceneTime)+6;
				P2z = TUNNEL_MINZ + (float)(z+1)*(TUNNEL_MAXZ-TUNNEL_MINZ)/TUNNEL_DEPTH_PIECES;
				
				P3x = TUNNEL_RADIUS*(2.5+0.5*cos(2*sceneTime+0.5*(z+1)))*cos((float)2*PI*(k+1)/TUNNEL_CIRCUMPIECES)+3*cos(sceneTime)+4;
				P3y = TUNNEL_RADIUS*(2.5+0.5*cos(2*sceneTime+0.5*(z+1)))*sin((float)2*PI*(k+1)/TUNNEL_CIRCUMPIECES)+3*sin(sceneTime)+6;
				P3z = TUNNEL_MINZ + (float)(z+1)*(TUNNEL_MAXZ-TUNNEL_MINZ)/TUNNEL_DEPTH_PIECES;
				
				P4x = TUNNEL_RADIUS*(2.5+0.5*cos(2*sceneTime+0.5*z))*cos((float)2*PI*(k+1)/TUNNEL_CIRCUMPIECES)+3*cos(sceneTime)+4;
				P4y = TUNNEL_RADIUS*(2.5+0.5*cos(2*sceneTime+0.5*z))*sin((float)2*PI*(k+1)/TUNNEL_CIRCUMPIECES)+3*sin(sceneTime)+6;
				P4z = TUNNEL_MINZ + (float)z*(TUNNEL_MAXZ-TUNNEL_MINZ)/TUNNEL_DEPTH_PIECES;
				
				glBegin(GL_QUADS);
				//glTexCoord2f(0.0f,0.0f);	
				glVertex3f(P1x, P1y, P1z);
				//glTexCoord2f(1.0f,0.0f);
				glVertex3f(P2x, P2y, P2z);
				//glTexCoord2f(1.0f,1.0f);
				glVertex3f(P3x, P3y, P3z);
				//glTexCoord2f(0.0f,1.0f);
				glVertex3f(P4x, P4y, P4z);
				glEnd();			
				
			}
		}

		glPopAttrib();
	}

	// 3D Scroller
	if (troisd_horizontalScroller || troisd_verticalScroller)
	{
//		glEnable(GL_TEXTURE_2D);				
//		glBindTexture(GL_TEXTURE_2D, textures[TEXTURE_BITMAPFONT].ID);
		
		glEnable(GL_LIGHTING);	
		
		glLoadIdentity();

		if (troisd_horizontalScroller)
			glTranslatef(0.0,scroller_movey,Zdistance);
		else if (troisd_verticalScroller)
			glTranslatef(scroller_movex,0.0,Zdistance);

		glRotatef(scroller_angleX,1.0,0.0,0.0);
		glRotatef(scroller_angleY,0.0,1.0,0.0);
		
		for (unsigned int k=0; k < NBTROISDSCROLLINGLETTERS; k++)
		{
			char temp[3];
			glPushMatrix();
			
			if (troisd_horizontalScroller)
			{
				if (scroller_twistX) glRotatef(180*sceneTime+10*troisdScrollingLetters[k].x,1.0,0.0,0.0);
				if (scroller_twistY) glRotatef(sceneTime+0.35*troisdScrollingLetters[k].x,0.0,1.0,0.0);
				glTranslatef(troisdScrollingLetters[k].x,-0.35 + scroller_sineAmplitudeY*sin(4*sceneTime+0.35*troisdScrollingLetters[k].x),scroller_sineAmplitudeZ*sin(sceneTime+0.35*troisdScrollingLetters[k].x));
			}
			else if (troisd_verticalScroller)
			{
				if (scroller_twistX) glRotatef(180*sceneTime+10*troisdScrollingLetters[k].y,1.0,0.0,0.0);
				if (scroller_twistY) glRotatef(sceneTime+0.35*troisdScrollingLetters[k].y,0.0,1.0,0.0);
				glTranslatef(scroller_sineAmplitudeX*sin(2*sceneTime+0.2*troisdScrollingLetters[k].y), troisdScrollingLetters[k].y,scroller_sineAmplitudeZ*sin(sceneTime+0.35*troisdScrollingLetters[k].y));
				glRotatef(45*sin(2*sceneTime+0.2*troisdScrollingLetters[k].y),0.0,1.0,0.0);
			}

			sprintf(temp, "%c",scroller_text[troisdScrollingLetters[k].letter_index]);
			if (troisd_horizontalScroller) OutputTextBig(temp, 1.0,1.0,1.0, scroller_isFat);
			else if (troisd_verticalScroller) OutputTextBig(temp, 1.0,0.0,0.0, scroller_isFat);
			glPopMatrix();
		}
		
		glDisable(GL_LIGHTING);	
	}

	////////////////////////////////////////////
	// Perform all 2D operations in this section
	////////////////////////////////////////////
	if (showAxes || showInfo || exiting || flashing)
	{
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		glDisable(GL_LIGHTING);
		glDisable(GL_TEXTURE_2D);				
		glDisable(GL_DEPTH_TEST);				
		EnterOrthoMode();
		
		// Fade to white management
		if (flashing)
		{
			glShadeModel(GL_FLAT);					
			glDisable(GL_CULL_FACE);	
			glDisable(GL_LIGHT0);
			glDisable(GL_COLOR_MATERIAL);
			
			glDisable(GL_TEXTURE_2D);
			glDisable(GL_LIGHTING);
			glDisable(GL_DEPTH_TEST);			
			glEnable(GL_BLEND);
			glBlendFunc(GL_ONE, GL_ONE);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			
			glColor4f(1.0f, 1.0f, 1.0f, flashFadeFactor); 	
			
			glBegin(GL_QUADS);
			glVertex2d(0, 0);
			glVertex2d(X_RESOLUTION-1, 0);
			glVertex2d(X_RESOLUTION-1, Y_RESOLUTION-1);
			glVertex2d(0,Y_RESOLUTION-1);
			glEnd();
		}

		// draw X/Y axes
		if (showAxes)
		{
			GLushort stipplePattern = 0x333;
			glEnable(GL_LINE_STIPPLE);
			glLineStipple(1, stipplePattern);
			glColor3f(1.0f, 1.0f, 1.0f);
			
			glPushMatrix();
			glLoadIdentity();
			
			glBegin(GL_LINES);
			glVertex2d(0, Y_RESOLUTION/2);
			glVertex2d(X_RESOLUTION-1, Y_RESOLUTION/2);
			glVertex2d(X_RESOLUTION/2, 0);
			glVertex2d(X_RESOLUTION/2,Y_RESOLUTION-1);
			glEnd();
			
			glPopMatrix();
			glDisable(GL_LINE_STIPPLE);
		}
		
		// display text info
		if (showInfo)
		{
			glPushMatrix();
			
			char temp[256];

			sprintf(temp, "test = %.2f", test);
			OutputTextInfo(temp, 5, 100);

			sprintf(temp, "angle X = %.2f", angleX);
			OutputTextInfo(temp, 5, 80);
			
			sprintf(temp, "angle Y = %.2f", angleY);
			OutputTextInfo(temp, 5, 60);
			
			sprintf(temp, "Z = %.2f", Zdistance);
			OutputTextInfo(temp, 5, 40);
			
			sprintf(temp, "%lu fps", fps);
			OutputTextInfo(temp, 5, 20);
			
			sprintf(temp, "t = %.1fs",currentTimeCounterInSeconds - startTime);
			OutputTextInfo(temp, 5, 0);
			
			glPopMatrix();
		}
		
		// big quad for final fadeout
		if (exiting)
		{
			glShadeModel(GL_FLAT);					
			glDisable(GL_CULL_FACE);	
			glDisable(GL_LIGHT0);
			glDisable(GL_COLOR_MATERIAL);
			
			glDisable(GL_LIGHTING);
			glDisable(GL_DEPTH_TEST);			
			glEnable(GL_BLEND);
			glBlendFunc(GL_ONE, GL_ONE);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			
			glColor4f(0.0f, 0.0f, 0.0f, fadeFactor); 	
			
			glBegin(GL_QUADS);
			glVertex2d(0, 0);
			glVertex2d(X_RESOLUTION-1, 0);
			glVertex2d(X_RESOLUTION-1, Y_RESOLUTION-1);
			glVertex2d(0,Y_RESOLUTION-1);
			glEnd();
		}
		
		glPopAttrib();
		ExitOrthoMode();
	}
	///////////////////////////////
	// End of 2D operations section
	///////////////////////////////

	glFlush();
	SwapBuffers(g_hDC);
}
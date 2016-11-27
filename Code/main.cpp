#define WIN32_LEAN_AND_MEAN		// trim the excess fat from Windows

#include "gl_defs.h"
#include "gl_main.h"
#include "gl_io.h"

HDC			g_hDC=NULL;		// Private GDI Device Context
HGLRC		hRC=NULL;		// Permanent Rendering Context
HWND		hwnd=NULL;		// Holds Our Window Handle
HINSTANCE	hInstance;		// Holds The Instance Of The Application

HSTREAM strWarning;
HMUSIC strMusic;

bool active = true;		
bool fullscreen = false;
bool keyPressed[256];
bool paused = false;
bool exiting = false;
bool showInfo = false;
bool showAxes = false;
char ErrorMessage[256];
unsigned int listBaseTextInfo;
unsigned int listBaseTextBigFat;
unsigned int listBaseTextBigThin;

GLuint fps = 0;
double startTime = -1.0f;
double currentTimeCounterInSeconds = 0.0f;
double oldTimeCounterInSeconds = 0.0f;

double finalFadeStartTime;
GLfloat fadeFactor = 0.0f;
GLfloat fadeSpeed;

int mouseX, mouseY;
float angleX = 0.0f; 
float angleY = 0.0f; 
float Zdistance = -6.0f;

extern float scroller_movex;
extern float test;

LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int GetFPS(void)
{
	static LARGE_INTEGER TimerFrequency,LastTime;
	int FPS=1;

	if (!TimerFrequency.QuadPart) QueryPerformanceFrequency(&TimerFrequency);
		else
	{
		LARGE_INTEGER Time;

		QueryPerformanceCounter(&Time);

		if (LastTime.QuadPart!=0 && (Time.QuadPart-LastTime.QuadPart)!=0)
			FPS=((int)(TimerFrequency.QuadPart/(Time.QuadPart-LastTime.QuadPart)));

		LastTime.QuadPart=Time.QuadPart;
	}

	if (FPS<1) FPS=1;

	return FPS;
}

double GetHiResTime(void)
{
	static LARGE_INTEGER TimerFrequency;
	LARGE_INTEGER Time;
	Time.QuadPart= 0;
	double timeInSeconds = 0.0f;

	if (!TimerFrequency.QuadPart) 
		QueryPerformanceFrequency(&TimerFrequency);

	QueryPerformanceCounter(&Time);
	timeInSeconds = (double)(Time.QuadPart)/TimerFrequency.QuadPart;

	return timeInSeconds;
}

GLvoid ReSizeGLScene(GLsizei width, GLsizei height)
{
	if (height==0)	{ height=1;}

	glViewport(0,0,width,height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(60.0f,(GLfloat)width/(GLfloat)height,0.1f,100.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void SetupPixelFormat(HDC hDC)
{
	int nPixelFormat;					// our pixel format index

	static PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),	// size of structure
		1,								// default version
		PFD_DRAW_TO_WINDOW |			// window drawing support
		PFD_SUPPORT_OPENGL |			// OpenGL support
		PFD_DOUBLEBUFFER,				// double buffering support
		PFD_TYPE_RGBA,					// RGBA color mode
		32,								// 16 bit color mode
		0, 0, 0, 0, 0, 0,				// ignore color bits, non-palettized mode
		0,								// no alpha buffer
		0,								// ignore shift bit
		0,								// no accumulation buffer
		0, 0, 0, 0,						// ignore accumulation bits
		16,								// 16 bit z-buffer size
		0,								// no stencil buffer
		0,								// no auxiliary buffer
		PFD_MAIN_PLANE,					// main drawing plane
		0,								// reserved
		0, 0, 0 };						// layer masks ignored

	nPixelFormat = ChoosePixelFormat(hDC, &pfd);	// choose best matching pixel format
	SetPixelFormat(hDC, nPixelFormat, &pfd);		// set pixel format to device context
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HGLRC hRC;
	static HDC hDC;
	int width, height;

	switch(message)
	{
		case WM_CREATE:

			hDC = GetDC(hwnd);
			g_hDC = hDC;
			SetupPixelFormat(hDC);

			/* Initialize output - default device, 44100hz, stereo, 16 bits */
			BASS_Init(1,44100,0,hwnd,NULL);
			BASS_StreamFree(strWarning);
			BASS_StreamFree(strMusic);
			strWarning = BASS_StreamCreateFile(FALSE, "siren4best.mp3", 0, 0, 0);
			strMusic = BASS_MusicLoad(FALSE,"lizardking-fellowship.mod",0,0,BASS_MUSIC_RAMP|BASS_MUSIC_LOOP,0);

			// create rendering context and make it current
			hRC = wglCreateContext(hDC);
			wglMakeCurrent(hDC, hRC);

			return 0;
			break;

		case WM_CLOSE:

			// deselect rendering context and delete it
			wglMakeCurrent(hDC, NULL);
			wglDeleteContext(hRC);

			BASS_StreamFree(strWarning);
			BASS_StreamFree(strMusic);

			// send WM_QUIT to message queue
			PostQuitMessage(0);

			return 0;
			break;

		case WM_SIZE:
			height = HIWORD(lParam);
			width = LOWORD(lParam);

			ReSizeGLScene(width, height);
			return 0;
			break;

		case WM_KEYDOWN:
			keyPressed[wParam] = true;
			return 0;
			break;

		case WM_KEYUP:
			keyPressed[wParam] = false;
			return 0;
			break;

		case WM_MOUSEMOVE:
		{
			int oldMouseX, oldMouseY;
			oldMouseX = mouseX;
			oldMouseY = mouseY;

			mouseX = LOWORD(lParam);
			mouseY = HIWORD(lParam);

			angleX = (float)180*mouseY/Y_RESOLUTION - 90;
			angleY = (float)180*mouseX/X_RESOLUTION - 90;

			return 0;
		}

		default:
			break;
	}

	return (DefWindowProc(hwnd, message, wParam, lParam));
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	WNDCLASSEX windowClass;
	MSG		   msg;
	bool	   done;
	bool       fullScreen;
	DWORD	   dwExStyle;
	DWORD	   dwStyle;
	RECT	   windowRect;

	int width = X_RESOLUTION;
	int height = Y_RESOLUTION;
	int bits = BITS_RESOLUTION;

	fullScreen = true;

	windowRect.left=(long)0;
	windowRect.right=(long)width;
	windowRect.top=(long)0;
	windowRect.bottom=(long)height;

	windowClass.cbSize			= sizeof(WNDCLASSEX);
	windowClass.style			= CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc		= WndProc;
	windowClass.cbClsExtra		= 0;
	windowClass.cbWndExtra		= 0;
	windowClass.hInstance		= hInstance;
	windowClass.hIcon			= LoadIcon(NULL, IDI_APPLICATION);
	windowClass.hCursor			= LoadCursor(NULL, IDC_ARROW);
	windowClass.hbrBackground	= NULL;
	windowClass.lpszMenuName	= NULL;
	windowClass.lpszClassName	= "OpenGL";
	windowClass.hIconSm			= LoadIcon(NULL, IDI_WINLOGO);

	if (!RegisterClassEx(&windowClass))
		return 0;

	if (fullScreen)
	{
		DEVMODE dmScreenSettings;
		memset(&dmScreenSettings,0,sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);	
		dmScreenSettings.dmPelsWidth = width;
		dmScreenSettings.dmPelsHeight = height;
		dmScreenSettings.dmBitsPerPel = bits;
		dmScreenSettings.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;

		if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
		{
			MessageBox(NULL, "Display mode failed", NULL, MB_OK);
			fullScreen=FALSE;	
		}
	}

	if (fullScreen)
	{
		dwExStyle=WS_EX_APPWINDOW;
		dwStyle=WS_POPUP;
		ShowCursor(FALSE);
	}
	else
	{
		dwExStyle=WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		dwStyle=WS_OVERLAPPEDWINDOW;
	}

	AdjustWindowRectEx(&windowRect, dwStyle, FALSE, dwExStyle);

	hwnd = CreateWindowEx(NULL,
						  "OpenGL",
						  "Bids'o'GL",
						  dwStyle | WS_CLIPCHILDREN |
						  WS_CLIPSIBLINGS,
						  0, 0,
						  windowRect.right - windowRect.left,
						  windowRect.bottom - windowRect.top,
						  NULL,
						  NULL,
						  hInstance,
						  NULL);

	if (!hwnd)
	{
		MessageBox(NULL,"CreateWindowEx failed","INIT ERROR",MB_OK | MB_ICONINFORMATION);
		return 0;
	}

	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);

	done = false;
	
	if(!Initialize())
	{
		MessageBox(NULL,"Initialization failed","INIT ERROR",MB_OK | MB_ICONINFORMATION);
		goto theEnd;
	}

	startTime = GetHiResTime();
	oldTimeCounterInSeconds = startTime;

	// main message loop
	while (!done)
	{
		PeekMessage(&msg, hwnd, NULL, NULL, PM_REMOVE);

		if (msg.message == WM_QUIT)
		{
			done = true;
		}
		else
		{
			currentTimeCounterInSeconds = GetHiResTime();
			
			if (keyPressed[VK_ESCAPE])
			{
				exiting = true;
				finalFadeStartTime = currentTimeCounterInSeconds;
				fadeFactor = 0.0f;
				fadeSpeed = 3.0f;
				keyPressed[VK_ESCAPE] = false;
			}
			
			if (keyPressed['P'])
			{
				paused = !paused; 
				keyPressed['P'] = false;
			}
			
			if (keyPressed['I'])
			{
				showInfo = !showInfo; 
				keyPressed['I'] = false;
			}
			
			if (keyPressed['X'])
			{
				showAxes = !showAxes; 
				keyPressed['X'] = false;
			}
			
			if (keyPressed['A'])
			{
				Zdistance += 0.1f;
				keyPressed['A'] = false;
			}

			if (keyPressed['Q'])
			{
				Zdistance -= 0.1f;
				keyPressed['Q'] = false;
			}

			if (keyPressed['Z'])
			{
				scroller_movex += 0.1f;
				keyPressed['Z'] = false;
			}

			if (keyPressed['S'])
			{
				scroller_movex -= 0.1f;
				keyPressed['S'] = false;
			}

			if (keyPressed['T'])
			{
				test += 0.1f;
				keyPressed['T'] = false;
			}

			if (keyPressed['G'])
			{
				test -= 0.1f;
				keyPressed['G'] = false;
			}

			if (exiting)
			{
				// count nb of music channels
				int channels=0;
				while (BASS_MusicGetVolume(strMusic,channels)!=-1) channels++;

				// gradually reduce channel volume to 0
				for (int c = 0; c < channels ; c++)
					BASS_MusicSetVolume(strMusic, c, __max(0,100-50*fadeSpeed*(currentTimeCounterInSeconds - finalFadeStartTime)));

				// gradually fade to black
				fadeFactor = (float)(1 - exp(-fadeSpeed*(currentTimeCounterInSeconds - finalFadeStartTime)));
				
				if (fadeFactor >= 0.96f)
				{
					Sleep(200);
					done = true;
				}
			}

			Animate(currentTimeCounterInSeconds-startTime);
				
			if (!paused) 
			{
				fps = GetFPS();
				Render();
			}
				
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

theEnd:

	CleanUp();

	if (fullScreen)
	{
		ChangeDisplaySettings(NULL,0);
		ShowCursor(TRUE);
	}

	if (hRC)
	{
		if (!wglMakeCurrent(NULL,NULL))
		{
			MessageBox(NULL,"Release Of DC And RC Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}

		if (!wglDeleteContext(hRC))
		{
			MessageBox(NULL,"Release Rendering Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}
		hRC=NULL;
	}

	if (g_hDC && !ReleaseDC(hwnd,g_hDC))
	{
		MessageBox(NULL,"Release Device Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		g_hDC=NULL;
	}

	if (hwnd && !DestroyWindow(hwnd))
	{
		MessageBox(NULL,"Could Not Release hwnd.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hwnd=NULL;
	}

	return msg.wParam;
}
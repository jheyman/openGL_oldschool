#include "gl_io.h"
#include "gl_defs.h"
extern HDC g_hDC;
extern unsigned int listBaseTextInfo;
extern unsigned int listBaseTextBigFat;
extern unsigned int listBaseTextBigThin;

GLYPHMETRICSFLOAT agmf[256]; 

///////////////////////////////////////////////////////////////////
// Input : 
// - TextureImage structure (unitialized)
// - filename of the TGA file to load into a texture
// Output : 
// - a new Texture ID is created, texture is loaded with image data
///////////////////////////////////////////////////////////////////
bool LoadTGATexture(TextureImage *texture, const char *filename)			
{    
	GLubyte	TGAheader[18];
	GLuint	bytesPerPixel;								
	GLuint	imageSize;									
	GLuint	temp;										
	GLuint	type=GL_RGBA;								

/* TGA FORMAT
typedef struct
{
    0 : byte  identsize;          // size in bytes of ID field that follows 18 byte header (0 usually)
    1 : byte  colourmaptype;      // type of colour map 0=none, 1=has palette
    2 : byte  imagetype;          // type of image 0=none,1=indexed,2=rgb,3=grey,+8=rle packed

    3-4 : short colourmapstart;     // first colour map entry in palette
    5-6 : short colourmaplength;    // number of colours in palette
    7 : byte  colourmapbits;      // number of bits per palette entry 15,16,24,32

    8-9 : short xstart;             // image x origin
    10-11 : short ystart;             // image y origin
    12-13 : short width;              // image width in pixels
    14-15 : short height;             // image height in pixels
    16 byte : bits;               // image bits per pixel 8,16,24,32
    17 byte : descriptor;         // image descriptor bits (vh flip bits)
    
    // pixel data follows header
    
} TGA_HEADER
*/

	if (texture == NULL)
		return false;
	
	FILE *file = fopen(filename, "rb");						
	if(file==NULL)											
	{
		return false;									
	}

	if(	fread(TGAheader,1,sizeof(TGAheader),file)!=sizeof(TGAheader) )			
	{
		fclose(file);										
		return false;										
	}

	//skip potential additional ID field
	if (TGAheader[0] != 0)
		fread(TGAheader,TGAheader[0],1,file);

	//only RGB(A) supported for now.
	if (TGAheader[2] != 2)
	{
		fclose(file);										
		return false;										
	}

	texture->width  = TGAheader[13] * 256 + TGAheader[12];			
	texture->height = TGAheader[15] * 256 + TGAheader[14];			
    
 	if(	texture->width	<=0	||								
		texture->height	<=0	||							
		(TGAheader[16]!=24 && TGAheader[16]!=32))					
	{
		fclose(file);										
		return false;									
	}

	texture->bpp	= TGAheader[16];						
	bytesPerPixel	= texture->bpp/8;						
	imageSize		= texture->width*texture->height*bytesPerPixel;	

	texture->imageData=(GLubyte *)malloc(imageSize);	

	if(	texture->imageData==NULL ||							
		fread(texture->imageData, 1, imageSize, file)!=imageSize)	
	{
		if(texture->imageData!=NULL)					
			free(texture->imageData);					

		fclose(file);									
		return false;									
	}

	for(GLuint i=0; i<int(imageSize); i+=bytesPerPixel)		
	{														
		temp=texture->imageData[i];							
		texture->imageData[i] = texture->imageData[i + 2];	
		texture->imageData[i + 2] = temp;					
	}

	fclose (file);											

	// Get a new Tex id and build a texture from the data
	glGenTextures(1, &(texture->ID));					
	UpdateTexture(texture);

	if(texture->imageData!=NULL)					
		free(texture->imageData);

	return true;											
}

/////////////////////////////////////////////////////////////////////////
// Input : 
// - TextureImage structure (initialized with appropriate data)
// Output : 
// - Texture whose id was passed in input struct is updated with its data 
/////////////////////////////////////////////////////////////////////////
bool UpdateTexture(const TextureImage *texture)
{
	GLuint type=GL_RGBA;
	
	glBindTexture(GL_TEXTURE_2D, texture->ID);		
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	
	
	if (texture->bpp==24)									
	{
		type=GL_RGB;										
	}

	glTexImage2D(GL_TEXTURE_2D, 0, type, texture->width, texture->height, 0, type, GL_UNSIGNED_BYTE, texture->imageData);

	return true;
}

HFONT theFontHandles[20];
int fontCounter=0;

unsigned int CreateBitmapFontInfo(char *fontName, int fontSize)
{
	HFONT hFont;         // windows font
	unsigned int base;

	base = glGenLists(96);      // create storage for 96 characters

	if (stricmp(fontName, "symbol") == 0)
	{
	     hFont = CreateFont(fontSize, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, SYMBOL_CHARSET, 
							OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
							FF_DONTCARE | DEFAULT_PITCH, fontName);
	}
	else
	{
		 hFont = CreateFont(fontSize, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, 
							OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
							FF_DONTCARE | DEFAULT_PITCH, fontName);
	}

	if (!hFont)
		return 0;
	else
		theFontHandles[fontCounter++] = hFont;

	SelectObject(g_hDC, hFont);
	wglUseFontBitmaps(g_hDC, 32, 96, base);
	
	return base;
}

unsigned int CreateBitmapFontBig(char *fontName, int fontSize, bool fat)
{
	HFONT hFont;         // windows font
	unsigned int base;

	base = glGenLists(96);      // create storage for 96 characters

	if (stricmp(fontName, "symbol") == 0)
	{
	     hFont = CreateFont(fontSize, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, SYMBOL_CHARSET, 
							OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
							FF_DONTCARE | DEFAULT_PITCH, fontName);
	}
	else
	{
		 hFont = CreateFont(fontSize, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, 
							OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
							FF_DONTCARE | DEFAULT_PITCH, fontName);
	}

	if (!hFont)
		return 0;
	else
		theFontHandles[fontCounter++] = hFont;

	SelectObject(g_hDC, hFont);
	
	if (fat)
		wglUseFontOutlines(g_hDC, 32, 96, base, 0.1f, 0.2f, WGL_FONT_POLYGONS, agmf);
	else
		wglUseFontOutlines(g_hDC, 32, 96, base, 0.1f, 0.02f, WGL_FONT_POLYGONS, agmf);

	return base;
}


void ClearFont(unsigned int base)
{
	if (base != 0)
		glDeleteLists(base, 96);

	for (int i=0; i < fontCounter ; i++)
		DeleteObject(theFontHandles[i]);

	fontCounter = 0;
}

void EnterOrthoMode()
{
    glMatrixMode(GL_PROJECTION);                        // Select The Projection Matrix
    glPushMatrix();                                     // Store The Projection Matrix
    glLoadIdentity();                                   // Reset The Projection Matrix
    glOrtho(0,X_RESOLUTION,0,Y_RESOLUTION,-100,100);    // Set Up An Ortho Screen
    glMatrixMode(GL_MODELVIEW);                         // Select The Modelview Matrix
    glPushMatrix();                                     // Store The Modelview Matrix
    glLoadIdentity();                                   // Reset The Modelview Matrix
}

void ExitOrthoMode()
{
	glMatrixMode(GL_PROJECTION);                        // Select The Projection Matrix
    glPopMatrix();                                      // Restore The Old Projection Matrix
    glMatrixMode(GL_MODELVIEW);                         // Select The Modelview Matrix
    glPopMatrix();                                      // Restore The Old Projection Matrix
}

void OutputTextInfo (char* text, unsigned int x, unsigned int y, float color_r, float color_g, float color_b)
{
	glColor3f(color_r, color_g, color_b);
	glRasterPos2d(x, y);

	if ((listBaseTextInfo == 0) || (text == (char*)NULL))
		return;

	glPushAttrib(GL_LIST_BIT);
		glListBase(listBaseTextInfo - 32);
		glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);
	glPopAttrib();
}

void OutputTextBig (char* text, float color_r, float color_g, float color_b, bool fat)
{
	glColor3f(color_r, color_g, color_b);

	if ((listBaseTextBigFat == 0) || (listBaseTextBigThin == 0) || (text == (char*)NULL))
		return;

	glPushAttrib(GL_LIST_BIT);
		if (fat)
			glListBase(listBaseTextBigFat - 32);
		else
			glListBase(listBaseTextBigThin - 32);
		glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);
	glPopAttrib();
}

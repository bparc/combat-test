// Client_Win32_OpenGL.cpp

// "glext.h"
#include "glext.h"
static PFNGLGENVERTEXARRAYSPROC 		glGenVertexArrays;
static PFNGLGENBUFFERSPROC 				glGenBuffers;
static PFNGLBINDBUFFERPROC 				glBindBuffer;
static PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
static PFNGLBINDVERTEXARRAYPROC			glBindVertexArray;
static PFNGLVERTEXATTRIBPOINTERPROC		glVertexAttribPointer;
static PFNGLACTIVETEXTUREPROC 			glActiveTexture;
static PFNGLBUFFERDATAPROC 				glBufferData;
static PFNGLGENERATEMIPMAPPROC 			glGenerateMipmap;
static PFNGLCREATESHADERPROC 			glCreateShader;
static PFNGLGETPROGRAMIVPROC 			glGetProgramiv;
static PFNGLGETPROGRAMINFOLOGPROC 		glGetProgramInfoLog;
static PFNGLDELETESHADERPROC 			glDeleteShader;
static PFNGLATTACHSHADERPROC 			glAttachShader;
static PFNGLLINKPROGRAMPROC 			glLinkProgram;
static PFNGLGETSHADERINFOLOGPROC 		glGetShaderInfoLog;
static PFNGLCREATEPROGRAMPROC 			glCreateProgram;
static PFNGLDELETEPROGRAMPROC 			glDeleteProgram;
static PFNGLSHADERSOURCEPROC 			glShaderSource;
static PFNGLCOMPILESHADERPROC 			glCompileShader;
static PFNGLGETSHADERIVPROC 			glGetShaderiv;
static PFNGLUSEPROGRAMPROC				glUseProgram;
static PFNGLGENFRAMEBUFFERSPROC			glGenFramebuffers;
static PFNGLBINDFRAMEBUFFERPROC			glBindFramebuffer;
static PFNGLFRAMEBUFFERTEXTUREPROC 		glFramebufferTexture;
static PFNGLDRAWBUFFERSPROC				glDrawBuffers;

static void LoadOpenGLFunctionPointer(int32_t *Sucess, const char *Name, void **PFN)
{
	*PFN = (void*)wglGetProcAddress(Name);
	if (!*PFN)
	{
		*Sucess = false;
		Error("Unsupported OpenGL extension! %s", Name);
	}
}

static int32_t LoadOpenGLExtensions(void)
{
	#define Load(Name) LoadOpenGLFunctionPointer(&Success, #Name, (void **)&Name)

	int32_t Success = true;
	Load(glGenVertexArrays);
	Load(glGenBuffers);
	Load(glBindBuffer);
	Load(glBindVertexArray);
	Load(glEnableVertexAttribArray);
	Load(glVertexAttribPointer);
	Load(glActiveTexture);
	Load(glBufferData);
	Load(glGenerateMipmap);
	Load(glCreateShader);
	Load(glGetProgramiv);
	Load(glGetProgramInfoLog);
	Load(glDeleteShader);
	Load(glAttachShader);
	Load(glLinkProgram);
	Load(glGetShaderInfoLog);
	Load(glCreateProgram);
	Load(glDeleteProgram);
	Load(glShaderSource);
	Load(glCompileShader);
	Load(glGetShaderiv);
	Load(glUseProgram);
	Load(glGenFramebuffers);
	Load(glBindFramebuffer);
	Load(glFramebufferTexture);
	Load(glDrawBuffers);

	#undef Load

	return Success;
}

// ...

static GLuint CompileShader(GLenum Type, const char* Source)
{
	GLuint Result = glCreateShader(Type);
	glShaderSource(Result, 1, &Source, 0);
	glCompileShader(Result);
	GLint Status;
	glGetShaderiv(Result, GL_COMPILE_STATUS, &Status);
	if ((Status == GL_FALSE))
	{
		char InfoLog[256] = "";
		glGetShaderInfoLog(Result, sizeof(InfoLog), 0, InfoLog);
		printf("%s\n", InfoLog);
	}
	return Result;
}

static GLuint CompileProgram(const char* VertexSource, const char *FragmentSource)
{
	GLuint Shaders[2] =
	{
		CompileShader(GL_VERTEX_SHADER,VertexSource),
		CompileShader(GL_FRAGMENT_SHADER,FragmentSource),
	};
	GLuint Result = glCreateProgram();
	glAttachShader(Result, Shaders[0]);
	glAttachShader(Result, Shaders[1]);
	glLinkProgram(Result);
	glDeleteShader(Shaders[0]);
	glDeleteShader(Shaders[1]);
	GLint Status;
	glGetProgramiv(Result, GL_LINK_STATUS, &Status);
	if ((Status == GL_FALSE))
	{
		char InfoLog[256] = "";
		glGetProgramInfoLog(Result, sizeof(InfoLog), 0, InfoLog);
		Error("%s", InfoLog);
		glDeleteProgram(Result);
		Result = 0;
	}
	return Result;
}

// ...

struct graphics_device_t
{
	// int32_t Reserved;
	vec2_t Viewport;

	int32_t Inited;

	GLuint VBO;
	GLuint VAO;

#if 0
	GLuint Framebuffer;
	GLuint FramebufferTexture;
#endif

	GLuint BitmapFilter;
};

static void InitializeGraphicsDevice(graphics_device_t *Device)
{
	if(!Device->Inited)
	{
	#if 1
		DebugLog("...");
		LoadOpenGLExtensions();
		glGenVertexArrays(1, &Device->VAO);
		glGenBuffers(1, &Device->VBO);
		glBindVertexArray(Device->VAO);
		glBindBuffer(GL_ARRAY_BUFFER, Device->VBO);
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glVertexPointer(2, GL_FLOAT, sizeof(vertex_t), (void *)offsetof(vertex_t, Offset));
		glColorPointer(4, GL_FLOAT, sizeof(vertex_t), (void *)offsetof(vertex_t, Color));
		glTexCoordPointer(2, GL_FLOAT, sizeof(vertex_t), (void *)offsetof(vertex_t, TexCoord));
		//glEnableVertexAttribArray(0);
		//glEnableVertexAttribArray(4);
		//glEnableVertexAttribArray(3);
		//glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void *)offsetof(vertex_t, Offset));
		//glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void *)offsetof(vertex_t, TexCoord));
		//glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void *)offsetof(vertex_t, Color));
		#endif

		Device->BitmapFilter = CompileProgram(
		"void main(){\n\
		gl_Position =  gl_ModelViewProjectionMatrix * gl_Vertex;\n\
		gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;\n\
		gl_FrontColor = gl_Color;\n\
		}"
		,

		"#version 130\n\
		uniform sampler2D Texture;\n\
		void main(){\n\
		vec2 TextureSize = textureSize(Texture, 0);\n\
		vec2 TexSpaceCoord = gl_TexCoord[0].xy * TextureSize;\n\
		vec2 TexCoord = floor(TexSpaceCoord) + min(fract(TexSpaceCoord) / fwidth(TexSpaceCoord), 1.0) - 0.5;\n\
		vec4 Pixel = texture2D(Texture, TexCoord / TextureSize);\n\
		gl_FragColor = Pixel * gl_Color;\n\
		}");

		#if 0
		glGenFramebuffers(1, &Device->Framebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, Device->Framebuffer);

		glGenTextures(1, &Device->FramebufferTexture);
		glBindTexture(GL_TEXTURE_2D, Device->FramebufferTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1920, 1080, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, Device->FramebufferTexture, 0);

		GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
		glDrawBuffers(1, DrawBuffers);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		#endif

		Device->Inited = true;
	}
}

static void ClearColor(graphics_device_t *Device, vec4_t Color)
{
	glClearColor(Color.x, Color.y, Color.z, Color.w);
	glClear(GL_COLOR_BUFFER_BIT);
}

static void BeginRender(graphics_device_t *Device, vec2_t Viewport)
{
	Assert(Device->Inited);
	Device->Viewport = Viewport;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	ClearColor(Device, {0.05f, 0.05f, 0.05f});

	#if 0
	glBindFramebuffer(GL_FRAMEBUFFER, Device->Framebuffer);
	glClear(GL_COLOR_BUFFER_BIT);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	#endif
}

static void EndRender(graphics_device_t *Device)
{
	#if 0
	glBindFramebuffer(GL_FRAMEBUFFER, 0)
	glBindTexture(GL_TEXTURE_2D, Device->FramebufferTexture);
	glViewport(0, 0, (int32_t)Device->Viewport.x, (int32_t)Device->Viewport.y);

	glPushMatrix();
	glLoadIdentity();
	glBegin(GL_QUADS);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glTexCoord2f(+0.0f, +0.0f); glVertex2f(-1.0f, +1.0f);
	glTexCoord2f(+1.0f, +0.0f); glVertex2f(+1.0f, +1.0f);
	glTexCoord2f(+1.0f, -1.0f); glVertex2f(+1.0f, -1.0f);
	glTexCoord2f(+0.0f, -1.0f); glVertex2f(-1.0f, -1.0f);
	glEnd();
	glPopMatrix();
	#endif
}

static void OutputVertices(const graphics_device_t *Device, vertex_t *Vertices, int32_t Count, GLuint Texture, GLenum Primitive)
{
	//DebugPrint("Dispatch->Count = %i, Texture = %i, Primitive = %i", Count, Texture, Primitive);

	glBindTexture(GL_TEXTURE_2D, Texture);
	glBufferData(GL_ARRAY_BUFFER, sizeof(*Vertices) * Count, Vertices, GL_DYNAMIC_DRAW);

#if 1
	glDrawArrays(Primitive, 0, Count);
#else
	glBegin(Primitive);
	for (int32_t Index = 0; Index < Count; Index++)
	{
		glColor4fv(&Vertices[Index].Color.x);
		glTexCoord2fv(&Vertices[Index].TexCoord.x);
		glVertex2fv(&Vertices[Index].Offset.x);
	}
	glEnd();
#endif
}

static void DispatchBegin(graphics_device_t *Device, vec4_t View, struct transform_t Transform, int32_t Flags)
{
#if 0
	glBindFramebuffer(GL_FRAMEBUFFER, Device->FramebufferTexture);
	#else
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	#endif

	if ((View.z == 0) && (View.w == 0))
	{
		View.z = Device->Viewport.x;
		View.w = Device->Viewport.y;
	}

	if ((Transform.Size.x == 0) && (Transform.Size.y == 0))
	{
		Transform.Size = {Device->Viewport.x, Device->Viewport.y};
		Transform.Center = (Transform.Size * 0.5f);
	}

#if 1
	glViewport(
		(GLsizei)View.x,
		(GLsizei)((Device->Viewport.y - View.w) - View.y),
		(GLint)View.z,
		(GLint)View.w); 
#endif
	//glViewport(0, 0, 1920, 1080);

	glActiveTexture(GL_TEXTURE0);
	glBindBuffer(GL_ARRAY_BUFFER, Device->VBO);
	glBindVertexArray(Device->VAO);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);

	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	glPushMatrix();

	glLoadIdentity();

	glOrtho(0.0, (GLdouble)Transform.Size.x, (GLdouble)Transform.Size.y, 0.0, -1.0, 1.0);
	glTranslatef(-Transform.Center.x + (Transform.Size.x * 0.5f), -Transform.Center.y + (Transform.Size.y * 0.5f), 0.0f);

	glUseProgram(0);

	if (Flags & Dispatch_Pixelated)
	{
		glUseProgram(Device->BitmapFilter);	
	}
}

static void DispatchEnd(graphics_device_t *Device)
{
	glPopMatrix();
}

static void Dispatch(graphics_device_t *Device, const struct command_buffer_t *Cmds, vec4_t View, struct transform_t Transform, int32_t Flags)
{
	Dispatch(Device, Cmds, 0, (Cmds->CmdCount - 1), View, Transform, Flags);
}

static void Dispatch(graphics_device_t *Device, const struct command_buffer_t *Cmds, int32_t First, int32_t Last, vec4_t View, struct transform_t Transform, int32_t Flags)
{
	DispatchBegin(Device, View, Transform, Flags);

	GLenum Primitives[4] = {};
	Primitives[Primitive_Quad] = GL_QUADS;
	Primitives[Primitive_Line] = GL_LINES;

	for (int32_t Index = First; (Index < Cmds->CmdCount) && (Index <= Last); Index++)
	{
		const render_command_t *Cmd = &Cmds->Commands[Index];
		OutputVertices(Device, &Cmds->Vertices[Cmd->Offset], Cmd->Count, (GLuint)Cmd->Texture, Primitives[Cmd->Primitive]);
	}

	DispatchEnd(Device);
}

static texture_resource_t CreateTextureResource(graphics_device_t *Device, int16_t X, int16_t Y, const void *Memory, int32_t Flags)
{
	texture_resource_t Result = {};
	Result.X = X;
	Result.Y = Y;

	glGenTextures(1, &Result.Handle);
	glBindTexture(GL_TEXTURE_2D, Result.Handle);
	
	surface_t Source = CreateSurface(X, Y, (uint32_t *)Memory);

	#if 0
	int32_t MipmapLevels = 1;
	for (int32_t Level = 0; Level < MipmapLevels; Level++)
	{
		surface_t Mipmap = {};
		Mipmap.X = X;
		Mipmap.Y = Y;
		Mipmap.Pixels = (uint32_t *)malloc(sizeof(uint32_t)*Mipmap.X*Mipmap.Y);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Mipmap.X, Mipmap.Y, 0, GL_RGBA, GL_UNSIGNED_BYTE, Mipmap.Pixels);

		free(Mipmap.Pixels);
	}

	#else
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, X, Y, 0, GL_RGBA, GL_UNSIGNED_BYTE, Memory);
	#endif

	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	DebugLog("#%i, %ix%i", Result.Handle, X, Y);

	return Result;
}

static texture_resource_t CreateTextureResource(graphics_device_t *Device, const surface_t *Surface, int32_t Flags)
{
	texture_resource_t Result = CreateTextureResource(Device, (int16_t)Surface->X, (int16_t)Surface->Y, (const void *)Surface->Pixels, Flags);
	return Result;
}

static void FreeTextureResource(graphics_device_t *Device, texture_resource_t *Resource)
{
	if (Resource->Handle > 0)
	{
		glDeleteTextures(1, &Resource->Handle);
		memset(Resource, 0, sizeof(*Resource));
	}
}
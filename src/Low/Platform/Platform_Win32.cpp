// Client_Win32.cpp

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// <GL/gl.h>
#include <GL/gl.h>
#include "Platform_Win32_OpenGL.cpp"

struct client_t
{
	HINSTANCE hInstance;
	HCURSOR hCursor;
	WNDCLASS WndClass;

	HWND Window;
	HGLRC hGLRC;
	HDC hDC;

	assets_t Assets;
	char Name[32];
	int32_t Running;

	graphics_device_t GraphicsDevice;
	int32_t VideoMode[2];

	input_t Input;
	double Time;
};

//

static LRESULT WINAPI WndProc(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	LRESULT result = 0;
	switch (uMsg)
	{
#if 1
	case WM_CLOSE:
	{
		client_t *Client = (client_t *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		Client->Running = false;
		// Running = 0;
	} break;
#endif
#if 0
	case WM_CREATE:
	{
		SetCursor(hCursor);
	} break;
#endif
	case WM_GETMINMAXINFO:
	{
		LPMINMAXINFO Info = (LPMINMAXINFO)lParam;
		Info->ptMinTrackSize.x = 256 * 2;
		Info->ptMinTrackSize.y = 224 * 2;
	} break;
	default:
	{
		result = DefWindowProc(hWnd, uMsg, wParam, lParam);
	} break;
	}
	return result;
}

static client_t *CreateClient(const char *Name, int32_t Flags)
{
	client_t *Result = (client_t *)malloc(sizeof(client_t));
	memset(Result, 0, sizeof(*Result));

	Result->hInstance = GetModuleHandle(NULL);
	Result->hCursor = LoadCursor(NULL, IDC_ARROW);

	ZeroMemory(&Result->WndClass, sizeof(Result->WndClass));
	Result->WndClass.lpfnWndProc = WndProc;
	Result->WndClass.hInstance = Result->hInstance;
	Result->WndClass.lpszClassName = "WndClass";
	RegisterClass(&Result->WndClass);

	Result->Running = false;
	strncpy(Result->Name, Name, Len(Result->Name));

	return Result;
}

static int32_t SetVideoMode(client_t *Client, int32_t X, int32_t Y)
{
	// HWND

	int32_t Style = WS_OVERLAPPEDWINDOW;
	RECT WindowRect = {0, 0, X, Y};
	AdjustWindowRect(&WindowRect, Style, FALSE);

	if (Client->Window)
	{
		DestroyWindow(Client->Window);	
	}

	char Title[64] = "";
#ifdef _DEBUG
	snprintf(Title, Len(Title), "%s.exe _DEBUG (%s %s)", Client->Name, __DATE__, __TIME__);
#else
	snprintf(Title, Len(Title), "%s", Client->Name);
#endif

	Client->Window = CreateWindowEx(0, Client->WndClass.lpszClassName, Title, Style | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
		(WindowRect.right - WindowRect.left), (WindowRect.bottom - WindowRect.top), 0, 0, Client->hInstance, 0);

	SetWindowLongPtr(Client->Window, GWLP_USERDATA, (LONG_PTR)Client);
	Client->hDC = GetDC(Client->Window);

	// HGLRC

	static const PIXELFORMATDESCRIPTOR PixelFormat =
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		32,
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		24,
		8,
		0,
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};

	int32_t BestPixelFormat = ChoosePixelFormat(Client->hDC, &PixelFormat);
	SetPixelFormat(Client->hDC, BestPixelFormat, &PixelFormat);

	Client->hGLRC = wglCreateContext(Client->hDC);
	wglMakeCurrent(Client->hDC, Client->hGLRC);

	//

	Client->Running = true;
	Client->VideoMode[0] = X;
	Client->VideoMode[1] = Y;

	Setup(&Client->Assets, &Client->GraphicsDevice);

	return true;
}

static void UpdateButton(button_t *Button, int32_t State, float_t DeltaTime)
{
	int32_t Prev = Button->State;
	Button->State = (uint8_t)State;
	Button->Down = !Prev && State;
	Button->Up = Prev && !State;
}

static double GetTime(void)
{
	double result = 0;

	LARGE_INTEGER frequency = {0};
	if (QueryPerformanceFrequency(&frequency))
	{
		LARGE_INTEGER time = {0};
		if (QueryPerformanceCounter(&time))
		{
			result = ((double)time.QuadPart / (double)frequency.QuadPart);
		}
	}
	return result;
}

static void AcquireInputs(HWND Window, input_t *Input, float_t DeltaTime)
{
	Input->DeltaTime = DeltaTime;

	RECT ClientRect = {};
	if (GetClientRect(Window, &ClientRect))
	{
		Input->Viewport.x = (float_t)(ClientRect.right - ClientRect.left);
		Input->Viewport.y = (float_t)(ClientRect.bottom - ClientRect.top);
	}	

	for (int32_t Index = 0; Index < Len(Input->Controllers); Index++)
	{
		controller_t *Con = &Input->Controllers[0];

		Con->Analogs[0] = {};
		Con->Analogs[1] = {};

		if (GetAsyncKeyState('W'))
			Con->Analogs[0].y -= 1.0f;
		if (GetAsyncKeyState('S'))
			Con->Analogs[0].y += 1.0f;
		if (GetAsyncKeyState('A'))
			Con->Analogs[0].x -= 1.0f;
		if (GetAsyncKeyState('D'))
			Con->Analogs[0].x += 1.0f;

		Con->DPad = {};

		if (GetAsyncKeyState(VK_UP) < 0)
			Con->DPad.y -= 1;
		else
		if (GetAsyncKeyState(VK_DOWN) < 0)
			Con->DPad.y += 1;
		else
		if (GetAsyncKeyState(VK_LEFT) < 0)
			Con->DPad.x -= 1;
		else
		if (GetAsyncKeyState(VK_RIGHT) < 0)
			Con->DPad.x += 1;

		UpdateButton(&Con->A, GetAsyncKeyState(VK_SPACE) < 0, Input->DeltaTime);

		Con->Analogs[0] = Normalize(Con->Analogs[0]);
		Con->Analogs[1] = Normalize(Con->Analogs[1]);
	}
}

static int32_t Host(client_t *Client, input_t *Input)
{
	UpdateAssetCache(&Client->Assets, &Client->GraphicsDevice);

	MSG Message;
	while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}

	double Time = Client->Time;
	Client->Time = GetTime();

	float_t DeltaTime = (float_t)(Client->Time - Time);

	AcquireInputs(Client->Window, &Client->Input, DeltaTime);

	if (Input)
	{	
		memcpy(Input, &Client->Input, sizeof(*Input));
	}
	
	BeginRender(&Client->GraphicsDevice, Input->Viewport);
	BeginDebugFrame(Client, &Client->Input);
	return (Client->Running > 0);
}

static void Present(client_t *Client)
{
	EndDebugFrame();
	EndRender(&Client->GraphicsDevice);

	SwapBuffers(Client->hDC);
}

static graphics_device_t *GetGraphicsDevice(client_t *Client)
{
	return (&Client->GraphicsDevice);
}

static assets_t *GetAssets(client_t *Client)
{
	return (&Client->Assets);
}

static file_contents_t LoadFileFromDirectory(const char *Directory)
{
	file_contents_t Result = {};
	int32_t Succeded = false;

	FILE *File = fopen(Directory, "rb");
	if (File)
	{
		fseek(File, 0, SEEK_END);
		Result.Length = ftell(File);
		fseek(File, 0, SEEK_SET);

		Result.Data = (uint8_t *)malloc((Result.Length + 1));
		if (Result.Data)
		{
			Result.Data[Result.Length] = 0;
			if (fread(Result.Data, Result.Length, 1, File) == 1)
			{
				Succeded = true;
			}	
		}

		fclose(File);
	}

	if (!Succeded)
	{
		FreeFileContents(&Result);
	}

	return Result;
}

static void FreeFileContents(file_contents_t *Contents)
{
	if (Contents->Data)
	{
		free(Contents->Data);
		memset(Contents, 0, sizeof(*Contents));
	}
}

static void Error(const char *_format, ...)
{
	va_list List = {0};
	va_start(List, _format);
	char message[512] = "";
	vsnprintf(message, Len(message), _format, List);
	va_end(List);
	
	MessageBox(0, message, 0, MB_ICONERROR);
	ExitProcess(1);
}

static void _Assert(const char *_message, const char *_file, const char *_function, int32_t _line)
{
	Error("Assertion failed! \n\nProgram: %s\nLine: %i\n\nFunction: %s\nExpression: %s", _file, _line, _function, _message);
}
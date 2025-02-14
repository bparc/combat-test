#include "Common.h"
#include "Common.cpp"

#include "Device.h"
#include "Assets.h"
#include "Assets.cpp"

typedef struct client_t client_t;

static client_t *CreateClient(const char *Name = NULL, int32_t Flags = 0);
static int32_t SetVideoMode(client_t *Client, int32_t X, int32_t Y);

static graphics_device_t *GetGraphicsDevice(client_t *Client);
static assets_t *GetAssets(client_t *Client);

// Input

struct button_t
{
	uint8_t State;
	uint8_t Up;
	uint8_t Down;
};

struct controller_t
{
	bool Connected;
	vec2_t Analogs[2]; // NOTE: Dead-zones already applied!
	float_t Triggers[2];
	point_t DPad;
	
	button_t A, B, Y, X; // XBOX controller buttons

	button_t DebugKeys[12]; // Mapped to F1 - F12
};

struct input_t
{
	vec2_t Viewport;
	vec2_t MouseCursor;
	button_t MouseButtons[2];

	controller_t Controllers[1];
	float_t DeltaTime;
};

// AcquireInputs() (Platform/Platform_*.cpp)

// ...

static int32_t Host(client_t *Client, /*Out*/ input_t *Input);
static void Present(client_t *Client);

static void Display(client_t *Client); // SwapBuffers()

#include "Renderer.h"
#include "Renderer.cpp"

#include "Debug.h"
#include "Debug.cpp"
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
	vec2_t Analogs[2]; // NOTE: Dead-zones already applied
	point_t DPad;
	
	button_t A, B, Y, X; // XBOX
};

struct input_t
{
	vec2_t Viewport;
	controller_t Controllers[1];
	float_t DeltaTime;
};

// ...

static int32_t Host(client_t *Client, /*Out*/ input_t *Input);
static void Present(client_t *Client);

#include "Renderer.h"
#include "Renderer.cpp"

#include "Debug.h"
#include "Debug.cpp"

// Log

struct log_line_t
{
	uint8_t Level;
	double_t TimeStamp;
	char Text[256];
};

struct log_t
{
	double_t ElapsedTime;
	int32_t LineCount;
	log_line_t Lines[64]; // this is a ring buffer
};

static void LogLn(log_t *Log, const char *Format, va_list List, int32_t Level = 0);
static void MessageLog(render_output_t *Out, log_t *Log, vec2_t At, float DeltaTime);

// ...

struct debug_state_t
{
	// Visual Debugging

	vec4_t DebugView;
	transform_t DebugCamera;
	render_output_t DebugOutput; 


	int32_t EnableDebugRenderer; //// DebugRect, DebugLine, etc.
	int32_t Inited;

	command_buffer_t Cmds[2];

	// ...

	// UI
	render_output_t Foreground;
	vec2_t PrintAt;

	log_t Log;
	//

	memory_t Memory;
	uint8_t DebugMemory[MB(10)];

	client_t *Client;

	graphics_device_t *Device;
	assets_t *Assets;

	float_t DeltaTime;
	vec2_t Viewport;
};

// Visual Debugging

static void BeginVisualDebugMode(vec4_t DebugView, transform_t DebugTrasform);
static void EndVisualDebugMode(void);

static void _DebugRect(vec2_t Offset, vec2_t Size, vec4_t Color);
static void _DebugLine(vec2_t A, vec2_t B, vec4_t Color);
static void _DebugCircle(vec2_t Offset, float_t Radius, vec4_t Color);
static void _DebugPoint(vec2_t Offset, vec4_t Color);
// UI

static void DebugPrint(const char *Format, ...);
static void _DebugLog(const char *Format, ...);

//

#ifdef _DEBUG
#define DebugRect(Offset, Size, Color) _DebugRect(Offset, Size, Color)
#define DebugLine(A, B, Color) _DebugLine(A, B, Color)
#define DebugCircle(Offset, Radius, Color) _DebugCircle(Offset, Radius, Color)
#define DebugPoint(Offset, Color) _DebugPoint(Offset, Color)
#else
#define DebugRect(...)
#define DebugLine(...)
#define DebugCircle(...)
#define DebugPoint(...)
#endif

// ...

static debug_state_t Debug;

// ...

typedef uint32_t hash_t;

struct interface_draw_command_t
{
	uint16_t First;
	uint16_t Last;
	interface_draw_command_t *Next;
};

struct window_t
{
	rect_t ClientBounds;
	hash_t Name;
	interface_draw_command_t *DrawListHead;
	interface_draw_command_t *DrawListBack;
	bool Inited;

	const char *Title;

	vec2_t LayoutCursor;
};

struct interface_t
{
	#define COUNT 32
	array_t<uint16_t,COUNT>SortBuffer;
	window_t Windows[COUNT];
	//uint16_t SortBuffer[COUNT];
	#undef COUNT

	//

	bool InterceptInputs;
	vec2_t MouseCursor;
	bool Interact[2];

	//
	vec2_t ClickOffset;
	hash_t Hovered;
	hash_t Active;
	window_t *Wnd;
	hash_t Root;

	//

	bool RenderOutputInvalid;
	render_output_t Out;

	command_buffer_t CommandBuffer;
	vertex_t Vertices[MB(10)];
	render_command_t Commands[1024];

	array_t<interface_draw_command_t,256>WindowCommands;
};

static void InitializeUserInterface(interface_t *State);

static void BeginUserInterface(interface_t *State, const input_t *Input, assets_t *Assets);
static void EndUserInterface(interface_t *State);

static void Render(interface_t *State, graphics_device_t *Device);

static bool BeginWindow(interface_t *State, hash_t WindowName, vec2_t Offset, vec2_t Size, const char *Title);
static void EndWindow(interface_t *State);

static bool IsHovered(const interface_t *State, rect_t Bb);
static bool IsHovered(const interface_t *State, hash_t ID);
static bool IsActive(const interface_t *State, hash_t ID);
static bool Interact(interface_t *State, rect_t Bb, hash_t ID);

static hash_t HashName(const interface_t *State, const char *Name);

static bool Button(interface_t *State, const char *Name);

// INTERNAL:

static void BindWindow(interface_t *State, window_t *Window);
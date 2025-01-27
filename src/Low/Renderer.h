// ...

struct transform_t
{
	vec2_t GlobalOffset;
	vec2_t Size;
};

static transform_t Transform(vec2_t Offset, vec2_t Size);

//

struct vertex_t
{
	vec2_t Offset;
	vec2_t TexCoord;
	vec4_t Color;
};

enum primitive_t
{
	Primitive_Quad,
	Primitive_Line,
};

struct render_command_t
{
	uint8_t Primitive;
	uint32_t Texture;
	int32_t Offset;
	int32_t Count;
};

struct command_buffer_t
{
	render_command_t CmdMemory[8];

	int32_t MaxCmdCount;
	int32_t CmdCount;
	render_command_t *Commands;

	int32_t MaxVertexCount;
	int32_t VertexCount;
	vertex_t *Vertices;
};

static void Setup(command_buffer_t *Cmds, vertex_t *Vertices, int32_t VertexCount, render_command_t *Commands, int32_t CommandCount);
static void SetupFromSize(command_buffer_t *Cmds, memory_t *Memory, int32_t VertexBufferSizeInBytes, int32_t CmdCount = 0);
static void Clear(command_buffer_t *Cmds, int32_t Count);
static void Clear(command_buffer_t *Cmds);

static vertex_t *ReserveVertices(command_buffer_t *Cmds, int32_t Count, primitive_t Prim, uint32_t Texture = 0);
static void AppendQuad(command_buffer_t *Cmds, vec2_t A, vec2_t B, vec2_t C, vec2_t D, vec4_t Color, uint32_t Texture = 0, vec2_t TexCoordMin = {}, vec2_t TexCoordMax = {});
static void AppendQuad(command_buffer_t *Cmds, vec2_t Offset, vec2_t Size, vec4_t Color, uint32_t Texture = 0, vec2_t TexCoordMin = {}, vec2_t TexCoordMax = {});
static void AppendLine(command_buffer_t *Cmds, vec2_t A, vec2_t B, vec4_t Color);

//

struct render_output_t
{
	command_buffer_t *Out;
	assets_t *Assets;

	float_t LineThickness; // Thickness = 1.0f + LineThickness
};

static render_output_t RenderTo(command_buffer_t *Out, assets_t *Assets);

// Primitive_Quad
static void RenderRect(render_output_t *Out, vec2_t Offset, vec2_t Size, vec4_t Color);
static void RenderRect(render_output_t *Out, rect_t Rectangle, vec4_t Color);
static void RenderPoint(render_output_t *Out, vec2_t Offset, vec4_t Color);

static void RenderBitmap(render_output_t *Out, vec2_t Offset, bitmap_t Bitmap, vec4_t Color = {1, 1, 1, 1});
static void RenderBitmap(render_output_t *Out, vec2_t Offset, bitmap_t Bitmap, vec2_t UVMin, vec2_t UVMax, vec4_t Color = {1, 1, 1, 1});

static void RenderString(render_output_t *Out, vec2_t Offset, const char *Text, vec4_t Color = {1, 1, 1, 1});

// Scaled
static void RenderBitmapScaled(render_output_t *Out, vec2_t Offset, bitmap_t Bitmap, vec2_t Size, vec4_t Color = {1, 1, 1, 1});
static void RenderBitmapScaled(render_output_t *Out, vec2_t Offset, bitmap_t Bitmap, vec2_t Size, vec2_t UVMin, vec2_t UVMax, vec4_t Color = {1, 1, 1, 1});

// Primitive_Line
static void PushLineThickness(render_output_t *Out, float_t Thickness);
static void RenderLine(render_output_t *Out, vec2_t From, vec2_t To, vec4_t Color);
static void RenderRectOutline(render_output_t *Out, vec2_t Offset, vec2_t Size, vec4_t Color);
static void RenderCircleOutline(render_output_t *Out, vec2_t Offset, float_t Radius, vec4_t Color);
// Client_Win32_OpenGL.cpp

struct graphics_device_t
{
	// int32_t Reserved;
	vec2_t Viewport;
};

static void ClearColor(graphics_device_t *Device, vec4_t Color)
{
	glClearColor(Color.x, Color.y, Color.z, Color.w);
	glClear(GL_COLOR_BUFFER_BIT);
}

static void BeginRender(graphics_device_t *Device, vec2_t Viewport)
{
	Device->Viewport = Viewport;
	ClearColor(Device, {0.05f, 0.05f, 0.05f});
}

static void EndRender(graphics_device_t *Device)
{

}

static void OutputVertices(vertex_t *Vertices, int32_t Count, GLuint Texture, GLenum Primitive)
{
	glBindTexture(GL_TEXTURE_2D, Texture);

	glBegin(Primitive);
	for (int32_t Index = 0; Index < Count; Index++)
	{
		glColor4fv(&Vertices[Index].Color.x);
		glTexCoord2fv(&Vertices[Index].TexCoord.x);
		glVertex2fv(&Vertices[Index].Offset.x);
	}
	glEnd();
}

static void Dispatch(graphics_device_t *Device, const struct command_buffer_t *Cmds, vec4_t View, struct transform_t Transform)
{
	if ((View.z == 0) && (View.w == 0))
	{
		View.z = Device->Viewport.x;
		View.w = Device->Viewport.y;
	}

	if ((Transform.Size.x == 0) && (Transform.Size.y == 0))
	{
		Transform.Size = {Device->Viewport.x, Device->Viewport.y};
	}

	glViewport(
		(GLsizei)View.x,
		(GLsizei)((Device->Viewport.y - View.w) - View.y),
		(GLint)View.z,
		(GLint)View.w); 
		
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	GLenum Primitives[4] = {};
	Primitives[Primitive_Quad] = GL_QUADS;
	Primitives[Primitive_Line] = GL_LINES;

	glPushMatrix();
	glOrtho(0.0, (GLdouble)Transform.Size.x, (GLdouble)Transform.Size.y, 0.0, -1.0, 1.0);
	for (int32_t Index = 0; Index < Cmds->CmdCount; Index++)
	{
		const render_command_t *Cmd = &Cmds->Commands[Index];
		OutputVertices(&Cmds->Vertices[Cmd->Offset], Cmd->Count, (GLuint)Cmd->Texture, Primitives[Cmd->Primitive]);
	}

	glPopMatrix();
}

static texture_resource_t CreateTextureResource(graphics_device_t *Device, int16_t X, int16_t Y, const void *Memory, int32_t Flags)
{
	texture_resource_t Result = {};
	Result.X = X;
	Result.Y = Y;

	glGenTextures(1, &Result.Handle);
	glBindTexture(GL_TEXTURE_2D, Result.Handle);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Result.X, Result.Y, 0, GL_RGBA, GL_UNSIGNED_BYTE, Memory);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	DebugLog("");

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

static transform_t Transform(vec2_t Offset, vec2_t Size)
{
	transform_t Result = {};
	Result.GlobalOffset = Offset;
	Result.Size = Size;
	return Result;
}

static void Setup(command_buffer_t *Cmds, vertex_t *Vertices, int32_t VertexCount, render_command_t *Commands, int32_t CommandCount)
{
	memset(Cmds, 0, sizeof(*Cmds));
	Cmds->MaxCmdCount = CommandCount;
	Cmds->Commands = Commands;
	Cmds->MaxVertexCount = VertexCount;
	Cmds->Vertices = Vertices;
}

static void SetupFromSize(command_buffer_t *Cmds, memory_t *Memory, int32_t VertexBufferSizeInBytes, int32_t CmdCount)
{
	memset(Cmds, 0, sizeof(*Cmds));

	Cmds->MaxVertexCount = VertexBufferSizeInBytes / sizeof(*Cmds->Vertices);
	Cmds->Vertices = (vertex_t *)Push(Memory, Cmds->MaxVertexCount * sizeof(*Cmds->Vertices));

	if (Cmds->CmdCount)
	{
		Cmds->MaxCmdCount = CmdCount;
		Cmds->Commands = (render_command_t *)Push(Memory, Cmds->MaxCmdCount * sizeof(*Cmds->Commands));
	}
	else
	{
		Cmds->MaxCmdCount = Len(Cmds->CmdMemory);
		Cmds->Commands = Cmds->CmdMemory;
	}
}

static void Clear(command_buffer_t *Cmds, int32_t Count)
{
	for (int32_t Index = 0; Index < Count; Index++)
	{
		Clear(&Cmds[Index]);
	}
}

static void Clear(command_buffer_t *Cmds)
{
	Cmds->VertexCount = 0;
	Cmds->CmdCount = 0;
}

static render_command_t *TryMerge(const command_buffer_t *Cmds, int32_t Count, primitive_t Prim, uint32_t Texture)
{
	render_command_t *Result = 0;
	if (Cmds->CmdCount > 0)
	{
		render_command_t *Top = &Cmds->Commands[Cmds->CmdCount - 1];
		if ((Top->Texture == Texture) && (Top->Primitive == Prim))
		{
			Result = Top;
		}
	}
	return Result;
}

static vertex_t *ReserveVertices(command_buffer_t *Cmds, int32_t Count, primitive_t Prim, uint32_t Texture)
{
	vertex_t *Result = NULL;
	render_command_t *Command;

	Command = TryMerge(Cmds, Count, Prim, Texture);
	
	if (!Command && (Cmds->CmdCount < Cmds->MaxCmdCount)) // Couldn't merge...
	{
		Command = &Cmds->Commands[Cmds->CmdCount++];
		memset(Command, 0, sizeof(*Command));
		Command->Primitive = (uint8_t)Prim;
		Command->Texture = Texture;
		Command->Offset = Cmds->VertexCount;
	}
	
	if (Command && (Cmds->VertexCount + Count) < Cmds->MaxVertexCount) // Reserve...
	{
		Result = &Cmds->Vertices[Cmds->VertexCount];
		Cmds->VertexCount += Count;
		Command->Count += Count;
	}

	return Result;
}

static void AppendQuad(command_buffer_t *Cmds, vec2_t A, vec2_t B, vec2_t C, vec2_t D, vec4_t Color, uint32_t Texture, vec2_t TexCoordMin, vec2_t TexCoordMax)
{
	vertex_t *Vertices = ReserveVertices(Cmds, 4, Primitive_Quad, Texture);
	if (Vertices)
	{
		Vertices[0].Offset = A;
		Vertices[1].Offset = B;
		Vertices[2].Offset = C;
		Vertices[3].Offset = D;
		
		Vertices[0].TexCoord = {TexCoordMin.x, TexCoordMin.y};
		Vertices[1].TexCoord = {TexCoordMax.x, TexCoordMin.y};
		Vertices[2].TexCoord = {TexCoordMax.x, TexCoordMax.y};
		Vertices[3].TexCoord = {TexCoordMin.x, TexCoordMax.y};

		Vertices[0].Color = Color;
		Vertices[1].Color = Color;
		Vertices[2].Color = Color;
		Vertices[3].Color = Color;
	}
}

static void AppendQuad(command_buffer_t *Cmds, vec2_t Offset, vec2_t Size, vec4_t Color, uint32_t Texture, vec2_t TexCoordMin, vec2_t TexCoordMax)
{
	vec2_t Min, Max;
	Min = Offset;
	Max = Offset + Size;

	AppendQuad(Cmds, {Min.x, Min.y}, {Max.x, Min.y}, {Max.x, Max.y}, {Min.x, Max.y}, Color, Texture, TexCoordMin, TexCoordMax);
}

static void AppendLine(command_buffer_t *Cmds, vec2_t A, vec2_t B, vec4_t Color)
{
	vertex_t *Vertices = ReserveVertices(Cmds, 2, Primitive_Line, 0);
	if (Vertices)
	{
		Vertices[0].Offset = A;
		Vertices[1].Offset = B;
		Vertices[0].Color = Color;
		Vertices[1].Color = Color;
	}
}

static render_output_t RenderTo(command_buffer_t *Out, assets_t *Assets)
{
	render_output_t Result = {};
	Result.Assets = Assets;
	Result.Out = Out;
	return Result;
}

static void RenderTexturedQuad(render_output_t *Out, vec2_t Offset, vec2_t Size, vec4_t Color, texture_resource_t Texture, vec2_t TexCoordMin, vec2_t TexCoordMax)
{
	vec2_t TexCoordScale = V2(Texture.X, Texture.Y);

	TexCoordMin = TexCoordMin / TexCoordScale;
	TexCoordMax = TexCoordMax / TexCoordScale;

	AppendQuad(Out->Out, Offset, Size, Color, Texture.Handle, TexCoordMin, TexCoordMax);
}

static void RenderTexturedQuad(render_output_t *Out, vec2_t Offset, vec2_t Size, vec4_t Color, texture_resource_t Texture)
{
	RenderTexturedQuad(Out, Offset, Size, Color, Texture, {}, V2(Texture.X, Texture.Y));
}

static void RenderTexturedQuad(render_output_t *Out, vec2_t Offset, vec2_t Size, vec4_t Color, texture_resource_t Texture, rect_t TexCoords)
{
	RenderTexturedQuad(Out, Offset, Size, Color, Texture, TexCoords.Offset, TexCoords.Offset + TexCoords.Size);
}

static void RenderRect(render_output_t *Out, vec2_t Offset, vec2_t Size, vec4_t Color)
{
	asset_bitmap_t *Asset = GetBitmap(Out->Assets, Out->Assets->None);
	if (Asset)
	{
		RenderTexturedQuad(Out, Offset, Size, Color, Out->Assets->Cache.Texture, Asset->Min + V2(1), Asset->Max);
	}
}

static void RenderRect(render_output_t *Out, rect_t Rectangle, vec4_t Color)
{
	RenderRect(Out, Rectangle.Offset, Rectangle.Size, Color);
}

static void RenderRectOutline(render_output_t *Out, vec2_t Offset, vec2_t Size, vec4_t Color)
{
	vec2_t Min = Offset;
	vec2_t Max = Offset + Size;
	RenderLine(Out, {Min.x, Min.y}, {Max.x, Min.y}, Color); 
	RenderLine(Out, {Max.x, Min.y}, {Max.x, Max.y}, Color); 
	RenderLine(Out, {Max.x, Max.y}, {Min.x, Max.y}, Color); 
	RenderLine(Out, {Min.x, Min.y}, {Min.x, Max.y}, Color); 
}

static void RenderCircleOutline(render_output_t *Out, vec2_t Offset, float_t Radius, vec4_t Color)
{
	int32_t PointCount = 32;
	for (int32_t Index = 0; Index < PointCount; Index++)
	{
		float_t T0 = (float_t)(Index + 0) / (float_t)PointCount;
		float_t T1 = (float_t)(Index + 1) / (float_t)PointCount;
		vec2_t P0 = {Sin(T0), Cos(T0)};
		vec2_t P1 = {Sin(T1), Cos(T1)};

		P0 = (P0 * Radius) + Offset;
		P1 = (P1 * Radius) + Offset;

		RenderLine(Out, P0, P1, Color);
	}
}

static void RenderBitmap(render_output_t *Out, vec2_t Offset, bitmap_t Bitmap, vec4_t Color)
{
	asset_bitmap_t *Asset = GetBitmap(Out->Assets, Bitmap);
	if (Asset)
	{
		RenderTexturedQuad(Out, Offset, Asset->Max - Asset->Min, Color, Out->Assets->Cache.Texture, Asset->Min, Asset->Max);
	}
}

static void RenderBitmap(render_output_t *Out, vec2_t Offset, bitmap_t Bitmap, vec2_t UVMin, vec2_t UVMax, vec4_t Color)
{
	asset_bitmap_t *Asset = GetBitmap(Out->Assets, Bitmap);
	if (Asset)
	{
		vec2_t Min = Asset->Min + UVMin;
		vec2_t Max = Min + UVMax;
		Min = Clamp(Min, Asset->Min, Asset->Max);
		Max = Clamp(Max, Asset->Min, Asset->Max);
		RenderTexturedQuad(Out, Offset, Asset->Max - Asset->Min, Color, Out->Assets->Cache.Texture, Min, Max);
	}
}

static void RenderBitmapScaled(render_output_t *Out, vec2_t Offset, bitmap_t Bitmap, vec2_t Size, vec2_t UVMin, vec2_t UVMax, vec4_t Color)
{
	asset_bitmap_t *Asset = GetBitmap(Out->Assets, Bitmap);
	if (Asset)
	{
		vec2_t Min = Asset->Min + UVMin;
		vec2_t Max = Min + (UVMax - UVMin);
		Min = Clamp(Min, Asset->Min, Asset->Max);
		Max = Clamp(Max, Asset->Min, Asset->Max);
		RenderTexturedQuad(Out, Offset, Size, Color, Out->Assets->Cache.Texture, Min, Max);
	}
}

static void RenderBitmapScaled(render_output_t *Out, vec2_t Offset, bitmap_t Bitmap, vec2_t Size, vec4_t Color)
{
	asset_bitmap_t *Asset = GetBitmap(Out->Assets, Bitmap);
	if (Asset)
	{
		RenderTexturedQuad(Out, Offset, Size, Color, Out->Assets->Cache.Texture, Asset->Min, Asset->Max);
	}
}

static void _RenderString(render_output_t *Out, vec2_t Offset, bitmap_t Bitmap, const bmfont_t *Info, vec4_t Color, const char *Text)
{
	const char *At = Text;
	Offset.x = floorf(Offset.x);
	Offset.y = floorf(Offset.y);

	while (*At)
	{
		uint8_t Codepoint = *At++;
		const bmfont_char_t *Ch = &Info->Chars[Codepoint];

		vec2_t TexCoordMin, TexCoordMax;
		vec2_t TextureScale;
		vec2_t QuadSize = V2(Ch->Width, Ch->Height);

		TexCoordMin = V2(Ch->X, Ch->Y);
		TexCoordMax = TexCoordMin + QuadSize;

		vec2_t QuadOffset = Offset;
		QuadOffset.x += (float)Ch->XOffset;
		QuadOffset.y += (float)Ch->YOffset;

		RenderBitmapScaled(Out, QuadOffset, Bitmap, QuadSize, TexCoordMin, TexCoordMax, Color);

		Offset.x = (float_t)((int32_t)Offset.x + (Ch->XAdvance));
	}
}

static void RenderString(render_output_t *Out, vec2_t Offset, const char *Text, vec4_t Color)
{
	font_t Font = Out->Assets->FontDefault;
	if (Font)
	{
		Offset.x = floorf(Offset.x);
		Offset.y = floorf(Offset.y);

		_RenderString(Out, Offset - V2(1, 1), Font->Bitmap, &Font->Info, ColorBlack, Text);
		_RenderString(Out, Offset, Font->Bitmap, &Font->Info, Color, Text);
	}
}

static void RenderLine(render_output_t *Out, vec2_t From, vec2_t To, vec4_t Color)
{
	AppendLine(Out->Out, From, To, Color);
}

static void RenderPoint(render_output_t *Out, vec2_t Offset, vec4_t Color)
{
	RenderRect(Out, Offset - V2(0.5f), V2(1), Color);
}

static void PushLineThickness(render_output_t *Out, float_t Thickness)
{
	Out->LineThickness = (-1.0f + Thickness);
}
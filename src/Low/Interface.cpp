
static void InitializeUserInterface(interface_t *State)
{
	memset(State, 0, sizeof(*State));
}

static void BeginUserInterface(interface_t *State, const input_t *Input, assets_t *Assets)
{
	Assert(!State->RenderOutputInvalid);
	State->RenderOutputInvalid = true;

	Setup(&State->CommandBuffer, State->Vertices, Len(State->Vertices), State->Commands, Len(State->Commands));
	State->Out = RenderTo(&State->CommandBuffer, Assets);

	State->WindowCommands.Clear();

	State->MouseCursor = Input->MouseCursor;
	State->Interact[0] = Input->MouseButtons[0].State;
	State->Interact[1] = Input->MouseButtons[1].State;

	//
		
	State->Hovered = 0;
	State->Root = UINT32_MAX;

	for (int32_t Index = 0; Index < State->SortBuffer.Count; Index++)
	{
		const window_t *Window = &State->Windows[State->SortBuffer[Index]];
		if (Contains(Window->ClientBounds, State->MouseCursor))
		{
			State->Root = Window->Name;
			if (Input->MouseButtons[0].Down &&
				(Index > 0)) // Focus
			{
				#if 1
				memmove(
					&State->SortBuffer.Values[1],
					&State->SortBuffer.Values[0],
					(Index) * sizeof(*State->SortBuffer.Values));
				#endif

				State->SortBuffer[0] = (uint16_t)Window->Name;
			}
			break;	
		}
	}

	//

	State->InterceptInputs = (State->Root != UINT32_MAX) || (State->Active);
}

static void EndUserInterface(interface_t *State)
{
	Assert(State->RenderOutputInvalid);
	State->RenderOutputInvalid = false;
}

static void _RenderDrawList(graphics_device_t *Device, command_buffer_t *CommandBuffer, interface_draw_command_t *Head)
{
	interface_draw_command_t *Cmd = Head;
	while (Cmd)
	{
		Dispatch(Device, CommandBuffer, Cmd->First, Cmd->Last, {}, {});
		Cmd = Cmd->Next;
	}
}

static void Render(interface_t *State, graphics_device_t *Device)
{
	Assert(!State->RenderOutputInvalid);

	//Dispatch(Device, &State->CommandBuffer, {}, {});
	for (int32_t Index = (State->SortBuffer.Count - 1); Index >= 0; Index--)
	//for (int32_t Index = 0; Index < 1; /*State->SortBuffer.Count;*/ Index++)
	{
		window_t *Window = &State->Windows[State->SortBuffer[Index]];

		_RenderDrawList(Device, &State->CommandBuffer, Window->DrawListHead);

		Window->DrawListHead = NULL;
		Window->DrawListBack = NULL;
	}
}

static bool BeginWindow(interface_t *State, hash_t WindowIndex, vec2_t Offset, vec2_t Size, const char *Title)
{
	Assert(WindowIndex < Len(State->Windows));
	window_t *Window = &State->Windows[WindowIndex];

	if (!Window->Inited)
	{
		Window->Inited = true;
		Window->Title = Title;

		Window->ClientBounds = Rect(Offset, Size);
		Window->Name = WindowIndex;

		uint16_t *Value = State->SortBuffer.Push();
		*Value = (uint16_t)WindowIndex;
	}

	//

	interface_draw_command_t *Cmd = State->WindowCommands.Push();
	memset(Cmd, 0, sizeof(*Cmd));

	render_command_t *Reserved = AppendCommand(&State->CommandBuffer, Primitive_Quad, GetTextureCacheHandle(State->Out.Assets));
	Assert(Reserved);

	#if 1
	Cmd->First = ((uint16_t)State->CommandBuffer.CmdCount - 1);

	if (!Window->DrawListHead)
	{
		Window->DrawListBack = Window->DrawListHead = Cmd;
	}
	else
	{
		Window->DrawListBack->Next = Cmd;
		Window->DrawListBack = Cmd;
	}
	#endif

	// Bind

	BindWindow(State, Window);

#if 1
	if (Window->DrawListBack == Window->DrawListHead) // First BeginWindow() call of the current frame
	{
		rect_t ClientArena = Window->ClientBounds;
		rect_t Bar = Rect(ClientArena.Offset, {ClientArena.Width, 20.0f});\
		vec4_t BarColor = ColorRed;

		// Dragging

		hash_t BarID = HashName(State, Title);
		Interact(State, Bar, BarID);
		if (IsActive(State, BarID))
		{
			Window->ClientBounds.Offset = (State->MouseCursor + State->ClickOffset);
			BarColor = ColorGreen;
		}

		//

		if ((State->Root == Window->Name) && State->Interact[0] &&
			Contains(Bar, State->MouseCursor))
		{
			
		}

		//

		Window->LayoutCursor = V2(Bar.Offset.x, Bar.y + Bar.Height);

		RenderRect(&State->Out, Stretch(ClientArena, 1.0f), ColorBlack); // Outline
		RenderRect(&State->Out, ClientArena, ColorWhite);

		RenderRect(&State->Out, Bar, BarColor);
		RenderString(&State->Out, Bar.Offset + V2(4.0f, 0.0f), Title);
	}
#endif

	return (true);
}

static void EndWindow(interface_t *State)
{
	window_t *Window = State->Wnd;
	interface_draw_command_t *Cmd = Window->DrawListBack;
	Cmd->Last = ((uint16_t)State->CommandBuffer.CmdCount - 1);

	BindWindow(State, 0);
}

static bool IsHovered(const interface_t *State, rect_t Bb)
{
	bool Result = Contains(Bb, State->MouseCursor) && (State->Wnd->Name == State->Root);
	return Result;
}

static bool IsHovered(const interface_t *State, hash_t ID)
{
	bool Result = (State->Hovered == ID);
	return Result;
}

static bool IsActive(const interface_t *State, hash_t ID)
{
	bool Result = (State->Active == ID);
	return Result;
}

static void BindWindow(interface_t *State, window_t *Window)
{
	if (Window)
	{
		Assert(!State->Wnd);
		State->Wnd = Window;	
	}
	else
	{
		Assert(State->Wnd);
		State->Wnd = Window;
	}
	
}

static hash_t HashName(const interface_t *State, const char *Name)
{
	hash_t Result = 0;

	MurmurHash3_x86_32(Name, (uint32_t)strlen(Name), State->Wnd->Name, &Result);

	return Result;
}

static bool Interact(interface_t *State, rect_t Bb, hash_t ID)
{
	bool Result = false;

	if ((State->Active == 0) && IsHovered(State, Bb))
	{
		State->Hovered = ID;

		if (State->Interact[0])
		{
			State->Active = ID;
			State->ClickOffset = Bb.Offset - State->MouseCursor;
		}
	}
	
	if (State->Active == ID)
	{
		if (!State->Interact[0])
		{
			if (IsHovered(State, Bb))
			{
				Result = true;
			}

			State->Active = 0;
		}
	}

	return Result;
}

static bool Button(interface_t *State, const char *Name)
{
	hash_t ID = HashName(State, Name);
	rect_t Bb = Rect(State->Wnd->LayoutCursor, V2(100.0f, 20.0f));

	bool Result = Interact(State, Bb, ID);

	vec4_t Color = ColorBlue;

	if (IsHovered(State, ID))
	{
		Color.x *= 0.9f;
		Color.y *= 0.9f;
		Color.z *= 0.9f;
	}

	if (State->Active == ID)
	{
		Color = ColorOrange;
	}

	RenderRect(&State->Out, Bb, Color);
	RenderString(&State->Out, Bb.Offset + V2(5.0f, 0.0f), Name);

	State->Wnd->LayoutCursor.y += 20.0f;

	return Result;
}
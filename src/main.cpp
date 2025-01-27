#include "Low/Client.h"
#include "Low/Platform/Platform_Win32.cpp"

#include "Game.hpp"
#include "Game.cpp"

static void SetupGame(game_state_t *State, const content_t *Content)
{
	memset(State, 0, sizeof(*State));
	State->Content = Content;

	CreateMap(&State->Map, 16, 16, &State->Tiles[0], 16);
	State->Players[0].Self = CreateMapObject(&State->Map, {8, 2})->Self;

	map_object_t *Player = GetMapObject(&State->Map, State->Players[0].Self);
	Assert(Player);

	Player->DebugColor = ColorBlue;

#define X 16
#define Y 16
	char Tiles[Y][X] = 
	{
		' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
		' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
		' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
		' ', '#', '#', '#', '#', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 'E', ' ', ' ',
		' ', '#', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
		' ', '#', ' ', 'E', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
		' ', '#', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
		' ', '#', '#', '#', '#', '#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
		' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', 'E', ' ', ' ',
		' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ',
		' ', 'E', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ',
		' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ',
		' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ',
		' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#', ' ', ' ', ' ', ' ', ' ',
		' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
		' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
	};

	uint8_t ValueTable[0x100] = "";
	ValueTable['#'] = 1;

	for (int32_t y = 0; y < Y; y++)
	{
		for (int32_t x = 0; x < X; x++)
		{
			char Ch = Tiles[y][x];
			switch (Ch)
			{
			case 'E':
				CreateMapObject(&State->Map, {x, y});
			default:
				SetTileValue(&State->Map, {x, y}, ValueTable[Ch]);
			}
		}
	}
#undef X
#undef Y

	// ...

	CreateCombatState(&State->Combat, &State->Map);

	JoinCombat(&State->Combat, State->Players[0].Self);

	for (int32_t Index = 1; Index < State->Map.Objects.Count; Index++)
	{
		map_object_t *Object = &State->Map.Objects[Index];
		JoinAsBot(&State->Combat, Object->Self);
	}
}

static void Monitor(game_state_t *State, float_t DeltaTime)
{
	float_t SampleFrequency = 1.5f;

	State->TimeSamples += DeltaTime;
	State->TimeSampleCount++;

	if (State->TimeSamples >= (1.0f / SampleFrequency))
	{
		State->TimeAvg = State->TimeSamples / (float_t)State->TimeSampleCount;
		State->TimeSampleCount = 0;
		State->TimeSamples = 0.0f;
	}

	int32_t FPS = (int32_t)(1.0f / State->TimeAvg);
	float_t Millisecond = State->TimeAvg * 1000.0f;

	char ClockFormat[32] = "";
	FormatClock(ClockFormat, Len(ClockFormat), State->Combat.ClockCycles);

	DebugPrint("%i FPS %0.2f MS | CLOCK (%s)", FPS, Millisecond, ClockFormat);


	DebugPrint("");
}

static void UpdateAndRender(game_state_t *State, float_t DeltaTime, render_output_t *Out, const controller_t *Controller)
{
	// 

	Monitor(State, DeltaTime);

	// ...

	combat_mode_state_t Mode = BeginCombatMode(&State->Combat, DeltaTime);

	// Combat (Interface)

	map_object_t *Player = GetMapObject(&State->Map, State->Players[0].Self);

	bool Issue = false;
	user_command_t Cmds[1] = {};

	if (Mode.AcceptsCommands && Player)
	{
		State->PollInputClock += DeltaTime;

		if (!Compare(Controller->DPad, {}) && (State->PollInputClock >= 0.15f))
		{
			State->PollInputClock = 0;

			memset(Cmds, 0, sizeof(Cmds));
			Cmds[0].User = Player->Self;
			Cmds[0].Dest = Player->OccupiedTile + Controller->DPad;
			Cmds[0].Interaction = Interaction_Transfer;

			Issue = true;
		}

		if (Controller->A.Down)
		{
			map_object_t *Hostile = GetMapObject(&State->Combat, FindAnyHostile(&State->Combat,  Player->Self));
			if (Hostile)
			{
				memset(Cmds, 0, sizeof(Cmds));
				Cmds[0].User = Player->Self;
				Cmds[0].Dest = Hostile->OccupiedTile;
				Cmds[0].Interaction = Interaction_Damage;

				Issue = true;
			}
		}
	}

	if (Issue)
	{
		IssueUserCommands(&State->Combat, Cmds, Len(Cmds));
	}

	// ...

	EndCombatMode(&State->Combat);

	// Animate/Render

	map_t *Map;

	Map = &State->Map;

	RenderRect(Out, {}, V2(256, 256), ColorDarkBlue); // Sky

	for (int32_t y = 0; y < Map->Y; y++)
	{
		for (int32_t x = 0; x < Map->X; x++)
		{
			point_t TileIndex = {x, y};
			const map_tile_t *Tile = GetTile(Map, TileIndex);

			rect_t Bounds = GetTileBounds(Map, TileIndex);
			DebugRect(Bounds.Offset, Bounds.Size, ColorWhite);

			if (Tile && Tile->Object)
			{
				RenderRect(Out, Bounds, ColorOrange);	
			}

			if (Tile && (Tile->ID > 0))
			{
				RenderRect(Out, Bounds, ColorWhite);
			}
		}
	}

	for (int32_t Index = 0; Index < Map->Objects.Count; Index++)
	{
		map_object_t *Object = &Map->Objects.Values[Index];

		rect_t ObjectBounds = Rect(Object->BitmapOffset, V2(16.0f));
		rect_t TileBounds = GetTileBounds(Map, Object->OccupiedTile);

		vec2_t Target = RectCenter(TileBounds) - RectExtends(ObjectBounds);

		// if (!(Object->TempOverrides & Override_Interpolator))
		{
			Object->BitmapOffset = Mix(Object->BitmapOffset, Target, DeltaTime * 15.0f);
			if (Length(Target - Object->BitmapOffset) < 1.0f)
				Object->BitmapOffset = Target;
		}

		RenderRect(Out, ObjectBounds, Object->DebugOverrideColor ? Object->ColorOverride : Object->DebugColor);

		Object->DebugOverrideColor = false;
		// Object->TempOverrides = 0;
	}
}

static vertex_t GlobalVertexBuffer[MB(10) / sizeof(vertex_t)];
static render_command_t GlobalCommandBuffer[8];

extern int main(void)
{
	client_t *Client = CreateClient("Project 3");

//	SetVideoMode(Client, 640, 420);
	SetVideoMode(Client, 1280, 720);

	assets_t *Assets = GetAssets(Client);
	MountDirectory(Assets, "Assets/");
	SetDefaultFont(Assets, LoadFontFromDirectory(Assets, "inconsolata.fnt"));

	content_t Content = {};
	LoadGameContent(Assets, &Content);

	game_state_t *Game = (game_state_t *)malloc(sizeof(*Game));
	SetupGame(Game, &Content);

	while (1)
	{
		// ...

		input_t Input = {};
		int32_t ExitCode = Host(Client, &Input);

		if (!ExitCode)
			break;

		// Begin

		transform_t Camera = {0, 0, 256, 256};
		rect_t View = Rect({}, Input.Viewport);
		View = MaintainAspectRatio(View, Camera.Size.x / Camera.Size.y);

		command_buffer_t Cmds = {};
		Setup(&Cmds, GlobalVertexBuffer, Len(GlobalVertexBuffer),
			GlobalCommandBuffer, Len(GlobalCommandBuffer));

		// ...

		// UpdateAndRender()

		BeginVisualDebugMode(V4(View), Camera);

		render_output_t Out = RenderTo(&Cmds, Assets);
		UpdateAndRender(Game, Input.DeltaTime, &Out, &Input.Controllers[0]);

		EndVisualDebugMode();

		// End

		graphics_device_t *Device = GetGraphicsDevice(Client);
		Dispatch(Device, &Cmds, V4(View), Camera);

		Present(Client);
	}
	return (0);
}
#include "Low/Client.h"
#include "Low/Platform_Win32.cpp"

#include "Low/Interface.h"
#include "Low/Interface.cpp"

#include "Game.hpp"
#include "Game.cpp"
#include "UserInterface.cpp"

#define COMPILE_EDITOR true

#if COMPILE_EDITOR
#include "Editor.hpp"
#include "Editor.cpp"
#endif

#if _DEBUG
#include "Debug.cpp"
#endif

static void SetupGameState(game_state_t *State, const content_t *Content)
{
	memset(State, 0, sizeof(*State));
	State->Content = Content;

	SetupMemory(&State->Memory, State->ReservedMemory, Len(State->ReservedMemory)); 

	CreateMap(&State->Map, 256, 256, &State->Tiles[0], 32);
	Assert((State->Map.X * State->Map.Y) <= Len(State->Tiles));

	State->Players[0].Self = CreateMapObject(&State->Map, {50, 50}, 0)->Self;

	map_object_t *Player = GetMapObject(&State->Map, State->Players[0].Self);
	Assert(Player);

	Player->DebugColor = ColorPink;

	State->Cameras[0] = CreateCamera({512, 512});
	State->Cameras[0].TrackedObject = State->Players[0].Self;

#define X 16
#define Y 16
	char Tiles[Y][X] = 
	{
		'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
		'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#',
		'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#',
		'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 'E', ' ', '#',
		'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#',
		'#', ' ', ' ', 'E', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#',
		'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#',
		' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#',
		' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 'E', ' ', '#',
		'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#',
		'#', 'E', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#',
		'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#',
		'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#',
		'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 'E', ' ', ' ', ' ', ' ', ' ', ' ', '#',
		'#', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '#',
		'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
	};

	uint8_t ValueTable[0x100] = "";
	ValueTable['#'] = 1;

	point_t Chunks[] = {{2, 1}, {5, 3}, {2, 3}};
	for (int32_t ChunkIndex = 0; ChunkIndex < Len(Chunks); ChunkIndex++)
	{
		point_t Chunk = Chunks[ChunkIndex];
		Chunk.x *= 16;
		Chunk.y *= 16;

		for (int32_t y = 0; y < Y; y++)
		{
			for (int32_t x = 0; x < X; x++)
			{
				point_t TileIndex = Point(x, y) + Chunk;

				char Ch = Tiles[y][x];
				switch (Ch)
				{
				case 'E': {}
					CreateMapObject(&State->Map, TileIndex, Object_Hostile, &Content->ObjectTypes[Object_ExampleEnemy]);
				default: {}
					SetTileValue(&State->Map, TileIndex, ValueTable[Ch]);
				}
			}
		}
	}
#undef X
#undef Y

	// ...

	InitializeCombatState(&State->Combat, &State->Map, &State->Memory);
	JoinCombat(&State->Combat, State->Players[0].Self);

	for (int32_t Index = 0; Index < State->Map.Objects.Count; Index++)
	{
		map_object_t *Object = &State->Map.Objects[Index];
		if (Object->Tags & Object_Hostile)
		{
			JoinAsBot(&State->Combat, Object->Self);
		}
	}
}

static void Update(game_state_t *State, float_t DeltaTime, graphics_device_t *Device,
	render_output_t *Out, const input_t *Input, const camera_t *DebugCamera, interface_t *GUI)
{
	// 

	DeltaTime = Clamp(DeltaTime, 0.0f, 1.0f / 30.0f);
	State->ElapsedTime += DeltaTime;
	State->ElapsedFrames++;
	
	// ...

	// Camera

	for (int32_t Index = 0; Index < Len(State->Cameras); Index++)
	{
		camera_t *Camera = &State->Cameras[Index];
		map_object_t *Tracked = GetMapObject(&State->Map, Camera->TrackedObject);
		if (Tracked)
		{
			vec2_t Target = Tracked->BitmapOffset;

			if (GetFocus(&State->Combat, &Target))
				Camera->Transform.Center = Target;
			else
				Camera->Transform.Center = Mix(Camera->Transform.Center, Target, 4.0f * DeltaTime);	

			//

			if (Length(Camera->Transform.Center - Target) < 0.5f)
				Camera->Transform.Center = Target;
		}
	}

	const camera_t *ActiveCamera = &State->Cameras[0];
	if (DebugCamera)
	{
		bounds_t Bounds = GetCameraBounds(GetCameraTransform(*ActiveCamera));
		DebugRect(Bounds.Min, Bounds.Max - Bounds.Min, ColorBlue);

		ActiveCamera = DebugCamera;
	}

	const transform_t Camera = GetCameraTransform(*ActiveCamera);
	//DebugPrint("Transform = {%.2f, %.2f} | Camera: Zoom =  %.2f", Camera.Size.x, Camera.Size.y, ActiveCamera->Zoom);
	rect_t View = Rect({}, Input->Viewport);
	View = MaintainAspectRatio(View, Camera.Size.x / Camera.Size.y);

	{
		State->NormalizedCursorCoords = (View.Offset - Input->MouseCursor) / View.Size;
		State->MappedCursorCoords = MapTo(Camera, (State->NormalizedCursorCoords * Camera.Size));
	}

	// ...

	BeginVisualDebugMode(V4(View), Camera);

	combat_mode_state_t Mode = BeginCombatMode(&State->Combat, DeltaTime);
	RunUserInterface(Mode, &State->Combat, &State->Map, GetMapObject(&State->Map, State->Players[0].Self), Input, GUI, DeltaTime);

	// ...

	// Render

	{
	transform_t CamTransform = Camera;
	BeginActiveRegion(&State->Region, State->ElapsedFrames, Stretch(GetCameraBounds(CamTransform), 64.0f));
	}	

	for (int32_t Index = 0; Index < State->Combat.Units.Count; Index++)
	{
		map_object_t *Obj = GetMapObject(&State->Map, State->Combat.Units[Index].Self);
		FetchMapObject(&State->Region, Obj);
	}

	RenderActiveRegion(Out, &State->Map, &State->Region, DeltaTime, State->ElapsedTime, State->Content);

	EndActiveRegion(&State->Region);

	//

	_Debug(State, Out, ActiveCamera);

	EndCombatMode(&State->Combat, Out);

	// Dispatch

	EndVisualDebugMode();

	Dispatch(Device, Out->Out, V4(View), Camera, Dispatch_Pixelated);
}

static vertex_t GlobalVertexBuffer[MB(32) / sizeof(vertex_t)];
static render_command_t GlobalCommandBuffer[1024];

extern int main(void)
{
	client_t *Client = CreateClient("Project 3");

//	SetVideoMode(Client, 640, 420);
	SetVideoMode(Client, 1600, 900);

	assets_t *Assets = GetAssets(Client);
	MountDirectory(Assets, "Assets/");
	SetDefaultFont(Assets, LoadFontFromDirectory(Assets, "inconsolata.fnt"));

	content_t Content = {};
	LoadGameContent(Assets, &Content);

	game_state_t *Game = (game_state_t *)malloc(sizeof(*Game));
	SetupGameState(Game, &Content);

#if COMPILE_EDITOR
	editor_t *EditorState = (editor_t *)malloc(sizeof(*EditorState));
	InitializeEditor(EditorState, Game);
#endif

	//

	interface_t *GUI = (interface_t *)malloc(sizeof(*GUI));
	InitializeUserInterface(GUI);

	while (1)
	{
		// ...

		input_t Input = {};
		int32_t ExitCode = Host(Client, &Input);

		Input.DeltaTime = Clamp(Input.DeltaTime, 0.0f, 1.0f);

		if (!ExitCode)
			break;

		// Begin

		command_buffer_t Cmds = {};
		Setup(&Cmds, GlobalVertexBuffer, Len(GlobalVertexBuffer),
			GlobalCommandBuffer, Len(GlobalCommandBuffer));

		BeginUserInterface(GUI, &Input, Assets);

		// ...

		render_output_t Out = RenderTo(&Cmds, Assets);
		
		const camera_t *EditorCamera = 0;
#if COMPILE_EDITOR
		EditorCamera = GetActiveEditorCamera(EditorState);
#endif		
		Update(Game, Input.DeltaTime, GetGraphicsDevice(Client), &Out, &Input, EditorCamera, GUI);

#if COMPILE_EDITOR
		Editor(EditorState, GUI, &Input, &Game->Cameras[0], Game->MappedCursorCoords, &Out);
#endif

		EndUserInterface(GUI);

		Present(Client);

		Render(GUI, GetGraphicsDevice(Client));

		Display(Client);
	}
	return (0);
}
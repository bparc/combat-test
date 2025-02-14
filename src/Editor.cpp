static const camera_t *GetActiveEditorCamera(editor_t *Editor)
{
	const camera_t *Result = Editor->CameraActive ? &Editor->Camera : NULL;
	return Result;
}

static void InitializeEditor(editor_t *Editor, game_state_t *State)
{
	memset(Editor, 0, sizeof(*Editor));
	Editor->Camera = CreateCamera({1920/3, 1080/3});
	Editor->Game = State;
}

static void MoveCamera(camera_t *Camera, const controller_t *Controller, float_t DeltaTime)
{	
	vec2_t MinBounds = {0.0f, 0.0f};
	vec2_t MaxBounds = {+9000.0f, +9000.0f};

	vec2_t Offset = {};

	float_t CameraSpeed = (1000.0f * 7.0f) + (3000.f * Camera->Zoom);
	float_t ZoomSpeed = 4.0f + (2.0f * Camera->Zoom);

	Camera->Velocity = Camera->Velocity + Controller->Analogs[0] * CameraSpeed * DeltaTime;
	Camera->Velocity = Camera->Velocity - (Camera->Velocity * 10.0f * DeltaTime);

	Camera->Transform.Center = Camera->Transform.Center + Camera->Velocity * DeltaTime;
	Camera->Transform.Center = Clamp(Camera->Transform.Center, MinBounds, MaxBounds);

	if (Controller->Triggers[0] > 0)
	{
		Camera->Zoom += ZoomSpeed * DeltaTime;
	}

	if (Controller->Triggers[1] > 0)
	{
		Camera->Zoom -= ZoomSpeed * DeltaTime; 
	}

	Camera->Zoom = Clamp(Camera->Zoom, -0.5f, 8.0f);
}

static void DebugMenu(editor_t *Editor, const input_t *Input, const camera_t *GameCamera)
{
	float_t SampleFrequency = 1.5f;

	Editor->TimeSamples += Input->DeltaTime;
	Editor->TimeSampleCount++;

	if (Editor->TimeSamples >= (1.0f / SampleFrequency))
	{
		Editor->TimeAvg = Editor->TimeSamples / (float_t)Editor->TimeSampleCount;
		Editor->TimeSampleCount = 0;
		Editor->TimeSamples = 0.0f;
	}

	int32_t FPS = (int32_t)(1.0f / Editor->TimeAvg);
	float_t Millisecond = Editor->TimeAvg * 1000.0f;

	DebugPrint("Editor.cpp %i FPS %0.2f MS | EDIT = %s (F1)", FPS, Millisecond,
		Editor->CameraActive ? "ON" : "OFF");

	if (Input->Controllers[0].DebugKeys[0].Down)
	{
		Editor->CameraActive = !Editor->CameraActive;
		Editor->Camera.Transform.Center = GameCamera->Transform.Center;
		Editor->Camera.Zoom = 1.0f;
	}

	if (Editor->CameraActive)
	{
		MoveCamera(&Editor->Camera, &Input->Controllers[0], Input->DeltaTime);
	}
}

static void Editor(editor_t *Editor, interface_t *UI, const input_t *Input, const camera_t *GameCamera, vec2_t GameCursor, render_output_t *Out)
{
	map_t *Map;

	Map = &Editor->Game->Map;

	DebugMenu(Editor, Input, GameCamera);
	
	if (!UI->InterceptInputs)
	{
		point_t Cell = CoordsToCell(Map, GameCursor);
//		DebugPrint("Cell = {%i, %i}", Cell.x, Cell.y);
		rect_t Bounds = GetTileBounds(Map, Cell);
		Bounds = Shrink(Bounds, Sin(Editor->Game->ElapsedTime));
	
		DebugRect(Bounds.Offset, Bounds.Size, RGBA(255, 0, 255));
		if (Input->MouseButtons[0].State)
		{
			SetTileValue(Map, Cell, 1);
		}
		if (Input->MouseButtons[1].State)
		{
			SetTileValue(Map, Cell, 0);
		}
	}

	//

#if 0
	BeginWindow(UI, 0, {650.0f, 25.0f}, {256.0f, 256.0f}, "Example Window");
	Button(UI, "Button 1");
	EndWindow(UI);
	BeginWindow(UI, 1, {450.0f, 40.0f}, {256.0f, 512.0f}, "Window 2");
	Button(UI, "Button 2");
	Button(UI, "Button 3");
	Button(UI, "Button 4");
	Button(UI, "Button 5");
	EndWindow(UI);
	BeginWindow(UI, 0, {650.0f, 25.0f}, {256.0f, 256.0f}, "Example Window");
	Button(UI, "Button 10");
	Button(UI, "Button 11");
	Button(UI, "Button 12");
	Button(UI, "Button 13");
	EndWindow(UI);
	BeginWindow(UI, 2, {140.0f, 40.0f}, {128.0f, 128.0f}, "Test 1");
	EndWindow(UI);
	BeginWindow(UI, 3, {140.0f, 40.0f}, {128.0f, 128.0f}, "Test 2");
	EndWindow(UI);
	BeginWindow(UI, 4, {140.0f, 40.0f}, {128.0f, 128.0f}, "Test 3");
	EndWindow(UI);
#endif
}
struct editor_t
{
	game_state_t *Game;

	bool CameraActive;
	camera_t Camera;

	int32_t TimeSampleCount;
	float_t TimeSamples;
	float_t TimeAvg;
};

static void InitializeEditor(editor_t *Editor, game_state_t *State);
static void Editor(editor_t *Editor, const input_t *Input, const camera_t *GameCamera, vec2_t GameCursor, render_output_t *Out);

static const camera_t *GetActiveEditorCamera(editor_t *Editor);
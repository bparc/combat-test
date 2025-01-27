// ...

typedef uint32_t object_t;

// ...

struct map_tile_t
{
	object_t Object; // Occupier
	uint8_t ID;
};

struct map_object_t
{
	uint16_t HP;

	object_t Self;
	point_t OccupiedTile; // NOTE: Has to be set via Translate()!!!

	// Rendering

	vec2_t BitmapOffset;

	//

	bool DebugOverrideColor;
	vec4_t ColorOverride;

	vec4_t DebugColor;

	// ...

	// ...
};

struct map_t
{
	vec2_t TileSize;
	int32_t X, Y;
	map_tile_t *Tiles;

	uint16_t ObjectLookasideTable[UINT16_MAX]; // Maps ID (object_t) -> Objects[Index]
	array_t <map_object_t, 1024> Objects;
	uint64_t TotalObjectCount; // The number of objects that ever existed
};

static void CreateMap(map_t *Map, int32_t X, int32_t Y, map_tile_t *Tiles, int32_t TileSize);

static bool CheckBounds(const map_t *Map, point_t Offset);
static const map_tile_t *GetTile(const map_t *Map, point_t Offset);
static map_tile_t *GetTile(map_t *Map, point_t Offset);
static void SetTileValue(map_t *Map, point_t Offset, int32_t Value);

static rect_t GetTileBounds(const map_t *Map, point_t Offset);
static vec2_t GetTileCenter(const map_t *Map, point_t Offset);

static map_object_t *CreateMapObject(map_t *Map, point_t At);
static map_object_t *GetMapObject(map_t *Map, object_t Object);
static map_object_t *GetMapObject(map_t *Map, point_t Offset);

static bool Translate(map_t *Map, map_object_t *Object, point_t At);
static bool Translate(map_t *Map, object_t Object, point_t At);

//static map_object_t *GetMapObject(map_t *Map, point_t Offset);

// ...

// combat_state_t

struct active_unit_t
{
	object_t Self;
	uint8_t Alliance; // NOTE(): Used to discriminate between allied and hostile targets. Units whose "alliance" is equal are considered to be in the same team and so on.
	uint8_t Bot; // Acts as a bot if TRUE
};

enum basic_interaction_mode_t
{
	Interaction_None,
	Interaction_Transfer,
	Interaction_Damage,
	//Interaction_Route,
};

struct action_t
{
	basic_interaction_mode_t Mode;

	float_t Clock;
	point_t Cell;
	object_t Target; // Aquired from action_t::Cell at the moment of querying an action.
	object_t User;
	
	uint8_t Commited;
	uint8_t Fence; // (bool) Blocks all following actions until this one is commited.

	// ...

	int32_t DebugSequenceIndex; // An unique identifier assigned by the initial scheduling process.
	uint8_t DebugChain; // When triggering actions sequentially, this variable is incremented by one and passed down to the following action.
};

struct user_command_t
{
	object_t User;

	basic_interaction_mode_t Interaction;
	point_t Dest;

	//point_t RelativeOffset; // Translation
};

struct combat_state_t //
{
	float_t ElapsedTime;
	float_t DeltaTime;

	map_t *Map;
	array_t <active_unit_t, 256> Units;
	array_t <user_command_t, 1> InputBuffer;
	
	#define COMBAT_CLOCK_FREQUENCY 10 // The smallest unit (fraction) of time we can differentiate. E.g., a frequency of "10" means that each "cycle" represents a 1/10 of a second.
	uint64_t ClockCycles;

	array_t <action_t, 1024>Actions;

	bool RejectIncomingCommands;
	bool Execute;
};

static void CreateCombatState(combat_state_t *State, map_t *Map);

struct combat_mode_state_t
{
	bool AcceptsCommands; // If TRUE, the scheduler is waiting for commands (see, IssueUserCommands())
};

static combat_mode_state_t BeginCombatMode(combat_state_t *State, float_t DeltaTime);
static void EndCombatMode(combat_state_t *State);

static void JoinCombat(combat_state_t *State, object_t Object, bool Bot = false);
static void JoinAsBot(combat_state_t *State, object_t Object);

static void IssueUserCommands(combat_state_t *State, user_command_t *Commands, int32_t Count);

static void QueryAsynchronousAction(combat_state_t *State, object_t User, basic_interaction_mode_t Mode, point_t Target, bool Blocking = false);
//static void QueryAsynchronousAction(combat_state_t *State, object_t User, object_t Target, bool Blocking = false);

static active_unit_t *FindCombatUnit(combat_state_t *State, object_t Object);
static active_unit_t *FindAnyHostile(combat_state_t *State, object_t Object);
static map_object_t *GetMapObject(combat_state_t *State, active_unit_t *Unit);

static void FormatClock(char *Buffer, int32_t Length, uint64_t Cycles);
// static float_t ClockCyclesToSeconds(const combat_state_t *State);

// ...

// game_state_t

struct content_t
{

};

static void LoadGameContent(assets_t *Assets, content_t *Content)
{

}

// player_t

struct player_t
{
	object_t Self;
};

// ...

struct game_state_t
{
	const content_t *Content;

	combat_state_t Combat;
	map_t Map;

	float_t PollInputClock;

	player_t Players[1];

	map_tile_t Tiles[1024 * 1024]; // Memmory

	int32_t TimeSampleCount;
	float_t TimeSamples;
	float_t TimeAvg;
};

static void SetupGame(game_state_t *State, const content_t *Content);
static void UpdateAndRender(game_state_t *State, float_t DeltaTime, render_output_t *Out, const controller_t *Controller);

// ...
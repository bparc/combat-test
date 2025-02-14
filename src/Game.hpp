// ...

typedef uint32_t object_t;

// ...

//

struct camera_t
{
	transform_t Transform;
	vec2_t Velocity;

	float_t Zoom; // 0 == No zoom
	object_t TrackedObject;
};

static transform_t GetCameraTransform(camera_t Camera);
static camera_t CreateCamera(vec2_t ViewableArenaSize);

//

struct map_tile_t
{
	object_t Object; // Occupier
	uint8_t ID;
};

bool IsEmpty(const map_tile_t *Tile);

enum object_tag_t
{
	Object_Hostile = (1 << 0),
};

struct object_behaviour_t
{
	//uint16_t ContactDamage;
};

struct map_object_t
{
	const object_behaviour_t *Behaviour;
	uint8_t Tags;

	float_t DamageFlickerTime; // NOTE: Object will flicker if > 0.0.
	uint16_t HP;

	object_t Self;
	point_t OccupiedTile; // NOTE: Has to be set via Translate()!!!

	uint64_t LastFrameFetched;

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

	object_behaviour_t None;
};

static void CreateMap(map_t *Map, int32_t X, int32_t Y, map_tile_t *Tiles, int32_t TileSize);

static bool CheckBounds(const map_t *Map, point_t Offset);
static const map_tile_t *GetTile(const map_t *Map, point_t Offset);
static 		 map_tile_t *GetTile(map_t *Map, point_t Offset);
static void SetTileValue(map_t *Map, point_t Offset, int32_t Value);

static rect_t GetTileBounds(const map_t *Map, point_t Offset);
static vec2_t GetTileCenter(const map_t *Map, point_t Offset);
static point_t CoordsToCell(const map_t *Map, vec2_t Coords);

static map_object_t *CreateMapObject(map_t *Map, point_t At, uint8_t Tags, const object_behaviour_t *Behaviour = NULL);
static map_object_t *GetMapObject(map_t *Map, object_t Object);
static map_object_t *GetMapObject(map_t *Map, point_t Offset);
//static map_object_t *GetMapObject(map_t *Map, point_t Offset);

static bool Translate(map_t *Map, map_object_t *Object, point_t At);
static bool Translate(map_t *Map, object_t Object, point_t At);
static bool IsPassable(const map_t *Map, point_t At);

struct min_queue_node_t
{
	point_t Offset;
	uint16_t Priority;
};

struct min_queue_t
{
	// NOTE: Stores a pointer to the person who currently operates on the queue.
	// Just to make sure that we are always passing the same pointers around during partial updates.
	const void *Lock;

	int32_t Count;
	min_queue_node_t Nodes[1024 * 1024];
};

static void Flush(min_queue_t *Queue);
static void InsertMin(min_queue_t *Queue, point_t Offset, uint16_t Priority);
static min_queue_node_t ExtractMin(min_queue_t *Queue);

struct distance_t
{
	#define RANGE_MAP_INFINITY UINT16_MAX
	uint16_t Distance;
};

struct distance_map_t
{
	bool Completed; // NOTE: If FALSE, the data is NOT valid!

	int32_t X;
	int32_t Y;
	distance_t *Nodes;
	point_t Origin;

	// Statistics
	double_t DebugBeginTimestamp;
	double_t IntegrateTimeAvg;
	int32_t IntegrateCount;
	int32_t ProcessedNodes;

	// ...
};

static void InitializeDistanceMap(distance_map_t *Map, point_t Min, point_t Max, memory_t *Memory);

static void ClearDistanceMap(distance_map_t *Map);

static void IntegrateRange(min_queue_t *Queue, distance_map_t *Range, const map_t *Obstacles, point_t Origin);
static void ProcessQueue(distance_map_t *Map, min_queue_t *Queue, const map_t *Obstacles, int32_t NodeLimit = INT32_MAX); // BeginPartialUpdate()

static bool CheckBounds(const distance_map_t *Map, point_t Offset);
static distance_t* GetDistance(distance_map_t *Map, point_t Offset);
static int32_t	   GetDistanceValue(distance_map_t *Map, point_t Offset);

static bool DebugRouteLocally(const distance_map_t *Map, point_t At, point_t *ClosestPoint, const map_t *Obstacles);

// ...

// combat_state_t

struct active_unit_t
{
	object_t Self;
	uint8_t Alliance; // Used to discriminate between allied and hostile targets. Units whose "alliance" is equal are considered to be in the same team and so on.
	uint8_t Bot; // Acts as a bot if TRUE
};

enum basic_interaction_mode_t
{
	Interaction_None,
	Interaction_Transfer,
	Interaction_Damage,
	Interaction_Seek, // NOTE: Seeks the currently designated "global" target. "Dest" and "Target" fields are ignored.
	//Interaction_Route,
};

enum animation_flags_t
{
	Anim_Ranged = (1 << 0),
	Anim_Bitmap = (1 << 1),
};

struct animation_t
{
	uint16_t Flags;
};

struct action_t
{
	basic_interaction_mode_t Mode;
	point_t Dest;
	object_t Target; // Aquired automatically from action_t::Dest at the moment of querying an action.
 
	float_t Time;

	object_t User;
	
	uint8_t Commited;
	uint8_t DebugFence; // (bool) Blocks all following actions until this one is commited.

	// ...

	int32_t DebugSequenceIndex; // An unique identifier assigned by the initial scheduling process.
	//uint8_t DebugChain; // When triggering actions sequentially, this variable is incremented by one and passed down to the following action.

	const animation_t *Anim;
};

struct user_command_t
{
	object_t User;

	basic_interaction_mode_t Interaction;
	point_t Dest;


	//point_t RelativeOffset; // Translation
};

static user_command_t CreateUserCommand(object_t User, basic_interaction_mode_t Mode, point_t Dest);

struct combat_state_t //
{
	float_t ElapsedTime;
	float_t DeltaTime;

	map_t *Map;
	array_t <active_unit_t, 1024> Units;
	array_t <user_command_t, 1> InputBuffer;

	array_t <action_t, 1024 * 4>Actions;

	bool RejectIncomingCommands;
	bool Execute;

	// ...

#define MAX_NODE_COUNT_PER_FRAME 4096 * 2
//#define LOCAL_RANGE 8
	distance_map_t GlobalRange;
	min_queue_t FloodQueue;

	int32_t DebugLowestSeekPriority;

	bool Focus;
	vec2_t FocusPosition;
};

static void InitializeCombatState(combat_state_t *State, map_t *Map, memory_t *Memory);

struct combat_mode_state_t
{
	bool WaitingForCommands; // If TRUE, the scheduler is waiting for commands (see, IssueUserCommands()).
};

static combat_mode_state_t BeginCombatMode(combat_state_t *State, float_t DeltaTime);
static void EndCombatMode(combat_state_t *State, render_output_t *Out);

static void JoinCombat(combat_state_t *State, object_t Object, bool Bot = false);
static void JoinAsBot(combat_state_t *State, object_t Object);

static void IssueUserCommands(combat_state_t *State, user_command_t *Commands, int32_t Count);

static active_unit_t *FindCombatUnit(combat_state_t *State, object_t Object);
static active_unit_t *FindAnyHostile(combat_state_t *State, object_t Object);
static map_object_t *GetMapObject(combat_state_t *State, active_unit_t *Unit);

static bool GetFocus(combat_state_t *State, vec2_t *Focus);
// static float_t ClockCyclesToSeconds(const combat_state_t *State);

// Internal:
static void QueryAsynchronousAction(combat_state_t *State, object_t User, basic_interaction_mode_t Mode, point_t Target, bool Blocking = false);
static void CombatMode(combat_state_t *State, render_output_t *Out);

//static void QueryAsynchronousAction(combat_state_t *State, object_t User, object_t Target, bool Blocking = false);

// ...

// game_state_t

enum object_type_t
{
	Object_None = 0,
	Object_ExampleEnemy,
	Object_Count,
};

struct content_t
{
	bitmap_t Player[4];
	bitmap_t Slime;

	bitmap_t Tiles[4];

	object_behaviour_t ObjectTypes[Object_Count];
};

static void LoadGameContent(assets_t *Assets, content_t *Content)
{
	Content->Player[0] = LoadBitmapFromDirectory(Assets, "PlayerWalk_0.tga");
	Content->Player[1] = LoadBitmapFromDirectory(Assets, "PlayerWalk_1.tga");
	Content->Player[2] = LoadBitmapFromDirectory(Assets, "PlayerWalk_2.tga");
	Content->Player[3] = LoadBitmapFromDirectory(Assets, "PlayerWalk_3.tga");
	Content->Slime = LoadBitmapFromDirectory(Assets, "slime_small_front.tga");

	Content->Tiles[0] = LoadBitmapFromDirectory(Assets, "32x16.tga");

	//
	
	//object_behaviour_t *ExampleEnemy = &Content->ObjectTypes[Object_ExampleEnemy];
	//ExampleEnemy->ActionCount = 2;
}

// player_t

struct player_t
{
	object_t Self;
};

// ...

struct active_region_t
{
	uint64_t CurrentFrame;
	array_t<object_t, 1024*4> VisibleObjects;
	bounds_t Bounds;
};

static void BeginActiveRegion(active_region_t *Region, uint64_t Frame, bounds_t Bounds);
static void EndActiveRegion(active_region_t *Region);
static void FetchMapObject(active_region_t *Region, map_object_t *Object);

struct game_state_t
{
	// Data

	const content_t *Content;

	uint8_t ReservedMemory[MB(10)];
	memory_t Memory;

	 // Systems

	combat_state_t Combat;

	// World

	active_region_t Region;
	map_t Map;

	camera_t Cameras[1];
	player_t Players[1];

	map_tile_t Tiles[(256 * 256) + 32]; // Memory

	// ...

	float_t ElapsedTime;
	uint64_t ElapsedFrames;

	vec2_t NormalizedCursorCoords;
	vec2_t MappedCursorCoords;
};

static vec2_t GetActiveCameraCenter(const game_state_t *State);
static void SetupGameState(game_state_t *State, const content_t *Content);
static void Update(game_state_t *State, float_t DeltaTime, graphics_device_t *Device, render_output_t *Out, const input_t *Input, const camera_t *DebugCamera = NULL);

static vec2_t Project(vec2_t Point);
static vec2_t Unproject(vec2_t Point);

static void RenderActiveRegion(render_output_t *Out, map_t *Map, active_region_t *Region, float_t DeltaTime, float_t ElapsedTime, const content_t *Content);
// camera_t

static transform_t GetCameraTransform(camera_t Camera)
{
	transform_t Result = Camera.Transform;
	Result.Center = Result.Center;
	Result.Size = Floor(Result.Size * (1.0f + Camera.Zoom));
	return Result;
}

static camera_t CreateCamera(vec2_t ViewableArenaSize)
{
	camera_t Result = {};
	Result.Transform = Transform({}, ViewableArenaSize);
	return Result;
}

//

// map_t

bool IsEmpty(const map_tile_t *Tile)
{
	bool Result = Tile ? (Tile->ID == 0) : 0;
	return Result;
}

static void CreateMap(map_t *Map, int32_t X, int32_t Y, map_tile_t *Tiles, int32_t TileSize)
{
	memset(Map, 0, sizeof(*Map));

	Map->TileSize = V2((float_t)TileSize);
	Map->Tiles = Tiles;
	Map->X = X;
	Map->Y = Y;
}

static bool CheckBounds(const map_t *Map, point_t Offset)
{
	bool Result = (Offset.x >= 0) && (Offset.y >= 0) &&
		(Offset.x < Map->X) && (Offset.y < Map->Y);
	return Result;
}

static const map_tile_t *GetTile(const map_t *Map, point_t Offset)
{
	const map_tile_t *Result = 0;
	if (CheckBounds(Map, Offset))
	{
		Result = &Map->Tiles[Offset.y * Map->X + Offset.x];
	}
	return Result;
}

static map_tile_t *GetTile(map_t *Map, point_t Offset)
{
	map_tile_t *Result = 0;
	if (CheckBounds(Map, Offset))
	{
		Result = &Map->Tiles[Offset.y * Map->X + Offset.x];
	}
	return Result;
}

static void SetTileValue(map_t *Map, point_t Offset, int32_t Value)
{
	map_tile_t *Tile = GetTile(Map, Offset);
	if (Tile && !Tile->Object)
	{
		Tile->ID = (uint8_t)Value;
	}
}

static rect_t GetTileBounds(const map_t *Map, point_t Offset)
{
	rect_t Result = {};
	Result.Size = Map->TileSize;
	Result.Offset = V2(Offset) * Result.Size;
	return Result;
}

static vec2_t GetTileCenter(const map_t *Map, point_t Offset)
{
	vec2_t Result = RectCenter(GetTileBounds(Map, Offset));
	return Result;
}

static point_t CoordsToCell(const map_t *Map, vec2_t Coords)
{
	point_t Result = {};
	Result.x = (int32_t)floorf(Coords.x / Map->TileSize.x);
	Result.y = (int32_t)floorf(Coords.y / Map->TileSize.y);
	return Result;
}

static map_object_t *CreateMapObject(map_t *Map, point_t At, uint8_t Tags, const object_behaviour_t *Behaviour)
{
	map_object_t *Result = 0;;

	map_tile_t *Tile = GetTile(Map, At);
	if (Tile && !Tile->Object)
	{
		Result = Map->Objects.Push();
	}

	if (Result)
	{
		Assert((Map->TotalObjectCount <= (UINT16_MAX - 1)));
		Result->Self = (object_t)(++Map->TotalObjectCount);
		Result->OccupiedTile = At;
		Result->DebugColor = ColorRed;
		Result->Behaviour = Behaviour ? Behaviour : &Map->None;
		Result->Tags = Tags;

		Assert(Result->Self < Len(Map->ObjectLookasideTable));
		Map->ObjectLookasideTable[Result->Self] = (uint16_t)Map->Objects.Count;

		Tile->Object = Result->Self;
	}
	return Result;
}

static map_object_t *GetMapObject(map_t *Map, object_t Object)
{
	map_object_t *Result = 0;

	uint16_t Reference = Map->ObjectLookasideTable[Object];
	if (Reference > 0)
	{
		Result = &Map->Objects[Reference - 1];
	}

	return Result;
}

static map_object_t *GetMapObject(map_t *Map, point_t Offset)
{
	map_object_t *Result = 0;
	map_tile_t *Tile = GetTile(Map, Offset);
	if (Tile)
	{
		Result = GetMapObject(Map, Tile->Object);
	}
	return Result;
}

static bool Translate(map_t *Map, map_object_t *Object, point_t At)
{
	bool Result = false;

	map_tile_t *Prev, *Next;

	Prev = GetTile(Map, Object->OccupiedTile);
	Next = GetTile(Map, At);

	if (Prev && (Next && (!Next->Object && !Next->ID)))
	{
		Object->OccupiedTile = At;

		Prev->Object = 0;
		Next->Object = Object->Self;

		Result = true;
	}

	return Result;
}

static bool Translate(map_t *Map, object_t Object, point_t At)
{
	bool Result = false;

	map_object_t *Reference = GetMapObject(Map, Object);
	if (Reference)
		Result = Translate(Map, Reference, At);
	return Result;
}

static bool IsPassable(const map_t *Map, point_t At)
{
	bool Result = 0;

	const map_tile_t *Tile = GetTile(Map, At);
	Result = Tile && (IsEmpty(Tile) && (Tile->Object == 0));

	return Result;
}
//
static void Flush(min_queue_t *Queue)
{
	Queue->Count = 0;
}

static void InsertMin(min_queue_t *Queue, point_t Offset, uint16_t Priority)
{
	if (Queue->Count < Len(Queue->Nodes))
	{
		min_queue_node_t *Node = &Queue->Nodes[Queue->Count++];
		Node->Priority = Priority;
		Node->Offset = Offset;
	}
}

static min_queue_node_t ExtractMin(min_queue_t *Queue)
{
	Assert(Queue->Count > 0);

	min_queue_node_t Result = {};

	int32_t BestIndex = 0, Min = (UINT16_MAX - 1);
	for (int32_t Index = 0; Index < Queue->Count; Index++)
	{
		const min_queue_node_t *Match = &Queue->Nodes[Index];
		if (Match->Priority < Min)
		{
			BestIndex = Index;
			Min = Match->Priority;
		}
	}

	Result = Queue->Nodes[BestIndex];
	Queue->Nodes[BestIndex] = Queue->Nodes[--Queue->Count];

	return Result;
}
//

static void InitializeDistanceMap(distance_map_t *Map, point_t Min, point_t Max, memory_t *Memory)
{
	memset(Map, 0, sizeof(*Map));
	Map->X = Max.x - Min.x;
	Map->Y = Max.y - Min.y;
	Map->Nodes = PushArray(Memory, distance_t, Map->X * Map->Y);

	Assert(Map->Nodes);

	ClearDistanceMap(Map);
	Map->Completed = true;
}

static void ClearDistanceMap(distance_map_t *Map)
{
	const int32_t NodeCount = Map->X * Map->Y;
	for (int32_t Index = 0; Index < NodeCount; Index++)
	{
		Map->Nodes[Index].Distance = RANGE_MAP_INFINITY;
	}
}

static void Flood(const map_t *Obstacles, min_queue_t *Queue, distance_map_t *Map, point_t At, uint16_t Distance, point_t Delta)
{
	point_t Offset = At + Delta;
	distance_t *Adjacent = GetDistance(Map, Offset);
	if ((Adjacent && (Adjacent->Distance == RANGE_MAP_INFINITY)) &&
		IsEmpty(GetTile(Obstacles, Offset)))
	{
		Adjacent->Distance = Distance;
		InsertMin(Queue, Offset, (Distance + 1));
	}
}

static void IntegrateRange(min_queue_t *Queue, distance_map_t *Map, const map_t *Obstacles, point_t Origin)
{
	Map->Completed = false;
	Map->Origin = Origin;
	
	ClearDistanceMap(Map);

	Assert(Map->Nodes);
	Assert((Obstacles->X * Obstacles->Y) < Len(Queue->Nodes));

	// DebugLog("Origin = {%i, %i}", Origin.x, Origin.y);

	//

	Flush(Queue);

	//

	Flood(Obstacles, Queue, Map, Origin, 0, {});

	Map->DebugBeginTimestamp = DebugGetTime();
	Map->IntegrateCount = 0;
	Map->IntegrateTimeAvg = 0;
	Map->ProcessedNodes = 0;

	Assert(!Queue->Lock);
	Queue->Lock = (const void *)Map;
}

static void ProcessQueue(distance_map_t *Map, min_queue_t *Queue, const map_t *Obstacles, int32_t NodeLimit)
{
	Assert(Queue->Lock == Map);

	int32_t IntegratedNodes = 0;
	double_t Time = DebugGetTime();

	while (Queue->Count && (IntegratedNodes < NodeLimit))
	{
		min_queue_node_t Min = ExtractMin(Queue);
		Flood(Obstacles, Queue, Map, Min.Offset, Min.Priority, {+1, +0});
		Flood(Obstacles, Queue, Map, Min.Offset, Min.Priority, {-1, +0});
		Flood(Obstacles, Queue, Map, Min.Offset, Min.Priority, {+0, +1});
		Flood(Obstacles, Queue, Map, Min.Offset, Min.Priority, {+0, -1});
		IntegratedNodes++;
	}

	Map->ProcessedNodes += IntegratedNodes;
	Map->IntegrateCount++;
	Map->IntegrateTimeAvg += (DebugGetTime() - Time);

	//DebugPrint("%.2f", (Map->IntegrateTimeAvg / Map->IntegrateCount) * 1000.0f);
//	DebugLog("IntegratedNodes = %i", IntegratedNodes);

	if (Queue->Count == 0)
	{
		Map->Completed = true;

		float_t TimeSpent = (float_t)(DebugGetTime() - Map->DebugBeginTimestamp);
		//DebugLog("Completed : Delay = %fs (%ims) (%i frames) AVG = %.2fms", TimeSpent, (int32_t)(TimeSpent * 1000.0f), Map->IntegrateCount,
		//	(Map->IntegrateTimeAvg / Map->IntegrateCount) * 1000.0f);

		Queue->Lock = 0;
	}
}

static bool CheckBounds(const distance_map_t *Map, point_t Offset)
{
	bool Result = (Offset.x >= 0) && (Offset.y >= 0) &&
		(Offset.x < Map->X) && (Offset.y < Map->Y);
	return Result;
}

static distance_t *GetDistance(distance_map_t *Map, point_t Offset)
{
	distance_t *Result = 0;
	if (CheckBounds(Map, Offset))
	{
		Result = &Map->Nodes[Offset.y * Map->X + Offset.x];
	}
	return Result;
}

static int32_t GetDistanceValue(distance_map_t *Map, point_t Offset)
{
	distance_t *Distance = GetDistance(Map, Offset);
	return (Distance ? Distance->Distance : RANGE_MAP_INFINITY);
}

static bool DebugRouteLocally(distance_map_t *Map, point_t At, point_t *ClosestPoint, const map_t *Obstacles)
{
	bool Result = false;

	if (Map->Completed)
	{
		distance_t *CurrentDist = GetDistance(Map, At);
		if (CurrentDist)
		{
			point_t Adjacent[8] = {{+1, +0}, {-1, +0}, {+0, +1}, {+0, -1},
				{+1, -1}, {+1, +1}, {-1, -1}, {-1, +1}};

			int32_t MinTentative = CurrentDist->Distance;
			float_t MinManhattan = Distance(Map->Origin, At);

			for (int32_t Index = 0; Index < Len(Adjacent); Index++)
			{
				point_t AdjCell = At + Adjacent[Index];
				int32_t Tentative = GetDistanceValue(Map, AdjCell);
				float_t Manhattan = Distance(Map->Origin, AdjCell);	
				
				if (IsPassable(Obstacles, AdjCell) &&
					((Tentative < MinTentative) ||
					((Tentative <= MinTentative) && (Manhattan < MinManhattan))) &&
					(Tentative != RANGE_MAP_INFINITY))
				{
					MinTentative = Tentative;
					MinManhattan = Manhattan;
					*ClosestPoint = AdjCell;
					Result = true;
				}
			}
		}
	}

	return Result;
}

// ...

// combat_state_t

static user_command_t CreateUserCommand(object_t User, basic_interaction_mode_t Mode, point_t Dest)
{
	user_command_t Result = {};
	Result.User = User;
	Result.Interaction = Mode;
	Result.Dest = Dest;
	return Result;
}

//

static void JoinCombat(combat_state_t *State, object_t Object, bool Bot)
{
	if (!FindCombatUnit(State, Object))
	{
		active_unit_t *Unit = State->Units.Push();
		if (Unit)
		{
			Unit->Self = Object;
			Unit->Bot = Bot;
			Unit->Alliance = Bot ? 1 : 0;
		}
	}
}

static void JoinAsBot(combat_state_t *State, object_t Object)
{
	JoinCombat(State, Object, true);
}

static void IssueUserCommands(combat_state_t *State, user_command_t *Commands, int32_t Count)
{	
	if (!State->RejectIncomingCommands)
	{
		State->Execute = true;

		for (int32_t Index = 0; Index < Count; Index++)
		{
			State->InputBuffer.Push(Commands[Index]);
		}
	}
}

static void QueryAsynchronousAction(combat_state_t *State, object_t User, basic_interaction_mode_t Mode, point_t Target, bool Blocking)
{
	map_object_t *Object;

	action_t *Action = State->Actions.Push();
	memset(Action, 0, sizeof(*Action));

	Action->DebugFence = Blocking;
	Action->Mode = Mode;
	Action->User = User;
	Action->Dest = Target;
	Action->DebugSequenceIndex = State->Actions.Count;

	Object = GetMapObject(State->Map, Target);
	Action->Target = Object ? Object->Self : 0;

	if (Mode == Interaction_Damage)
	{
		static animation_t DebugAnim = {};
		DebugAnim.Flags |= Anim_Ranged;

		Action->Anim = &DebugAnim;
	}
}

static active_unit_t *FindCombatUnit(combat_state_t *State, object_t Object)
{
	// TODO(): Lookaside buffer for translating objects to active_units?

	active_unit_t *Result = 0;
	for (int32_t Index = 0; Index < State->Units.Count; Index++)
	{
		active_unit_t *Match = &State->Units[Index];
		if (Match->Self == Object)
		{
			Result = Match;
			break;
		}
	}
	return Result;
}

static active_unit_t *FindAnyHostile(combat_state_t *State, object_t Object)
{
	active_unit_t *Result = 0;

	active_unit_t *Unit = FindCombatUnit(State, Object);
	if (Unit)
	{
		for (int32_t Index = 0; Index < State->Units.Count; Index++)
		{
			active_unit_t *Match = &State->Units[Index];
			if (Match->Alliance != Unit->Alliance)
			{
				Result = Match;
				break;
			}
		}
	}
	return Result;
}

static map_object_t *GetMapObject(combat_state_t *State, active_unit_t *Unit)
{
	map_object_t *Result = 0;
	if (Unit)
	{
		Result = GetMapObject(State->Map, Unit->Self);
	}
	return Result;
}

static bool GetFocus(combat_state_t *State, vec2_t *Focus)
{
	if (State->Focus)
		*Focus = State->FocusPosition;
	return State->Focus;
}

static void InitializeCombatState(combat_state_t *State, map_t *Map, memory_t *Memory)
{
	memset(State, 0, sizeof(*State));
	State->Map = Map;
	InitializeDistanceMap(&State->GlobalRange, {}, {Map->X, Map->Y}, Memory);
}

static combat_mode_state_t BeginCombatMode(combat_state_t *State, float_t DeltaTime)
{
	combat_mode_state_t Result = {};
	Result.WaitingForCommands = (State->RejectIncomingCommands == false);

	Assert(State->Map);

	State->DeltaTime = DeltaTime;
	State->ElapsedTime += State->DeltaTime;

	if (!State->GlobalRange.Completed && State->FloodQueue.Lock)
	{
		ProcessQueue(&State->GlobalRange, &State->FloodQueue, State->Map, MAX_NODE_COUNT_PER_FRAME);
	}

	return Result;
}

static void EndCombatMode(combat_state_t *State, render_output_t *Out)
{
	State->Focus = false;

	CombatMode(State, Out);
}

static void CombatMode(combat_state_t *State, render_output_t *Out)
{
	// Schedule

	if ((State->Actions.Count == 0) && State->Execute) // Accept commands only if there are no ongoing actions.
	{
		while ((State->InputBuffer.Count > 0))
		{
			// Poll

			const user_command_t *Cmd = State->InputBuffer.Pop();
			//DebugLog("(%i)->{%i,%i}", (int32_t)Cmd->User, Cmd->RelativeOffset.x, Cmd->RelativeOffset.y);

			int32_t ConsumedTime = 10;

			// User

			map_object_t *User = GetMapObject(State->Map, Cmd->User);
			if (User)
			{
				QueryAsynchronousAction(State, User->Self, Cmd->Interaction, Cmd->Dest, true);

				// NOTE:
				// Query an update of the global distance map if the user requested a transfer command.
				// This will potentially allow us to overlap the distance integration with other animations.

				point_t PredictedDest = User->OccupiedTile;
				if ((Cmd->Interaction == Interaction_Transfer) &&
					IsPassable(State->Map, Cmd->Dest))
				{
					PredictedDest = Cmd->Dest;
				}

				IntegrateRange(&State->FloodQueue, &State->GlobalRange, State->Map, PredictedDest);
			}

			// Bots

			for (int32_t Index = 0; (Index < State->Units.Count) && User; Index++)
			{
				active_unit_t *Unit = &State->Units[Index];

				//Unit->DebugChainCount = 0;

				map_object_t *Object = GetMapObject(State->Map, Unit->Self);
				if (Unit->Bot && Object)
				{
					int32_t Choice = 0;//rand() % 2;
					#if 1
					if (Choice == 0)
						QueryAsynchronousAction(State, Object->Self, Interaction_Seek, {}, {});
					if (Choice == 1)
						QueryAsynchronousAction(State, Object->Self, Interaction_Damage, User->OccupiedTile);
					#endif
				}
			}
		}

		State->Execute = false;
		State->DebugLowestSeekPriority = -1;
	}

	// Actions

	int32_t MinDist = INT32_MAX;

	State->RejectIncomingCommands = (State->Actions.Count > 0); // We won't be accepting any more user commands until all actions will finish.

	for (int32_t Index = 0; Index < State->Actions.Count; Index++)
	{
		action_t *Action = &State->Actions[Index];

		map_object_t *Target, *Source;
		Source = GetMapObject(State->Map, Action->User);
		Target = GetMapObject(State->Map, Action->Target);

		// Animate
		
		float_t Lerp = 1.5f; // Actions that don't define an animation are instantaneous.
		float_t CommitTime = 1.0f;

		if (Action->Anim)
		{
			const animation_t *Anim = Action->Anim;
			if ((Anim->Flags & Anim_Ranged))
			{
				//
				float_t ParticleSpeed = 450.0f;
				
				float_t ParticleStartTime = 0.2f;
				float_t ParticleEndTime = 0.8f;
				//

				// float_t Duration[1] = {};
				CommitTime = ParticleEndTime;

				float_t Dist = Length(Source->BitmapOffset - Target->BitmapOffset);
				float_t CollideTime = Dist / ParticleSpeed;

				if (Action->Time < CollideTime)
				{
					Lerp = (Action->Time / Dist) * ParticleSpeed; // The speed of a ranged animation is distance-depended.
				}

				// DebugPrint("%.2f, %.2f, %.2f", Action->Time, CollideTime, Lerp);

				float_t T = Step(ParticleStartTime, CommitTime, Lerp);
				vec2_t BitmapOffset = Mix(Source->BitmapOffset, Target->BitmapOffset, T);
				vec2_t BitmapSize = V2(4.0f);

				if ((T > 0.0f) && (T < 1.0f))
				{
					RenderRect(Out, BitmapOffset - (BitmapSize * 0.5f), BitmapSize, ColorRed);
				}

				State->Focus = true;
				State->FocusPosition = BitmapOffset;
			}
			else
			{
				Assert(0);
			}

			Action->Time += State->DeltaTime;
		}

		// Commit

		// NOTE: Game logic happens here...

		if ((Lerp >= CommitTime) && !Action->Commited)
		{
			bool Commit = true;

			switch (Action->Mode)
			{
			case Interaction_Transfer:
				Translate(State->Map, Action->User, Action->Dest);
				break;
			case Interaction_Seek:
				{
					Commit = false; // Deffer until the transfer is finished.
						
					if (State->GlobalRange.Completed)
					{
						int32_t CurDist = GetDistanceValue(&State->GlobalRange, Source->OccupiedTile);
						if (CurDist < MinDist)
							MinDist = CurDist;

						if ((State->DebugLowestSeekPriority != -1) && (CurDist == State->DebugLowestSeekPriority))
						{
							point_t ClosestPoint = {};
	
							int32_t MoveCount = 1;
							while (MoveCount--)
							{
							if (DebugRouteLocally(&State->GlobalRange, Source->OccupiedTile, &ClosestPoint, State->Map))
							{
								Translate(State->Map, Action->User, ClosestPoint);
								continue;
							}
							break;
							}
	
							Commit = true;
						}
					}
				} break;
			case Interaction_Damage:
					Target->DamageFlickerTime = 1.0f;
					DebugLog("#%i inflicted ??? damage to #%i!", Action->User, Target->Self);
				break;
			}

			Action->Commited = Commit;
		}

		// Remove

		if ((Lerp > 1.0f) && Action->Commited) 
		{
			// Chaining

			//if ((Action->DebugChain < 1) && (Action->Mode == Interaction_Damage)) // For now, always chain attacks two times.
			//{
			//	State->Actions[Index].Commited = false;
//				State->Actions[Index].Time = 0.0f;

//				Action->DebugChain++;
//			}
			//else
			//{
				State->Actions.Remove(Index--);
			//}
		}

		if (Action->DebugFence)
		{
			break;
		}
	}

	if (MinDist != INT32_MAX)
		State->DebugLowestSeekPriority = MinDist;
}
// ...

//

static void BeginActiveRegion(active_region_t *Region, uint64_t Frame, bounds_t Bounds)
{
	Region->CurrentFrame = Frame;
	Region->VisibleObjects.Clear();
	Region->Bounds = Bounds;
}

static void EndActiveRegion(active_region_t *Region)
{

}

static void FetchMapObject(active_region_t *Region, map_object_t *Object)
{
	if (Object)
	{
		if (Object->LastFrameFetched < Region->CurrentFrame)
		{
			//DebugPrint("%i->%i", Object->LastFrameFetched, Region->CurrentFrame);
			Object->LastFrameFetched = Region->CurrentFrame;
			Region->VisibleObjects.Push(Object->Self);
		}
	}
}

//

// game_state_t

static vec2_t GetActiveCameraCenter(const game_state_t *State)
{
	return (State->Cameras[0].Transform.Center);
}

static vec2_t Project(vec2_t Point)
{
	vec2_t Result = {};
	Result.x = Dot(Point, V2(+0.50f, -0.50f));;
	Result.y = Dot(Point, V2(+0.25f, +0.25f));
	return Result;
}

static vec2_t Unproject(vec2_t Point)
{
	vec2_t Result = {};
	Result.x = Dot(Point, V2(+2.0f, +4.0f));
	Result.y = Dot(Point, V2(-2.0f, +4.0f));
	return Result;
}
// ...

static void RenderActiveRegion(render_output_t *Out, map_t *Map, active_region_t *Region,
		float_t DeltaTime, float_t ElapsedTime, const content_t *Content)
{
	bounds_t CameraBounds = Region->Bounds;
	//DebugPrint("VisibleRegion->Count = %i", VisibleObjects->Count);
	RenderRect(Out, CameraBounds.Min, CameraBounds.Max - CameraBounds.Min, ColorDarkBlue); // Sky

	
	point_t Min = CoordsToCell(Map, CameraBounds.Min);
	point_t Max = CoordsToCell(Map, CameraBounds.Max);

	Min = Clamp(Min, {}, {Map->X - 1, Map->Y - 1});
	Max = Clamp(Max, {}, {Map->X - 1, Map->Y - 1});

	for (int32_t y = Min.y; y <= Max.y; y++)
	{
		for (int32_t x = Min.x; x <= Max.x; x++)
		{
			point_t TileIndex = {x, y};
			const map_tile_t *Tile = GetTile(Map, TileIndex);
			FetchMapObject(Region, GetMapObject(Map, Tile->Object));

			//Assert(Tile);

			rect_t Bounds = GetTileBounds(Map, TileIndex);
			vec4_t Color = ColorWhite;
			
			#if 1
			point_t Grid = TileIndex;
			Grid.x /= 16;
			Grid.y /= 16;

			if ((((Grid.x - 0) % 2) && ((Grid.y - 1) % 2)) ||
				(((Grid.x - 1) % 2) && ((Grid.y - 0) % 2)))
			{
				Color = ColorDarkGreen;
			}
			#endif

			RenderRect(Out, Bounds.Offset, Map->TileSize, Color);
			//RenderBitmapScaled(Out, Project(Bounds.Offset), State->Content->Tiles[0], {32.0f, 16.0f}, Color);

			#if 1
			if (Tile && Tile->Object)
			{
				//RenderRect(Out, Bounds, ColorOrange);	
			}

			if (Tile && (Tile->ID > 0))
			{
				RenderRect(Out, Bounds, ColorBlue);
			}
			#endif
		}
	}
	
	//DebugPrint("VisibleRegion->Count = %i", Region->VisibleObjects.Count);

	for (int32_t Index = 0; Index < Region->VisibleObjects.Count; Index++)
	{
		map_object_t *Object = GetMapObject(Map, Region->VisibleObjects[Index]);

		rect_t ObjectBounds = Rect(Object->BitmapOffset, V2(42.0f));
		rect_t TileBounds = GetTileBounds(Map, Object->OccupiedTile);

		vec2_t Target = RectCenter(TileBounds);

		// if (!(Object->TempOverrides & Override_Interpolator))
		{
			Object->BitmapOffset = Mix(Object->BitmapOffset, Target, DeltaTime * 15.0f);
			if (Length(Target - Object->BitmapOffset) < 1.0f)
				Object->BitmapOffset = Target;
		}

		//DebugPoint(Object->BitmapOffset, ColorRed);

		//RenderRect(Out, ObjectBounds, Object->DebugOverrideColor ? Object->ColorOverride : Object->DebugColor);

		bitmap_t DebugBitmap = {};
		if ((Object->Tags & Object_Hostile)) // Hard-coded player animation...
		{
			DebugBitmap = Content->Slime;
		}
		else
		{
			float_t Time = fmodf(ElapsedTime * 6.0f, 4.0f);
			int32_t Frame = Clamp((int32_t)Time, 0, Len(Content->Player) - 1);
			DebugBitmap = Content->Player[Frame];
		}

		vec4_t Color = Object->DebugOverrideColor? Object->ColorOverride : ColorWhite;
		if (Object->DamageFlickerTime > 0.0f)
		{
			float_t FlickerHz = 1.0f / 6.0f;

			Object->DamageFlickerTime -= DeltaTime;
			Color = (fmodf(Object->DamageFlickerTime, FlickerHz) >= FlickerHz * 0.5f) ? V4(1.0f) : V4(0.0f);
		}

		RenderBitmapScaled(Out, ObjectBounds.Offset - RectExtends(ObjectBounds), DebugBitmap, ObjectBounds.Size, Color);
		DebugRect(ObjectBounds.Offset - RectExtends(ObjectBounds), ObjectBounds.Size, RGBA(255, 0, 255));

		Object->DebugOverrideColor = false;
		// Object->TempOverrides = 0;
	}
	
	//
}

// map_t

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
	if (Tile)
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

static map_object_t *CreateMapObject(map_t *Map, point_t At)
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

// ...

// combat_state_t

static void CreateCombatState(combat_state_t *State, map_t *Map)
{
	memset(State, 0, sizeof(*State));
	State->Map = Map;
}

static combat_mode_state_t BeginCombatMode(combat_state_t *State, float_t DeltaTime)
{
	combat_mode_state_t Result = {};
	Result.AcceptsCommands = (State->RejectIncomingCommands == false);

	Assert(State->Map);

	State->DeltaTime = DeltaTime;
	State->ElapsedTime += State->DeltaTime;

	return Result;
}

static void EndCombatMode(combat_state_t *State)
{
	// Schedule

	if ((State->Actions.Count == 0) && State->Execute) // Only if there are no ongoing actions
	{
		while ((State->InputBuffer.Count > 0))
		{
			// Poll

			const user_command_t *Cmd = State->InputBuffer.Pop();
			//DebugLog("(%i)->{%i,%i}", (int32_t)Cmd->User, Cmd->RelativeOffset.x, Cmd->RelativeOffset.y);

			int32_t ConsumedTime = 5;

			// User

			map_object_t *User = GetMapObject(State->Map, Cmd->User);
			if (User)
			{
				QueryAsynchronousAction(State, User->Self, Cmd->Interaction, Cmd->Dest, true);
			}

			State->ClockCycles += ConsumedTime;

			// Bots

			for (int32_t Index = 0; (Index < State->Units.Count) && User; Index++)
			{
				active_unit_t *Unit = &State->Units[Index];

				//Unit->DebugChainCount = 0;

				map_object_t *Object = GetMapObject(State->Map, Unit->Self);
				if (Unit->Bot && Object)
				{
					int32_t Choice = rand() % 2;
					#if 1
					if (Choice == 0)
						QueryAsynchronousAction(State, Object->Self, Interaction_Transfer, Object->OccupiedTile + Point(-1 + (rand() % 3), -1 + (rand() % 3)));
					if (Choice == 1)
						QueryAsynchronousAction(State, Object->Self, Interaction_Damage, User->OccupiedTile);
					#endif
				}
			}
		}

		State->Execute = false;
	}

	// Actions

	State->RejectIncomingCommands = (State->Actions.Count > 0);

	for (int32_t Index = 0; Index < State->Actions.Count; Index++)
	{
		action_t *Action = &State->Actions[Index];
		Action->Clock += State->DeltaTime;

		map_object_t *Target = GetMapObject(State->Map, Action->Target);

		float_t Duration[3] = {0.45f, 0.45f, 0.45f};
		float_t Lerp = Action->Clock / Duration[Action->DebugSequenceIndex % Len(Duration)];

		map_object_t *User = GetMapObject(State->Map, Action->User);
		if (User)
		{
			DebugPrint("(#%i) -> %i | %.2f%%", User->Self, Action->DebugChain, Lerp * 100.0f);
			User->DebugOverrideColor = true;
			User->ColorOverride = Mix(User->DebugColor, ColorGreen, Step(0.0f, 0.5f, Lerp < 0.5f ? Lerp : (1.0f - Lerp)));
		}

		bool Commit = false;
		bool Remove = false;

		if (Lerp >= 1.0f)
		{
			int32_t Damage = 10;

			if (!Action->Commited)
			{
				Action->Commited = true;
				Commit = true;
			}

			Remove = true;
		}

		switch (Action->Mode)
		{
		case Interaction_Transfer:
			if (Commit)
			{
				Translate(State->Map, Action->User, Action->Cell);
			}	
			break;
		case Interaction_Damage:
			if (User && Target)
			{
				vec4_t Color = ((User->Self == 1) ? ColorGreen : ColorRed);
				DebugLine(
					GetTileCenter(State->Map, User->OccupiedTile),
					GetTileCenter(State->Map, Target->OccupiedTile), Color);
				if (Commit)
				{
					// Damage(State, Target->Self);
				}
			}
			break;
		}

		if (Remove)
		{
			// Chaining

			if ((Action->DebugChain < 1) && (Action->Mode == Interaction_Damage)) // For now, always chain attacks two times.
			{
				State->Actions[Index].Commited = false;
				State->Actions[Index].Clock = 0.0f;

				Action->DebugChain++;
			}
			else
			{
				State->Actions.Remove(Index--);
			}
		}

		if (Action->Fence)
		{
			break;
		}
	}
}

static void JoinCombat(combat_state_t *State, object_t Object, bool Bot)
{
	#if _DEBUG
	
	// NOTE(): Make sure there aren't any duplicates...

	for (int32_t Index = 0; Index < State->Units.Count; Index++)
	{
		Assert(State->Units[Index].Self != Object);
	}

	#endif

	active_unit_t *Unit = State->Units.Push();
	if (Unit)
	{
		Unit->Self = Object;
		Unit->Bot = Bot;
		Unit->Alliance = Bot ? 1 : 0;
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

	Action->Fence = Blocking;
	Action->Mode = Mode;
	Action->User = User;
	Action->Cell = Target;
	Action->DebugSequenceIndex = State->Actions.Count;
	
	Object = GetMapObject(State->Map, Target);
	Action->Target = Object ? Object->Self : 0;
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

static void FormatClock(char *Buffer, int32_t Length, uint64_t Cycles)
{
	double Time = (double)Cycles / (double)COMBAT_CLOCK_FREQUENCY;
	int32_t Seconds = (int32_t)fmod(Time, 60.0f);
	int32_t Minutes = (int32_t)(Time / 60.0f) % 60;
	int32_t Hours = Minutes / 60;

	snprintf(Buffer, Length, "HH:MM:SS = %02i:%02i:%02i", Hours, Minutes, Seconds);
}
// /..

static void RunUserInterface(combat_mode_state_t Mode, combat_state_t *Combat,
	map_t *Map, map_object_t *Player, const input_t *Input, interface_t *UI, float_t DeltaTime)
{
	bool Issue = false;
	user_command_t Cmds[1] = {};

	const controller_t *Controller = &Input->Controllers[0];

	static float_t DebugKeyDelayTime = 0.0f;

	if (Mode.WaitingForCommands && Player)
	{
		DebugKeyDelayTime += DeltaTime;

		if (!Compare(Controller->DPad, {}) && (DebugKeyDelayTime >= 0.5f)) // Move
		{
			DebugKeyDelayTime = 0.0f;

			if (IsPassable(Map, Cmds[0].Dest))
			{
				Cmds[0] = CreateUserCommand(Player->Self, Interaction_Transfer, Player->OccupiedTile + Controller->DPad);
				Issue = true;
			}
		}

		if (Controller->A.Down) // Attack
		{
			map_object_t *Hostile = GetMapObject(Combat, FindAnyHostile(Combat,  Player->Self));
			if (Hostile)
			{
				Cmds[0] = CreateUserCommand(Player->Self, Interaction_Damage, Hostile->OccupiedTile);
				Issue = true;
			}
		}
	}

	if (BeginWindow(UI, 0, {40, 250}, {200, 200}, "Window 1"))
	{
		if (Button(UI, "Test 1"))
			DebugLog("Test 1");
		if (Button(UI, "Test 2"))
			DebugLog("Test 2");
		EndWindow(UI);
	}

	if (BeginWindow(UI, 1, {40, 300}, {150, 400}, "Window 2"))
	{
		if (Button(UI, "Test 1"))
			DebugLog("Test test test test");
		EndWindow(UI);
	}

	if (BeginWindow(UI, 2, {80, 80}, {150, 150}, "Window 3"))
	{

		EndWindow(UI);
	}

	if (Issue)
	{
		IssueUserCommands(Combat, Cmds, Len(Cmds));
	}
}
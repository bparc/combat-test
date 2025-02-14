static void _Debug(game_state_t *State, render_output_t *Out, const camera_t *Camera)
{
	distance_map_t *Map = &State->Combat.GlobalRange;

	bounds_t CameraBounds = GetCameraBounds(GetCameraTransform(State->Cameras[0]));
	point_t Min = CoordsToCell(&State->Map, CameraBounds.Min) - Point(16, 16);
	point_t Max = CoordsToCell(&State->Map, CameraBounds.Max) + Point(16, 16);

	//float_t CompletedPercentage = (float_t)Map->ProcessedNodes / (float_t) ((Map->X * Map->Y )- 2) * 100.0f;

	#if 1
	//if (Map->Completed)
	{
		for (int32_t y = Min.y; y <= Max.y; y++)
		{
			for (int32_t x = Min.x; x <= Max.x; x++)
			{
				rect_t Tile = GetTileBounds(&State->Map, {x, y});
				
				vec4_t Color = ColorDarkGrey;

				//if (Map->Completed)
				//{
				int32_t Dist = GetDistanceValue(Map, {x, y});
				if (Dist != RANGE_MAP_INFINITY)
				{
					float_t T = (float_t)Dist / (32.0f * 1.0f);
					if (T < 0.6f)
					{
						Color = Mix(ColorRed, ColorYellow, Step(0.0f, 0.3f, T));
					}
					else
					{
						Color = Mix(ColorYellow, ColorBlue, Step(0.3f, 1.0f, T));
					}
				}
				//}

				if (Dist < 9)
				{
					rect_t Rect = Shrink(Tile, 4.0f);
					Color = ColorPink;
				}

				if (IsPassable(&State->Map, {x, y}))
				{
					DebugRect(Tile.Offset, Tile.Size, Color);
				}
			}
		}
	}
	#endif
}
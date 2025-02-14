static float_t Sin(float_t Angle)
{
	float_t Result = (float_t)sinf(Angle * (float_t)M_TAU);
	return Result;
}

static float_t Cos(float_t Angle)
{
	float_t Result = (float_t)cosf(Angle * (float_t)M_TAU);
	return Result;
}

static int32_t Clamp(int32_t Value, int32_t Min, int32_t Max)
{
	int32_t Result = Value;
	if (Value < Min)
		Result = Min;
	if (Value > Max)
		Result = Max;
	return Result;
}

static float_t Clamp(float_t Value, float_t Min, float_t Max)
{
	float_t Result = Value;
	if (Value < Min)
		Result = Min;
	if (Value > Max)
		Result = Max;
	return Result;
}

static float_t Abs(float_t A)
{
	float_t Result = fabsf(A);
	return Result;
}

static float_t Min(float_t A, float_t B)
{
	float_t Result = A < B ? A : B;
	return Result;
}

static float_t Max(float_t A, float_t B)
{
	float_t Result = A > B ? A : B;
	return Result;
}

static int32_t Min(int32_t A, int32_t B)
{
	int32_t Result = A < B ? A : B;
	return Result;
}

static int32_t Max(int32_t A, int32_t B)
{
	int32_t Result = A > B ? A : B;
	return Result;
}

static float_t Sign(float_t Value)
{
	float_t Result = Value >= 0.0f ? +1.0f : -1.0f;
	return Result;
}

static float_t Mix(float_t X, float_t Y, float_t A)
{
	float_t Result = X;
	if (A <= 0.0f)
		Result = X;
	else
	if (A >= 1.0f)
		Result = Y;
	else
		Result = ((1.0f - A) * X) + (A * Y);
	return Result;
}

static float_t Step(float_t Edge0, float_t Edge1, float_t t)
{
	float Result = Clamp((t - Edge0) / (Edge1 - Edge0), 0.0f, 1.0f);
	return Result;
}

static void SetupMemory(memory_t *Memory, uint8_t *Bytes, int32_t Length)
{
	memset(Memory, 0, sizeof(*Memory));
	Memory->Length = Length;
	Memory->Bytes = Bytes;
}

static void Flush(memory_t *Memory)
{
	Memory->Offset = 0;
}

static void *Push(memory_t *Memory, int32_t Count)
{
	uint8_t *Result = 0;

	#if _DEBUG
	Result = (uint8_t *)malloc(Count);
	#else
	if ((Memory->Offset + Count) <= Memory->Length)
	{
		Result = &Memory->Bytes[Memory->Offset];
		Memory->Offset += Count;
	}
	#endif

	memset(Result, 0, Count);

	return Result;
}

static void Setup(byte_stream_t *Stream, uint8_t *Bytes, int32_t Length, int32_t From)
{
	memset(Stream, 0, sizeof(*Stream));
	Stream->Length = Length;
	Stream->Bytes = Bytes;
	Stream->Offset = From;
}

static void Setup(byte_stream_t *Stream, const file_contents_t *Contents, int32_t From)
{
	Setup(Stream, Contents->Data, Contents->Length, From);
}

static void *Read(byte_stream_t *Stream, int32_t Count)
{
	void *Result = 0;
	if ((Stream->Offset + Count) <= Stream->Length)
	{
		Result = &Stream->Bytes[Stream->Offset];
	}
	Stream->Offset += Count;
	return Result;
}

static int32_t _EOF(byte_stream_t *Stream)
{
	int32_t Result = (Stream->Offset >= Stream->Length);
	return Result;
}

static float_t Milliseconds(float_t Seconds)
{
	float_t Result = Seconds * 1000.0f;
	return Result;
}

static const char *SkipToLastOccurence(const char *Str, char Ch)
{
	const char *Result = Str;

	const char *At = Str;
	while (*At)
	{
		if (*At == Ch)
		{
			Result = &At[1];
		}

		At++;
	}

	return Result;
}

static point_t Point(int32_t X, int32_t Y)
{
	point_t Result = {X, Y};
	return Result;
}

static point_t Point(struct vec2_t A)
{
	point_t Result = {};
	Result.x = (int32_t)A.x;
	Result.y = (int32_t)A.y;
	return Result;
}

static bool Compare(point_t A, point_t B)
{
	bool Result = (A.x == B.x) && (A.y == B.y);
	return Result;
}

static int32_t ManhattanDistance(point_t A, point_t B)
{
	int32_t Result = abs(B.x - A.x) + abs(B.y - A.y);
	return Result;
}


static float_t Distance(point_t A, point_t B)
{
	float_t Result = Length(V2(B) - V2(A));
	return Result;
}

static point_t operator+(const point_t &A, const point_t &B)
{
	point_t Result = A;
	Result.x += B.x;
	Result.y += B.y;
	return Result;
}

static point_t operator-(const point_t &A, const point_t &B)
{
	point_t Result = A;
	Result.x -= B.x;
	Result.y -= B.y;
	return Result;
}

static point_t Clamp(point_t Value, point_t Min, point_t Max)
{
	point_t Result = Value;
	Result.x = Clamp(Result.x, Min.x, Max.x);
	Result.y = Clamp(Result.y, Min.y, Max.y);
	return Result;
}
//

static vec2_t V2(float_t x, float_t y)
{
	vec2_t Result = {x, y};
	return Result;
}

static vec2_t V2(float_t x)
{
	vec2_t Result = {x, x};
	return Result;
}

static vec2_t V2(int32_t x, int32_t y)
{
	vec2_t Result = {};
	Result.x = (float_t)x;
	Result.y = (float_t)y;
	return Result;
}

static vec2_t V2(int32_t *x)
{
	vec2_t Result = V2(x[0], x[1]);
	return Result;
}

static vec2_t V2(point_t Point)
{
	vec2_t Result = {};
	Result.x = (float_t)Point.x;
	Result.y = (float_t)Point.y;
	return Result;
}

static bool Compare(vec2_t A, vec2_t B)
{
	bool Result = (A.x == B.x) && (A.y == B.y);
	return Result;
}

static vec2_t operator+(const vec2_t &A, const vec2_t &B)
{
	vec2_t Result = A;
	Result.x += B.x;
	Result.y += B.y;
	return Result;
}

static vec2_t operator-(const vec2_t &A, const vec2_t &B)
{
	vec2_t Result = A;
	Result.x -= B.x;
	Result.y -= B.y;
	return Result;
}

static vec2_t operator*(const vec2_t &A, const vec2_t &B)
{
	vec2_t Result = A;
	Result.x *= B.x;
	Result.y *= B.y;
	return Result;
}

static vec2_t operator/(const vec2_t &A, const vec2_t &B)
{
	vec2_t Result = A;
	Result.x /= B.x;
	Result.y /= B.y;
	return Result;
}

static vec2_t operator*(const vec2_t &A, float B)
{
	vec2_t Result = A;
	Result.x *= B;
	Result.y *= B;
	return Result;
}

static vec2_t operator/(const vec2_t &A, float B)
{
	vec2_t Result = A;
	Result.x /= B;
	Result.y /= B;
	return Result;
}

static float_t Dot(vec2_t A, vec2_t B)
{
	float_t Result = A.x * B.x + A.y * B.y;
	return Result;
}

static float_t Length(vec2_t A)
{
	float_t Result = sqrtf(Dot(A, A));
	return Result;
}

static vec2_t Normalize(vec2_t A)
{
	float_t Len = Length(A);
	if (Len > 0)
	{
		A = A / Len;
	}
	return A;
}

static vec2_t Clamp(vec2_t Value, vec2_t Min, vec2_t Max)
{
	vec2_t Result = {};
	Result.x = Clamp(Value.x, Min.x, Max.x);
	Result.y = Clamp(Value.y, Min.y, Max.y);
	return Result;
}

static vec2_t Abs(vec2_t A)
{
	vec2_t Result = A;
	Result.x = fabsf(A.x);
	Result.y = fabsf(A.y);
	return Result;
}

static vec2_t Floor(vec2_t A)
{
	vec2_t Result = A;
	Result.x = floorf(A.x);
	Result.y = floorf(A.y);
	return Result;
}

static vec2_t Mix(vec2_t X, vec2_t Y, float_t A)
{
	vec2_t Result = {};
	Result.x = Mix(X.x, Y.x, A);
	Result.y = Mix(X.y, Y.y, A);
	return Result;
}

static vec4_t V4(float_t x)
{
	vec4_t Result = {x, x, x, x};
	return Result;
}

static vec4_t V4(vec4_t A, float_t W)
{
	vec4_t Result = {A.x, A.y, A.z, W};
	return Result;
}

static vec4_t V4(vec2_t A, vec2_t B)
{
	vec4_t Result = {};
	Result.x = A.x;
	Result.y = A.y;
	Result.z = B.x;
	Result.w = B.y;
	return Result;
}

static vec4_t Mix(vec4_t X, vec4_t Y, float_t A)
{
	vec4_t Result = {};
	Result.x = Mix(X.x, Y.x, A);
	Result.y = Mix(X.y, Y.y, A);
	Result.z = Mix(X.z, Y.z, A);
	Result.w = Mix(X.w, Y.w, A);
	return Result;
}

static vec4_t RGBA(uint8_t R, uint8_t G, uint8_t B, uint8_t A)
{
	vec4_t Result = {};
	Result.x = (float_t)R / 255.0f;
	Result.y = (float_t)G / 255.0f;
	Result.z = (float_t)B / 255.0f;
	Result.w = (float_t)A / 255.0f;
	return Result;
}

static vec4_t operator*(const vec4_t &A, float B)
{
	vec4_t Result = {};
	Result.x = A.x * B;
	Result.y = A.y * B;
	Result.z = A.z * B;
	Result.w = A.w * B;
	return Result;
}

static rect_t Rect(vec2_t Offset, vec2_t Size)
{
	rect_t Result = {};
	Result.Offset = Offset;
	Result.Size = Size;
	return Result;
}

static rect_t Rect(struct bounds_t Bounds)
{
	rect_t Result = {};
	Result.Offset = Bounds.Min;
	Result.Size = Bounds.Max - Bounds.Min;
	return Result;
}

static vec2_t RectCenter(rect_t Rect)
{
	vec2_t Result = Rect.Offset + Rect.Size * 0.5f;
	return Result;
}

static vec2_t RectExtends(rect_t Rect)
{
	vec2_t Result = Rect.Size * 0.5f;
	return Result;
}

static vec2_t RectMin(rect_t Rect)
{
	return (Rect.Offset);
}

static vec2_t RectMax(rect_t Rect)
{
	vec2_t Result = Rect.Offset + Rect.Size;
	return Result;
}

static rect_t Shrink(rect_t Rect, float_t Amount)
{
	rect_t Result = Rect;
	Result.Offset.x += Amount;
	Result.Offset.y += Amount;
	Result.Size.x -= Amount * 2.0f;
	Result.Size.y -= Amount * 2.0f;
	return Result;
}

static rect_t Stretch(rect_t Rect, float_t Amount)
{
	rect_t Result = Rect;
	Result.Offset.x -= Amount;
	Result.Offset.y -= Amount;
	Result.Size.x += Amount * 2.0f;
	Result.Size.y += Amount * 2.0f;
	return Result;
}

static vec4_t V4(rect_t Rect)
{
	vec4_t Result = {Rect.x, Rect.y, Rect.Width, Rect.Height};
	return Result;
}

static rect_t MaintainAspectRatio(rect_t Rect, float Aspect)
{
	rect_t Result = Rect;

	if ((Rect.Width / Rect.Height) > Aspect)
	{
		Result.Width = Rect.Height * Aspect;
		Result.x = (Rect.Width * 0.5f) - (Result.Width * 0.5f);
	}
	else
	{
		Result.Height = Rect.Width / Aspect;
		Result.y = (Rect.Height * 0.5f) - (Result.Height * 0.5f);
	}

	return Result;
}

static bool Contains(rect_t Rect, vec2_t Point)
{
	bounds_t Bound = Bounds(Rect);
	bool Result = (Point.x >= Bound.Min.x) && (Point.y >= Bound.Min.y) &&
		(Point.x <= Bound.Max.x) && (Point.y <= Bound.Max.y);
	return Result;
}

static bounds_t Bounds(vec2_t Min, vec2_t Max)
{
	bounds_t Result = {};
	Result.Min = Min;
	Result.Max = Max;
	return Result;
}

static bounds_t Bounds(rect_t Rect)
{
	bounds_t Result = Bounds(Rect.Offset, Rect.Offset + Rect.Size);
	return Result;
}

static bounds_t Stretch(bounds_t Bb, float_t Amount)
{
	bounds_t Result = Bb;
	Result.Min = Result.Min - V2(Amount);
	Result.Max = Result.Max + V2(Amount);
	return Result;
}

//

static sphere_t Sphere(vec2_t Center, float_t Radius)
{
	sphere_t Result = {};
	Result.Center = Center;
	Result.Radius = Radius;
	return Result;
}

//

static intersection_t IntersectRectangles(rect_t A, rect_t B)
{
	vec2_t Delta = RectCenter(B) - RectCenter(A);
	vec2_t AxisDistance = Abs(Delta);
	vec2_t Overlap = (RectExtends(A) + RectExtends(B)) - AxisDistance;

	intersection_t Result = {};
	Result.Intersected = (Overlap.x >= 0.0f) && (Overlap.y >= 0.0f);

	if (Result.Intersected)
	{
		if (Overlap.x < Overlap.y)
		{
			Result.Normal = {Delta.x > 0 ? -1.0f : +1.0f, 0.0f};
			Result.Depth = Overlap.x;
		}
		else
		{
			Result.Normal = {0.0f, Delta.y > 0 ? -1.0f : +1.0f};
			Result.Depth = Overlap.y;
		}
	}

	return Result;
}

static intersection_t IntersectRectangleToSphere(rect_t A, sphere_t B)
{
	vec2_t ClippedSphereCenter = Clamp(B.Center, RectMin(A), RectMax(A));

	vec2_t Delta = B.Center - ClippedSphereCenter;
	float_t Distance = Length(Delta);

	intersection_t Result = {};
	Result.Intersected = Distance <= B.Radius;

	if (Result.Intersected)
	{
		Result.Normal = Normalize(Delta);
		Result.Depth = B.Radius - Distance;
	}

	return Result;
}

// ...

static surface_t CreateSurface(int32_t X, int32_t Y, uint32_t *Pixels)
{
	surface_t Result = {};
	Result.Pixels = Pixels;
	Result.X = X;
	Result.Y = Y;
	return Result;
}

static void SetPixel(surface_t *Surface, int32_t X, int32_t Y, uint32_t Pixel)
{
	uint32_t *At = &Surface->Pixels[Surface->X * Y + X];
	if ((X >= 0) && (Y >= 0) && (X < Surface->X) && (Y < Surface->Y))
		*At = Pixel;
}

static uint32_t GetPixel(const surface_t *Surface, int32_t X, int32_t Y)
{
	uint32_t Result = 0;
	if ((X >= 0) && (Y >= 0) && (X < Surface->X) && (Y < Surface->Y))
		Result = Surface->Pixels[Surface->X * Y + X];
	return Result;
}

static void Copy(const surface_t *From, surface_t *To, int32_t XOffset, int32_t YOffset)
{
	const uint32_t *At = From->Pixels;
	for (int32_t y = 0; y < From->Y; y++)
	{
		for (int32_t x = 0; x < From->X; x++)
		{
			SetPixel(To, XOffset + x, YOffset + y, *At++);
		}
	}
}

static void CopyResize(const surface_t *From, surface_t *To)
{
	for (int32_t y = 0; y < To->Y; y++)
	{
		for (int32_t x = 0; x < To->X; x++)
		{
			float_t U = ((float_t)x / (float_t)To->X) * (float_t)From->X;
			float_t V = ((float_t)y / (float_t)To->Y) * (float_t)From->Y;

			uint32_t Pixel = GetPixel(From, (int32_t)U, (int32_t)V);
			SetPixel(To, x, y, Pixel);
		}
	}
}

static void ClearColor(surface_t *Surface, int32_t Color)
{
	for (int32_t y = 0; y < Surface->Y; y++)
	{
		for (int32_t x = 0; x < Surface->X; x++)
		{
			SetPixel(Surface, x, y, Color);
		}
	}
}

static void PremultiplyAlpha(surface_t *Surface)
{
	for (int32_t y = 0; y < Surface->Y; y++)
	{
		for (int32_t x = 0; x < Surface->X; x++)
		{
			uint32_t Pixel = GetPixel(Surface, x, y);
			uint8_t R = Pixel & 0xff;
			uint8_t G = (Pixel >> 8) & 0x00ff;
			uint8_t B = (Pixel >> 16) & 0x0000ff;
			uint8_t Alpha = (Pixel >> 24) & 0x000000ff;
			float_t A = (float_t)Alpha / 255.0f;
			R = (uint8_t)((float_t)R * A);
			G = (uint8_t)((float_t)G * A);
			B = (uint8_t)((float_t)B * A);
			Pixel = (Alpha << 24) | (B << 16) | (G << 8) | R;
			SetPixel(Surface, x, y, Pixel);
		}
	}
}
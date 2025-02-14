#define _CRT_SECURE_NO_WARNINGS
#include <inttypes.h>
#include <ctype.h>
#include <float.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <stdlib.h>
#include <stdarg.h>
#define _USE_MATH_DEFINES
#include <math.h>
#define M_TAU (M_PI * 2)
#include "MurmurHash3.h"
#include "MurmurHash3.cpp"

#define Len(Array) (sizeof(Array) / sizeof(*Array))

#define KB(Bytes) (Bytes * 1024)
#define MB(Bytes) (Bytes * 1024 * 1024)
#define GB(Bytes) (Bytes * 1024 * 1024 * 1024)

//

static void _Assert(const char *message, const char *file, const char *function, int32_t line);
#define Assert(expression) (void) ((!!(expression)) || (_Assert(#expression, __FILE__, __FUNCTION__, __LINE__), 0))

static void _Error(const char *_format, ...);
#define Error(Message, ...) _Error(Message, __VA_ARGS__)

static void _DebugLog(const char *Format, ...);
#ifdef _DEBUG
#define DebugLog(Format, ...) _DebugLog("%04i %-10s %s(): " ## Format ## "\n", __LINE__, SkipToLastOccurence(__FILE__,'\\'), __func__, __VA_ARGS__)
#else
#define DebugLog(...)
#endif

static float_t Milliseconds(float_t Seconds);
static double DebugGetTime(void);

//

static const char *SkipToLastOccurence(const char *Str, char Ch);

// 
static float_t Sin(float_t Angle); // Range: 0.0 - 1.0
static float_t Cos(float_t Angle);

static float_t Min(float_t A, float_t B);
static float_t Max(float_t A, float_t B);
static int32_t Min(int32_t A, int32_t B);
static int32_t Max(int32_t A, int32_t B);

static float_t Sign(float_t Value);

static int32_t Clamp(int32_t Value, int32_t Min, int32_t Max);
static float_t Clamp(float_t Value, float_t Min, float_t Max);

static float_t Mix(float_t X, float_t Y, float_t A);
static float_t Step(float_t Edge0, float_t Edge1, float_t t);

// ...

struct memory_t
{
	int32_t Length;
	int32_t Offset;
	uint8_t *Bytes;
};

static void SetupMemory(memory_t *Memory, uint8_t *Bytes, int32_t Length);
static void Flush(memory_t *Memory);
static void *Push(memory_t *Memory, int32_t Count);
#define PushType(Memory, Type) (Type *)Push(Memory, sizeof(Type))
#define PushArray(Memory, Type, Count) (Type *)Push(Memory, sizeof(Type) * Count)


struct file_contents_t
{
	int32_t Length;
	uint8_t *Data;
};

static file_contents_t LoadFileFromDirectory(const char *Directory); // Implemented by Platform/
static void FreeFileContents(file_contents_t *Contents);

struct byte_stream_t
{
	int32_t Length;
	int32_t Offset;
	uint8_t *Bytes;
};

static void Setup(byte_stream_t *Stream, uint8_t *Bytes, int32_t Length, int32_t From = 0);
static void Setup(byte_stream_t *Stream, const file_contents_t *Contents, int32_t From = 0);
static void *Read(byte_stream_t *Stream, int32_t Count);
static int32_t _EOF(byte_stream_t *Stream);

#define ReadStruct(Stream, Type) (Type *) Read(Stream, sizeof(Type))
//

struct point_t
{
	int32_t x;
	int32_t y;
};

static point_t Point(int32_t X, int32_t Y);
static point_t Point(struct vec2_t A);
static bool Compare(point_t A, point_t B);
static int32_t ManhattanDistance(point_t A, point_t B);
static float_t Distance(point_t A, point_t B);

static point_t operator+(const point_t &A, const point_t &B);
static point_t operator-(const point_t &A, const point_t &B);

static point_t Clamp(point_t Value, point_t Min, point_t Max);

struct vec2_t
{
	float_t x;
	float_t y;
};

static vec2_t V2(float_t x, float_t y);
static vec2_t V2(float_t x);
static vec2_t V2(int32_t x, int32_t y);
static vec2_t V2(int32_t *x);
static vec2_t V2(point_t Point);

static bool Compare(vec2_t A, vec2_t B);

static vec2_t operator+(const vec2_t &A, const vec2_t &B);
static vec2_t operator-(const vec2_t &A, const vec2_t &B);
static vec2_t operator*(const vec2_t &A, const vec2_t &B);
static vec2_t operator/(const vec2_t &A, const vec2_t &B);
static vec2_t operator*(const vec2_t &A, float B);
static vec2_t operator/(const vec2_t &A, float B);

static float_t Dot(vec2_t A, vec2_t B);
static float_t Length(vec2_t A);
static vec2_t Normalize(vec2_t A);

static vec2_t Mix(vec2_t X, vec2_t Y, float_t A);
static vec2_t Clamp(vec2_t Value, vec2_t Min, vec2_t Max);
static vec2_t Abs(vec2_t A);
static vec2_t Floor(vec2_t A);

struct 
vec4_t
{
	float_t x;
	float_t y;
	float_t z;
	float_t w;
};

static vec4_t V4(float_t x); // x, x, x, x
static vec4_t V4(vec4_t A, float_t W);
static vec4_t V4(vec2_t A, vec2_t B);
static vec4_t Mix(vec4_t X, vec4_t Y, float_t A);
static vec4_t RGBA(uint8_t R, uint8_t G, uint8_t B, uint8_t A = 255);
static vec4_t operator*(const vec4_t &A, float B);

#define _RGB(R, G, B) {(float)R / 255.0f, (float)G / 255.0f, (float)B / 255.0f, 1.0f}
// PICO-8 Palette (https://lospec.com/palette-list/pico-8)
static const vec4_t ColorBlack = _RGB(0, 0, 0);
static const vec4_t ColorDarkBlue = _RGB(29, 43, 83);
static const vec4_t ColorDarkPurple = _RGB(126, 37, 83);
static const vec4_t ColorDarkGreen = _RGB(0, 135, 81);
static const vec4_t ColorBrown = _RGB(171, 82, 54);
static const vec4_t ColorDarkGrey = _RGB(95, 87, 79);
static const vec4_t ColorLightGrey = _RGB(194, 195, 199);
static const vec4_t ColorWhite = _RGB(255, 241, 232);
static const vec4_t ColorRed = _RGB(255, 0, 77);
static const vec4_t ColorOrange = _RGB(255, 163, 0);
static const vec4_t ColorYellow = _RGB(255, 236, 39);
static const vec4_t ColorGreen = _RGB(0, 228, 54);
static const vec4_t ColorBlue = _RGB(41, 173, 255);
static const vec4_t ColorLavender = _RGB(131, 118, 156);
static const vec4_t ColorPink = _RGB(255, 119, 168);
static const vec4_t ColorLightPeach = _RGB(255, 204, 170);
static const vec4_t ColorMagenta = _RGB(255, 0, 255);
/*
static const vec4_t ColorPalette[16] = {
	ColorBlack, ColorDarkBlue, ColorDarkPurple, ColorDarkGreen,ColorBrown, ColorDarkGrey,
	ColorLightGrey, ColorWhite, ColorRed, ColorOrange, ColorYellow, ColorGreen, ColorBlue,
	ColorLavender, ColorPink, ColorLightPeach};
*/
#undef _RGB

union rect_t
{
	struct
	{
		float_t x;
		float_t y;
		float_t Width;
		float_t Height;
	};

	struct
	{
		vec2_t Offset;
		vec2_t Size;
	};
};

static rect_t Rect(vec2_t Offset, vec2_t Size);
static rect_t Rect(struct bounds_t Bounds);
static vec2_t RectCenter(rect_t Rect);
static vec2_t RectExtends(rect_t Rect);
static vec2_t RectMin(rect_t Rect);
static vec2_t RectMax(rect_t Rect);
static rect_t Shrink(rect_t Rect, float_t Amount);
static rect_t Stretch(rect_t Rect, float_t Amount);
static vec4_t V4(rect_t Rect);
static rect_t MaintainAspectRatio(rect_t Rect, float Aspect);

static bool Contains(rect_t Rect, vec2_t Point);

struct bounds_t
{
	vec2_t Min;
	vec2_t Max;
};

static bounds_t Bounds(vec2_t Min, vec2_t Max);
static bounds_t Bounds(rect_t Rect);
static bounds_t Stretch(bounds_t Bb, float Amount);
	
struct sphere_t
{
	vec2_t Center;
	float_t Radius;
};

static sphere_t Sphere(vec2_t Center, float_t Radius);

// ...
struct intersection_t
{
	int32_t Intersected;
	vec2_t Normal;
	float_t Depth;
};

static intersection_t IntersectRectangles(rect_t A, rect_t B);
static intersection_t IntersectSpheres(sphere_t A, sphere_t B);
static intersection_t IntersectRectangleToSphere(rect_t A, sphere_t B);

//

struct surface_t
{
	uint32_t *Pixels;
	int32_t X;
	int32_t Y;
};

static surface_t CreateSurface(int32_t X, int32_t Y, uint32_t *Pixels);
static void SetPixel(surface_t *Surface, int32_t X, int32_t Y, uint32_t Pixel);
static uint32_t GetPixel(const surface_t *Surface, int32_t X, int32_t Y);

static void Copy(const surface_t *From, surface_t *To, int32_t XOffset, int32_t YOffset);
static void CopyResize(const surface_t *From, surface_t *To);

static void ClearColor(surface_t *Surface, int32_t Color);
static void PremultiplyAlpha(surface_t *Surface);

template<typename _Type, int32_t MaxCount>
struct array_t
{
	int32_t Count;
	_Type Values[MaxCount];

	_Type *Get(int32_t Index)
	{
		_Type *Result = 0;
		if (Index >= 0 && Index < Count)
			Result = &Values[Index];
		return Result;
	}

	_Type *Push(int32_t Number = 1)
	{
		_Type *Result = 0;
		if ((Count + Number) <= Len(Values))
		{
			Result = &Values[Count];
			Count += Number;
		}
		return Result;
	}

	_Type *Push(_Type Value, int32_t Number = 1)
	{
		_Type *Result = Push(Number);
		if (Result)
			*Result = Value;
		return Result;
	}

	_Type *Pop(void)
	{
		_Type *Result = 0;
		if (Count > 0)
		{
			Result = &Values[--Count];
		}
		return Result;
	}

	_Type *Top(void)
	{
		_Type *Result = 0;
		if (Count > 0)
		{
			Result = &Values[Count - 1];
		}
		return Result;
	}

	void Remove(int32_t Index) // unordered
	{
		if (Index >= 0 && Index < Count)
		{
			Values[Index] = Values[--Count];
		}
	}

	void Clear(void)
	{
		Count = 0;
	}

	_Type &operator[](int32_t Index)
	{
		Assert(Index >= 0 && Index < Count);
		_Type &Result = Values[Index];
		return Result;
	}
};
// Assets.h
#include "File Formats/BMFont.h"
#include "File Formats/BMFont.cpp"

// Handles
typedef uint16_t bitmap_t;
typedef struct asset_font_t *font_t;
// ...

struct asset_bitmap_t
{
	vec2_t Min;
	vec2_t Max;
};

struct asset_font_t
{
	bitmap_t Bitmap;
	bmfont_t Info;
};

// ...

struct cache_page_t
{
	uint8_t Reserved;
};

struct bitmap_cache_t
{
	int32_t Padding;
	int32_t PageSize;
	int32_t Pitch;
	cache_page_t Pages[64][64];

	texture_resource_t Texture;
	surface_t Surface;
	uint32_t BitmapMemory[4096][4096];
	bool Dirty;
};

static cache_page_t *GetPage(bitmap_cache_t *Cache, int32_t X, int32_t Y);
static int32_t TryReservePages(const bitmap_cache_t *Cache, int32_t X, int32_t Y, int32_t XCount, int32_t YCount);
static point_t FindPage(bitmap_cache_t *Cache, int32_t XMin, int32_t YMin);

static void SetupBitmapCache(bitmap_cache_t *Cache);
static void CopyToCache(bitmap_cache_t *Cache, int32_t X, int32_t Y, uint32_t *Pixels, bounds_t *TexCoords);
static void UpdateCache(bitmap_cache_t *Cache, graphics_device_t *Device);

// ...

struct assets_t
{
	char RootDirectory[256];
	
	int32_t BitmapCount;
	asset_bitmap_t Bitmaps[4096];
	bitmap_t None;

	int32_t FontCount;
	asset_font_t Fonts[8];
	font_t FontDefault;

	bitmap_cache_t Cache;
};

static void MountDirectory(assets_t *Assets, const char *Directory);

static bitmap_t LoadBitmapFromMemory(assets_t *Assets, int32_t X, int32_t Y, const void *Pixels);
static bitmap_t LoadBitmapFromDirectory(assets_t *Assets, const char *Directory); // Formats: *.tga
static font_t LoadFontFromDirectory(assets_t *Assets, const char *Directory); // Formats: BMFont (binary)
static void SetDefaultFont(assets_t *Assets, font_t Font);

static asset_bitmap_t *GetBitmap(assets_t *Assets, bitmap_t Bitmap);
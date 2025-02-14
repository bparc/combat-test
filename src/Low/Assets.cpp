
// bitmap_cache_t

static void SetupBitmapCache(bitmap_cache_t *Cache)
{
	memset(Cache, 0, sizeof(*Cache));
	int32_t X = Len(Cache->BitmapMemory[0]);
	int32_t Y = Len(Cache->BitmapMemory);
	Cache->Surface = CreateSurface(X, Y, &Cache->BitmapMemory[0][0]);

	Cache->PageSize = 32;
	Cache->Padding = 4;
	Cache->Pitch = X / Cache->PageSize;
}

static cache_page_t *GetPage(bitmap_cache_t *Cache, int32_t X, int32_t Y)
{
	cache_page_t *Result = 0;

	if ((X < Len(Cache->Pages[0])) && (Y < Len(Cache->Pages)) &&
	    (X >= 0) && (Y >= 0))
	{
		Result = &Cache->Pages[Y][X];
	}
	return Result;
}

static int32_t TryReservePages(bitmap_cache_t *Cache, int32_t X, int32_t Y, int32_t XCount, int32_t YCount)
{
	int32_t Count = 0;

	for (int32_t y = Y; y <= (Y + YCount); y++)
	{
		for (int32_t x = X; x <= (X + XCount); x++)
		{
			cache_page_t *Page = GetPage(Cache, x, y);
			if (Page && !Page->Reserved)
			{
				Count++;
				continue;
			}

			return (0);
		}
	}

	for (int32_t y = Y; y <= (Y + YCount); y++)
	{
		for (int32_t x = X; x <= (X + XCount); x++)
		{
			cache_page_t *Page = GetPage(Cache, x, y);
			if (Page)
				Page->Reserved = true;
		}
	}

	return Count;
}

static point_t FindPage(bitmap_cache_t *Cache, int32_t XMin, int32_t YMin)
{
	int32_t XPageCount = XMin / Cache->PageSize;
	int32_t YPageCount = YMin / Cache->PageSize;

	for (int32_t y = 0; y < Len(Cache->Pages); y++)
	{
		for (int32_t x = 0; x < Len(Cache->Pages[0]); x++)
		{
			if (TryReservePages(Cache, x, y, XPageCount, YPageCount))
				return {x, y};
		}
	}

	return {-1, -1};
}

static void CopyToCache(bitmap_cache_t *Cache, int32_t X, int32_t Y, uint32_t *Pixels, bounds_t *TexCoords)
{
	point_t BestPage = FindPage(Cache, X + (Cache->Padding * 2), Y + (Cache->Padding * 2));
	if ((BestPage.x >= 0) && (BestPage.y >= 0))
	{
		int32_t XDest = (BestPage.x * Cache->PageSize) + Cache->Padding;
		int32_t YDest = (BestPage.y * Cache->PageSize) + Cache->Padding;

		surface_t Source = CreateSurface(X, Y, Pixels);
		PremultiplyAlpha(&Source);

		Copy(&Source, &Cache->Surface, XDest, YDest);
		Cache->Dirty = true;
	
		TexCoords->Min = V2(XDest, YDest);
		TexCoords->Max = TexCoords->Min + V2(X, Y);
	}
}

static void UpdateCache(bitmap_cache_t *Cache, graphics_device_t *Device)
{
	if (Cache->Dirty)
	{
		FreeTextureResource(Device, &Cache->Texture);
		Cache->Texture = CreateTextureResource(Device, &Cache->Surface, 0);
		Cache->Dirty = false;
	}
}

// ...

static void Setup(assets_t *Assets, graphics_device_t *Device)
{
	memset(Assets, 0, sizeof(*Assets));

	SetupBitmapCache(&Assets->Cache);

	uint32_t White[16][16] =
	{
		0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
		0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
		0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
		0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	};

	Assets->None = LoadBitmapFromMemory(Assets, 4, 4, &White[0][0]);
}

static void UpdateAssetCache(assets_t *Assets, graphics_device_t *Device)
{
	UpdateCache(&Assets->Cache, Device);
}

static void MountDirectory(assets_t *Assets, const char *Directory)
{
	strncpy(Assets->RootDirectory, Directory, Len(Assets->RootDirectory));
}

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#define STBI_ONLY_TGA
#include "stb/stb_image.h"

static bitmap_t LoadBitmapFromDirectory(assets_t *Assets, const char *Directory)
{
	bitmap_t Result = 0;

	char FullDirectory[256] = "";
	snprintf(FullDirectory, Len(FullDirectory), "%s%s", Assets->RootDirectory, Directory); // TODO: GetFullDirectory()

	int32_t X, Y;
	uint8_t *Pixels = stbi_load(FullDirectory, &X, &Y, 0, 4);
	if (Pixels)
	{
		Result = LoadBitmapFromMemory(Assets, X, Y, Pixels);
		free(Pixels);
	}
	else
	{
		DebugLog("Couldn't load %s", FullDirectory);
	}

	return Result;
}

static font_t LoadFontFromDirectory(assets_t *Assets, const char *Directory)
{
	asset_font_t *Result = 0;
	if (Assets->FontCount < Len(Assets->Fonts))
	{
		char FullDirectory[256] = "";
		snprintf(FullDirectory, Len(FullDirectory), "%s%s", Assets->RootDirectory, Directory); // TODO: GetFullDirectory()

		Result = &Assets->Fonts[Assets->FontCount++];
		LoadBMFontFromDirectory(&Result->Info, FullDirectory);
		Result->Bitmap = LoadBitmapFromDirectory(Assets, Result->Info.Pages[0].FileName);

		Assert(Result->Bitmap);
	}
	return Result;
}

static void SetDefaultFont(assets_t *Assets, font_t Font)
{
	Assets->FontDefault = Font;
	Assert(Assets->FontDefault);
}

static bitmap_t LoadBitmapFromMemory(assets_t *Assets, int32_t X, int32_t Y, const void *Pixels)
{
	bitmap_t Result = 0;

	if (Assets->BitmapCount < Len(Assets->Bitmaps))
	{
		asset_bitmap_t *Bitmap = &Assets->Bitmaps[Assets->BitmapCount++];
		Result = (bitmap_t)Assets->BitmapCount;

		bounds_t TexCoords = {};
		CopyToCache(&Assets->Cache, X, Y, (uint32_t *)Pixels, &TexCoords);
		Bitmap->Min = TexCoords.Min;
		Bitmap->Max = TexCoords.Max;
	}

	return Result;
}

static asset_bitmap_t *GetBitmap(assets_t *Assets, bitmap_t Bitmap)
{
	asset_bitmap_t *Result = 0;
	if ((Bitmap > 0) && (Bitmap <= Assets->BitmapCount))
	{
		Result = &Assets->Bitmaps[Bitmap - 1];
	}
	return Result;
}

static uint32_t GetTextureCacheHandle(const assets_t *Assets)
{
	return (Assets->Cache.Texture.Handle);
}
// Device.h

typedef struct graphics_device_t graphics_device_t;
static void Dispatch(graphics_device_t *Device, const struct command_buffer_t *Cmds, vec4_t View, struct transform_t Transform);
//static void ClearColor(graphics_device_t *Device, vec4_t Color);

// Resources

struct texture_resource_t
{
	int16_t X;
	int16_t Y;
	uint32_t Handle;
};

static texture_resource_t CreateTextureResource(graphics_device_t *Device, int16_t X, int16_t Y, const void *Memory, int32_t Flags);
static texture_resource_t CreateTextureResource(graphics_device_t *Device, const surface_t *Surface, int32_t Flags);
static void FreeTextureResource(graphics_device_t *Device, texture_resource_t *Resource);

// ...
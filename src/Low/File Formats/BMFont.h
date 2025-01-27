#pragma pack(push,1)

/*
field	size	type	pos	comment
lineHeight	2	uint	0	
base	2	uint	2	
scaleW	2	uint	4	
scaleH	2	uint	6	
pages	2	uint	8	
bitField	1	bits	10	bits 0-6: reserved, bit 7: packed
alphaChnl	1	uint	11	
redChnl	1	uint	12	
greenChnl	1	uint	13	
blueChnl	1	uint	14	
*/

struct bmfont_common_t
{
	uint16_t LineHeight;
	uint16_t Base;
	uint16_t WScale;
	uint16_t HScale;
	uint16_t Pages;
	int8_t BitField;
	uint8_t AlphaChnl;
	uint8_t ChnlRed;
	uint8_t ChnlGreen;
	uint8_t ChnlBlue;
};

struct bmfont_char_t
{
	uint32_t ID;
	uint16_t X;
	uint16_t Y;
	uint16_t Width;
	uint16_t Height;
	int16_t XOffset;
	int16_t YOffset;
	int16_t XAdvance;
	uint8_t Page;
	uint8_t Chn1;
};

struct bmfont_header_t
{
	uint8_t Signature[3];
	uint8_t Version;
};

struct bmfont_block_t
{
	uint8_t Type;
	int32_t Size;
};

#pragma pack(pop)

struct bmfont_page_t
{
	char FileName[32];
};

struct bmfont_t
{
	int32_t Loaded;
	bmfont_char_t Chars[256];
	bmfont_page_t Pages[1];
	bmfont_common_t Common;
};

static int32_t LoadBMFontFromDirectory(bmfont_t *Font, const char *Directory);
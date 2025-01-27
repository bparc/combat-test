static void ParseBMFontBlock(bmfont_t *Font, bmfont_block_t *Block, byte_stream_t *Stream)
{
	switch (Block->Type)
	{
	case 2: // Common
		{
			bmfont_common_t *Common;
			Common = ReadStruct(Stream, bmfont_common_t);
			if (Common)
			{
				Font->Common = *Common;
			}
		} break;
	case 3: // Pages
		{
			char *FileName = (char *)Stream->Bytes;
			strcpy(Font->Pages[0].FileName, FileName);
		} break;
	case 4: // Chars
		{
			while (!_EOF(Stream))
			{
				bmfont_char_t *Ch = ReadStruct(Stream, bmfont_char_t);
				if (Ch)
				{
					if (Ch->ID < Len(Font->Chars))
					{
						Font->Chars[Ch->ID] = *Ch;
					}

					continue;
				}

				break;
			}
		} break;
	}
}

static int32_t LoadBMFontFromDirectory(bmfont_t *Font, const char *Directory)
{
	file_contents_t Contents = LoadFileFromDirectory(Directory);
	if (Contents.Data)
	{
		byte_stream_t Stream = {};
		Setup(&Stream, &Contents);

		bmfont_header_t *Header = ReadStruct(&Stream, bmfont_header_t);
		if (Header)
		{
			while (!_EOF(&Stream))
			{
				bmfont_block_t *Block = ReadStruct(&Stream, bmfont_block_t);
				if (Block)
				{
					byte_stream_t BlockStream = {};
					Setup(&BlockStream, (uint8_t *)Read(&Stream, Block->Size), Block->Size);
					if (BlockStream.Bytes)
					{
						ParseBMFontBlock(Font, Block, &BlockStream);
						continue;
					}
				}
				break;
			}

			if (_EOF(&Stream))
			{
				Font->Loaded = true;
			}
		}

		FreeFileContents(&Contents);
	}

	return (Font->Loaded);	
}
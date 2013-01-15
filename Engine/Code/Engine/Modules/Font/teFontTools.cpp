/*
 *  teFontTools.cpp
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 12/20/11.
 *  Copyright 2011 Tatem Games. All rights reserved.
 *
 */

#include "teFontTools.h"

namespace te
{
	namespace font
	{
		teFont * LoadFontFNT(core::IBuffer * buffer, f32 downscale)
		{
			buffer->Lock(core::BLT_READ);

			c8 * content = (c8*)TE_ALLOCATE(buffer->GetSize() + 1);
			buffer->SetPosition(0);
			buffer->Read(content, buffer->GetSize());
			content[buffer->GetSize()] = '\0';

			enum EFNTSection
			{
				FNT_NONE,
				FNT_INFO,
				FNT_COMMON,
				FNT_PAGE,
				FNT_CHARS,
				FNT_CHAR,
				FNT_KERNINGS,
				FNT_KERNING
			};

			#define TE_FNT_PARSE_SEPARATORS " \n\r="
			#define TE_FNT_GET currentExp = strtok(NULL, TE_FNT_PARSE_SEPARATORS);\
				if(!currentExp) break;
			#define TE_FNT_ARG(__Name) (strcmp(currentExp, __Name) == 0)

			#define TE_FNT_SECTION_START(__Name, __Section) if(TE_FNT_ARG(__Name)) { currentSection = __Section; continue; }
			#define TE_FNT_SECTION(__Name, __Section) else TE_FNT_SECTION_START(__Name, __Section)
			#define TE_FNT_SECTION_END

			#define TE_FNT_SECTION_CLOSED(__Section) lastSection == __Section && currentSection != __Section

			#define TE_FNT_VALUE_START(__Name, __Value) if(valueSetup) { if(TE_FNT_ARG(__Name)) { currentValue = __Value; valueSetup = false; continue; }
			#define TE_FNT_VALUE(__Name, __Value) else if(TE_FNT_ARG(__Name)) { currentValue = __Value; valueSetup = false; continue; }
			#define TE_FNT_VALUE_END else currentValue = -1; }

			#define TE_FNT_VALUE_CHECK else {
			#define TE_FNT_VALUE_CHECK_END valueSetup = true; }

			#define TE_FNT_VALUE_F32 (((f32)atof(currentExp)) / downscale)
			#define TE_FNT_VALUE_U16 ((u16)TE_FNT_VALUE_F32)
			#define TE_FNT_VALUE_WCHAR ((te_wchar_t)atoi(currentExp))

			// first read all file and determinate metrics and kernings count

			c8 * currentExp = strtok(content, TE_FNT_PARSE_SEPARATORS);
			EFNTSection lastSection = FNT_NONE;
			EFNTSection currentSection = FNT_NONE;
			s32 currentValue = -1;
			u1 valueSetup = true;

			u32 metricsCount = 0;
			u32 kerningsCount = 0;

			while(currentExp != NULL)
			{
				TE_FNT_GET
				
				if(TE_FNT_ARG("char"))
					++metricsCount;
				else if(TE_FNT_ARG("kerning"))
					++kerningsCount;

				TE_FNT_SECTION_START("info", FNT_INFO)
				TE_FNT_SECTION("common", FNT_COMMON)
				TE_FNT_SECTION("page", FNT_PAGE)
				TE_FNT_SECTION("chars", FNT_CHARS)
				TE_FNT_SECTION("char", FNT_CHAR)
				TE_FNT_SECTION("kernings", FNT_KERNINGS)
				TE_FNT_SECTION("kerning", FNT_KERNING)
				TE_FNT_SECTION_END

				switch(currentSection)
				{
				case FNT_INFO: break;
				case FNT_COMMON:
					{
						TE_FNT_VALUE_START("scaleW",	0)
						TE_FNT_VALUE("scaleH",			1)
						TE_FNT_VALUE("base",			2)
						TE_FNT_VALUE_END
						TE_FNT_VALUE_CHECK
						TE_FNT_VALUE_CHECK_END
						break;
					}
				case FNT_PAGE: break;
				case FNT_CHARS: break;
				case FNT_CHAR:
					{
						TE_FNT_VALUE_START("id",	0)
						TE_FNT_VALUE("x",			1)
						TE_FNT_VALUE("y",			2)
						TE_FNT_VALUE("width",		3)
						TE_FNT_VALUE("height",		4)
						TE_FNT_VALUE("xoffset",		5)
						TE_FNT_VALUE("yoffset",		6)
						TE_FNT_VALUE("xadvance",	7)
						TE_FNT_VALUE_END
						TE_FNT_VALUE_CHECK
						TE_FNT_VALUE_CHECK_END
						break;
					}
				case FNT_KERNINGS: break;
				case FNT_KERNING:
					{
						TE_FNT_VALUE_START("first",	0)
						TE_FNT_VALUE("second",		1)
						TE_FNT_VALUE("amount",		2)
						TE_FNT_VALUE_END
						TE_FNT_VALUE_CHECK
						TE_FNT_VALUE_CHECK_END
						break;
					}
				case FNT_NONE:
				default:
					break;
				}

				lastSection = currentSection;
			}

			// read font structure data from file

			teFont * font = reinterpret_cast<teFont*>(TE_ALLOCATE(sizeof(teFont) + metricsCount * sizeof(teCharacterMetrics) + kerningsCount * sizeof(teCharacterKerning)));
			font->materialIndex = u32Max;
			font->metricsCount = metricsCount;
			font->kerningsCount = kerningsCount;

			buffer->SetPosition(0);
			buffer->Read(content, buffer->GetSize());
			content[buffer->GetSize()] = '\0';
			buffer->Unlock();
			buffer = NULL;

			currentExp = strtok(content, TE_FNT_PARSE_SEPARATORS);
			lastSection = FNT_NONE;
			currentSection = FNT_NONE;
			currentValue = -1;
			valueSetup = true;

			teVector2df textureSize;
			teCharacterMetrics metric;
			wchar_t kerningCharLeft, kerningCharRight;
			f32 kerning;

			u32 currentMetric = 0;
			u32 currentKerning = 0;
			
			u1 charModified = false;
			u1 kerningModified = false;

			while(currentExp != NULL)
			{
				TE_FNT_GET
				
				u1 needSaveChar = false;
				u1 needSaveKerning = false;
				
				if(TE_FNT_ARG("char") && charModified)
					needSaveChar = true;
				else if(TE_FNT_ARG("kerning") && kerningModified)
					needSaveKerning = true;

				TE_FNT_SECTION_START("info", FNT_INFO)
				TE_FNT_SECTION("common", FNT_COMMON)
				TE_FNT_SECTION("page", FNT_PAGE)
				TE_FNT_SECTION("chars", FNT_CHARS)
				TE_FNT_SECTION("char", FNT_CHAR)
				TE_FNT_SECTION("kernings", FNT_KERNINGS)
				TE_FNT_SECTION("kerning", FNT_KERNING)
				TE_FNT_SECTION_END

				if(TE_FNT_SECTION_CLOSED(FNT_CHAR))
					needSaveChar = true;
				else if(TE_FNT_SECTION_CLOSED(FNT_KERNING))
					needSaveKerning = true;
				
				if(needSaveChar)
				{
					teVector2df position = metric.uv1;
					metric.uv1 = ((position /*+ 0.5f*/) / (textureSize )) * ((f32)u16Max - 1.0f);
					metric.uv2 = ((position /*+ 0.5f*/ + metric.size) / (textureSize )) * ((f32)u16Max - 1.0f);
					font->GetMetric(currentMetric++) = metric;
					charModified = false;
					needSaveChar = false;
				}
				else if(needSaveKerning)
				{
					teCharacterKerning temp;
					temp.characterLeft = kerningCharLeft;
					temp.characterRight = kerningCharRight;
					temp.kerning = kerning;
					font->GetKerning(currentKerning++) = temp;
					kerningModified = false;
					needSaveKerning = false;
				}

				switch(currentSection)
				{
				case FNT_INFO: break;
				case FNT_COMMON:
					{
						TE_FNT_VALUE_START("scaleW",	0)
						TE_FNT_VALUE("scaleH",			1)
						TE_FNT_VALUE("base",			2)
						TE_FNT_VALUE_END

						TE_FNT_VALUE_CHECK
						switch(currentValue)
						{
						case 0: textureSize.x = TE_FNT_VALUE_U16; break;
						case 1: textureSize.y = TE_FNT_VALUE_U16; break;
						case 2: font->height = TE_FNT_VALUE_F32; break;
						default: break;
						}
						TE_FNT_VALUE_CHECK_END
						break;
					}
				case FNT_PAGE: break;
				case FNT_CHARS: break;
				case FNT_CHAR:
					{
						TE_FNT_VALUE_START("id",	0)
						TE_FNT_VALUE("x",			1)
						TE_FNT_VALUE("y",			2)
						TE_FNT_VALUE("width",		3)
						TE_FNT_VALUE("height",		4)
						TE_FNT_VALUE("xoffset",		5)
						TE_FNT_VALUE("yoffset",		6)
						TE_FNT_VALUE("xadvance",	7)
						TE_FNT_VALUE_END

						TE_FNT_VALUE_CHECK
						switch(currentValue)
						{
						case 0: metric.character = (u32)atol(currentExp); charModified = true; break;
						case 1: metric.uv1.x =   TE_FNT_VALUE_F32; charModified = true; break;
						case 2: metric.uv1.y =   TE_FNT_VALUE_F32; charModified = true; break;
						case 3: metric.size.x =  TE_FNT_VALUE_F32; charModified = true; break;
						case 4: metric.size.y =  TE_FNT_VALUE_F32; charModified = true; break;
						case 5: metric.shift.x = TE_FNT_VALUE_F32; charModified = true; break;
						case 6: metric.shift.y = TE_FNT_VALUE_F32; charModified = true; break;
						case 7: metric.shiftCharacter.x = TE_FNT_VALUE_F32; metric.shiftCharacter.y = 0; charModified = true; break;
						default: break;
						}
						TE_FNT_VALUE_CHECK_END
						break;
					}
				case FNT_KERNINGS: break;
				case FNT_KERNING:
					{
						TE_FNT_VALUE_START("first",	0)
						TE_FNT_VALUE("second",		1)
						TE_FNT_VALUE("amount",		2)
						TE_FNT_VALUE_END

						TE_FNT_VALUE_CHECK
						switch(currentValue)
						{
						case 0: kerningCharLeft = (u32)atol(currentExp); kerningModified = true; break;
						case 1: kerningCharRight = (u32)atol(currentExp); kerningModified = true; break;
						case 2: kerning = TE_FNT_VALUE_F32; kerningModified = true; break;
						default: break;
						}
						TE_FNT_VALUE_CHECK_END
						break;
					}
				case FNT_NONE:
				default:
					break;
				}

				lastSection = currentSection;
			}
			
			if(charModified)
			{
				teVector2df position = metric.uv1;
				metric.uv1 = ((position + 0.5f) / textureSize) * ((f32)u16Max - 1.0f);
				metric.uv2 = ((position + 0.5f + metric.size) / textureSize) * ((f32)u16Max - 1.0f);
				font->GetMetric(currentMetric++) = metric;
				charModified = false;
			}
			else if(kerningModified)
			{
				teCharacterKerning temp;
				temp.characterLeft = kerningCharLeft;
				temp.characterRight = kerningCharRight;
				temp.kerning = kerning;
				font->GetKerning(currentKerning++) = temp;
				kerningModified = false;
			}
			
			TE_FREE(content);

			// TODO add sorting by character for metrics
			// TODO add sorting by character for kernings

			return font;
		}
	}
}

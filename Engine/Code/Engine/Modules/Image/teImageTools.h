/*
 *  teImageTools.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 01/20/12.
 *  Copyright 2012 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TEIMAGETOOLS_H
#define TE_TEIMAGETOOLS_H

#include "teImage.h"
#include "IBuffer.h"

namespace te
{
	namespace image
	{
		//! @brief		Load TGA image
		//! @details	Use TE_ALLOCATE for memory allocation for image, dont swap channels so BGRA image becomes RGBA.
		//! @param[in] buffer buffer with image data
		//! @return teImage* loaded image, return NULL if failed
		teImage * LoadTGA(core::IBuffer * buffer);

		/*
		//! @brief		Load DDS image
		//! @details	Use TE_ALLOCATE for memory allocation for image.
		//! @param[in] buffer buffer with image data
		//! @return teImage* loaded image, return NULL if failed
		teImage * LoadDDS(core::IBuffer * buffer);
		*/

		//! @brief		Load PVR image
		//! @details	Use TE_ALLOCATE for memory allocation for image.
		//! @param[in] buffer buffer with image data
		//! @return teImage* loaded image, return NULL if failed
		teImage * LoadPVR(core::IBuffer * buffer);

		//! @brief		Load PNG image
		//! @details	Use TE_ALLOCATE for memory allocation for image.
		//! @param[in] buffer buffer with image data
		//! @return teImage* loaded image, return NULL if failed
		teImage * LoadPNG(core::IBuffer * buffer);

		//! @brief		Save TGA image
		//! @details	Save image as is, dont swap color channels, so image becomes BGRA, support only 24 and 32 bit images.
		//! @param[out] buffer buffer for image save
		//! @param[in] image image for saving
		void SaveTGA(core::IBuffer * buffer, teImage * image); // wip
	}
}

#endif
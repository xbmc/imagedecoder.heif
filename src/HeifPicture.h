/*
 *  Copyright (C) 2005-2021 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include <kodi/addon-instance/ImageDecoder.h>
#include <libheif/heif.h>

class ATTR_DLL_LOCAL HeifPicture : public kodi::addon::CInstanceImageDecoder
{
public:
  HeifPicture(KODI_HANDLE instance, const std::string& version);
  ~HeifPicture() override;

  bool SupportsFile(const std::string& file) override;
  bool ReadTag(const std::string& file, kodi::addon::ImageDecoderInfoTag& tag) override;
  bool LoadImageFromMemory(const std::string& mimetype,
                           const uint8_t* buffer,
                           size_t bufSize,
                           unsigned int& width,
                           unsigned int& height) override;
  bool Decode(uint8_t* pixels,
              unsigned int width,
              unsigned int height,
              unsigned int pitch,
              ADDON_IMG_FMT format) override;

private:
  heif_context* m_heifCtx = nullptr;
};

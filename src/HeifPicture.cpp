/*
 *      Copyright (C) 2005-2013 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include <libheif/heif.h>
#include <stdint.h>

#include <kodi/addon-instance/ImageDecoder.h>

class HeifPicture : public kodi::addon::CInstanceImageDecoder
{
public:
  HeifPicture(KODI_HANDLE instance)
    : CInstanceImageDecoder(instance)
  {
    ctx = heif_context_alloc();
  }

  virtual ~HeifPicture()
  {
    if (ctx)
      heif_context_free(ctx);

    ctx = nullptr;
  }

  bool LoadImageFromMemory(unsigned char* buffer, unsigned int bufSize,
                           unsigned int& width, unsigned int& height) override
  {
    struct heif_error error = heif_context_read_from_memory(ctx, buffer, bufSize, nullptr);
    if (error.code != heif_error_Ok)
    {
      kodi::Log(ADDON_LOG_ERROR, "%s", error.message);
      return false;
    }
    heif_image_handle* handle;
    heif_context_get_primary_image_handle(ctx, &handle);
    width = heif_image_handle_get_width(handle);
    height = heif_image_handle_get_height(handle);
    return true;
  }

  bool Decode(unsigned char *pixels,
              unsigned int width, unsigned int height,
              unsigned int pitch, ImageFormat format) override
  {
    heif_image_handle* handle;
    heif_context_get_primary_image_handle(ctx, &handle);
    heif_image* img;
    heif_chroma fmt = heif_chroma_interleaved_24bit;
    struct heif_error error = heif_decode_image(handle, &img,
                                                heif_colorspace_RGB, fmt, nullptr);

    if (error.code != heif_error_Ok)
    {
      kodi::Log(ADDON_LOG_ERROR, "%s", error.message);
      return false;
    }

    int stride;
    const uint8_t* data = heif_image_get_plane_readonly(img, heif_channel_interleaved, &stride);
    if (!data)
      return false;

    for (size_t i = 0; i < height; ++i)
    {
      const uint8_t* src = data + i*stride;
      uint8_t* dst = pixels + i*pitch;
      for (size_t j = 0; j < width; ++j, src += 3)
      {
        for (size_t k = 0; k < 3; ++k)
          *dst++ = *(src+2-k);
        if (format == ADDON_IMG_FMT_A8R8G8B8)
          *dst++ = 255;
      }
    }

    return true;
  }

private:
  heif_context* ctx = nullptr;
};

class ATTRIBUTE_HIDDEN CMyAddon : public kodi::addon::CAddonBase
{
public:
  CMyAddon() { }
  ADDON_STATUS CreateInstance(int instanceType, std::string instanceID, KODI_HANDLE instance, KODI_HANDLE& addonInstance) override
  {
    if (instanceType == ADDON_INSTANCE_IMAGEDECODER)
    {
      addonInstance = new HeifPicture(instance);
      return ADDON_STATUS_OK;
    }

    return ADDON_STATUS_NOT_IMPLEMENTED;
  }
};

ADDONCREATOR(CMyAddon);

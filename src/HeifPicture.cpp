/*
 *  Copyright (C) 2005-2021 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "HeifPicture.h"

#include "TinyEXIF/TinyEXIF.h"

#include <kodi/Filesystem.h>

HeifPicture::HeifPicture(const kodi::addon::IInstanceInfo& instance)
  : CInstanceImageDecoder(instance), m_heifCtx(heif_context_alloc())
{
}

HeifPicture::~HeifPicture()
{
  heif_context_free(m_heifCtx);
}

bool HeifPicture::SupportsFile(const std::string& file)
{
  kodi::vfs::CFile fileData;
  if (!fileData.OpenFile(file))
    return false;

  uint8_t magic[12];
  fileData.Read(magic, sizeof(magic));

  enum heif_filetype_result filetype_check = heif_check_filetype(magic, sizeof(magic));
  if (filetype_check == heif_filetype_no || filetype_check == heif_filetype_yes_unsupported)
    return false;

  return true;
}

bool HeifPicture::ReadTag(const std::string& file, kodi::addon::ImageDecoderInfoTag& tag)
{
  kodi::vfs::CFile fileData;
  if (!fileData.OpenFile(file))
    return false;

  const size_t length = fileData.GetLength();

  std::vector<uint8_t> buffer(length);
  fileData.Read(buffer.data(), buffer.size());

  struct heif_error error =
      heif_context_read_from_memory(m_heifCtx, buffer.data(), length, nullptr);
  if (error.code != heif_error_Ok)
  {
    kodi::Log(ADDON_LOG_ERROR, "%s: Read error '%s'", __func__, error.message);
    return false;
  }
  heif_image_handle* handle{nullptr};
  heif_context_get_primary_image_handle(m_heifCtx, &handle);

  tag.SetWidth(heif_image_handle_get_width(handle));
  tag.SetHeight(heif_image_handle_get_height(handle));

  int metadata_count = heif_image_handle_get_number_of_metadata_blocks(handle, nullptr);
  heif_item_id* metadata_ids =
      static_cast<heif_item_id*>(malloc(metadata_count * sizeof(heif_item_id)));
  int metadata_ids_count = heif_image_handle_get_list_of_metadata_block_IDs(
      handle, nullptr, metadata_ids, metadata_count);
  for (int i = 0; i < metadata_count; i++)
  {
    const char* type = heif_image_handle_get_metadata_type(handle, metadata_ids[i]);

    if (type && strcmp(type, "Exif") == 0)
    {
      size_t metadata_size = heif_image_handle_get_metadata_size(handle, metadata_ids[i]);
      uint8_t* metadata = static_cast<uint8_t*>(malloc(metadata_size));
      heif_image_handle_get_metadata(handle, metadata_ids[i], metadata);

      TinyEXIF::EXIFInfo imageEXIF;

      imageEXIF.parseFromEXIFSegment(metadata + 4, metadata_size);

      switch (imageEXIF.Orientation)
      {
        case 3:
          tag.SetOrientation(ADDON_IMG_ORIENTATION_ROTATE_180_CCW);
          break;
        case 5:
          tag.SetOrientation(ADDON_IMG_ORIENTATION_ROTATE_90_CCW);
          break;
        case 6:
          tag.SetOrientation(ADDON_IMG_ORIENTATION_ROTATE_270_CCW);
          break;
        default:
          tag.SetOrientation(ADDON_IMG_ORIENTATION_NONE);
          break;
      }

      std::string dateTime;
      if (!imageEXIF.DateTimeOriginal.empty())
        dateTime = imageEXIF.DateTimeOriginal;
      else if (!imageEXIF.DateTime.empty())
        dateTime = imageEXIF.DateTime;
      else if (!imageEXIF.DateTimeDigitized.empty())
        dateTime = imageEXIF.DateTimeDigitized;
      if (dateTime.size() == 19)
      {
        struct tm tm = {0};
        tm.tm_year = atoi(dateTime.substr(0, 4).c_str()) - 1900;
        tm.tm_mon = atoi(dateTime.substr(5, 2).c_str()) - 1;
        tm.tm_mday = atoi(dateTime.substr(8, 2).c_str());
        tm.tm_hour = atoi(dateTime.substr(11, 2).c_str());
        tm.tm_min = atoi(dateTime.substr(14, 2).c_str());
        tm.tm_sec = atoi(dateTime.substr(17, 2).c_str());
        tm.tm_isdst = -1; // Assume local daylight setting per date/time
        tag.SetTimeCreated(mktime(&tm));
      }
      tag.SetDistance(imageEXIF.SubjectDistance);
      tag.SetISOSpeed(imageEXIF.ISOSpeedRatings);
      tag.SetFocalLength(imageEXIF.FocalLength);
      tag.SetFocalLengthIn35mmFormat(imageEXIF.LensInfo.FocalLengthIn35mm);
      tag.SetCameraManufacturer(imageEXIF.Make);
      tag.SetCameraModel(imageEXIF.Model);
      tag.SetExposureBias(imageEXIF.ExposureBiasValue);
      tag.SetExposureTime(imageEXIF.ExposureTime);
      tag.SetExposureProgram(static_cast<ADDON_IMG_EXPOSURE_PROGRAM>(imageEXIF.ExposureProgram));
      tag.SetMeteringMode(static_cast<ADDON_IMG_METERING_MODE>(imageEXIF.MeteringMode));
      tag.SetApertureFNumber(imageEXIF.FNumber);
      tag.SetFlashUsed(static_cast<ADDON_IMG_FLASH_TYPE>(imageEXIF.Flash));
      tag.SetLightSource(static_cast<ADDON_IMG_LIGHT_SOURCE>(imageEXIF.LightSource));
      tag.SetDescription(imageEXIF.ImageDescription);
      tag.SetDigitalZoomRatio(imageEXIF.LensInfo.DigitalZoomRatio);

      if (imageEXIF.GeoLocation.hasLatLon() && imageEXIF.GeoLocation.hasAltitude() &&
          isalpha(imageEXIF.GeoLocation.LatComponents.direction))
      {
        float lat[] = {static_cast<float>(imageEXIF.GeoLocation.LatComponents.degrees),
                       static_cast<float>(imageEXIF.GeoLocation.LatComponents.minutes),
                       static_cast<float>(imageEXIF.GeoLocation.LatComponents.seconds)};

        float lon[] = {static_cast<float>(imageEXIF.GeoLocation.LonComponents.degrees),
                       static_cast<float>(imageEXIF.GeoLocation.LonComponents.minutes),
                       static_cast<float>(imageEXIF.GeoLocation.LonComponents.seconds)};
        tag.SetGPSInfo(true, imageEXIF.GeoLocation.LatComponents.direction, lat,
                       imageEXIF.GeoLocation.LonComponents.direction, lon,
                       imageEXIF.GeoLocation.AltitudeRef, imageEXIF.GeoLocation.Altitude);
      }

      free(metadata);
    }
  }

  return true;
}

bool HeifPicture::LoadImageFromMemory(const std::string& mimetype,
                                      const uint8_t* buffer,
                                      size_t bufSize,
                                      unsigned int& width,
                                      unsigned int& height)
{
  struct heif_error error = heif_context_read_from_memory(m_heifCtx, buffer, bufSize, nullptr);
  if (error.code != heif_error_Ok)
  {
    kodi::Log(ADDON_LOG_ERROR, "%s: Read error '%s'", __func__, error.message);
    return false;
  }
  heif_image_handle* handle;
  heif_context_get_primary_image_handle(m_heifCtx, &handle);
  width = heif_image_handle_get_width(handle);
  height = heif_image_handle_get_height(handle);
  return true;
}

bool HeifPicture::Decode(uint8_t* pixels,
                         unsigned int width,
                         unsigned int height,
                         unsigned int pitch,
                         ADDON_IMG_FMT format)
{
  heif_image_handle* handle;
  heif_context_get_primary_image_handle(m_heifCtx, &handle);
  heif_image* img;
  heif_chroma fmt = heif_chroma_interleaved_24bit;
  struct heif_error error = heif_decode_image(handle, &img, heif_colorspace_RGB, fmt, nullptr);

  if (error.code != heif_error_Ok)
  {
    kodi::Log(ADDON_LOG_ERROR, "%s: Decode error '%s'", __func__, error.message);
    return false;
  }

  int stride;
  const uint8_t* data = heif_image_get_plane_readonly(img, heif_channel_interleaved, &stride);
  if (!data)
    return false;

  for (size_t i = 0; i < height; ++i)
  {
    const uint8_t* src = data + i * stride;
    uint8_t* dst = pixels + i * pitch;
    for (size_t j = 0; j < width; ++j, src += 3)
    {
      for (size_t k = 0; k < 3; ++k)
        *dst++ = *(src + 2 - k);
      if (format == ADDON_IMG_FMT_A8R8G8B8)
        *dst++ = 255;
    }
  }

  return true;
}

class ATTR_DLL_LOCAL CMyAddon : public kodi::addon::CAddonBase
{
public:
  CMyAddon() = default;
  ADDON_STATUS CreateInstance(const kodi::addon::IInstanceInfo& instance,
                              KODI_ADDON_INSTANCE_HDL& hdl) override
  {
    if (instance.IsType(ADDON_INSTANCE_IMAGEDECODER))
    {
      hdl = new HeifPicture(instance);
      return ADDON_STATUS_OK;
    }

    return ADDON_STATUS_NOT_IMPLEMENTED;
  }
};

ADDONCREATOR(CMyAddon)

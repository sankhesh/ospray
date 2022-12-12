// Copyright 2009 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
#ifdef OSPRAY_ENABLE_VOLUMES

#pragma once

#include "Volume.h"
#include "openvkl/openvkl.h"
// comment break to prevent clang-format from reordering openvkl includes
#include "openvkl/device/openvkl.h"
// ispc shared
#include "volume/VolumetricModelShared.h"

namespace ospray {

struct OSPRAY_SDK_INTERFACE VolumetricModel
    : public AddStructShared<ISPCDeviceObject, ispc::VolumetricModel>
{
  VolumetricModel(api::ISPCDevice &device, Volume *geometry);
  ~VolumetricModel() override;
  std::string toString() const override;

  void commit() override;

  RTCGeometry embreeGeometryHandle() const;

  box3f bounds() const;

  Ref<Volume> getVolume() const;

 private:
  box3f volumeBounds;
  Ref<Volume> volume;
  const Ref<Volume> volumeAPI;
  // TODO: need to remove/not use for the SYCL case
  VKLIntervalIteratorContext vklIntervalContext{nullptr, nullptr};
};

OSPTYPEFOR_SPECIALIZATION(VolumetricModel *, OSP_VOLUMETRIC_MODEL);

} // namespace ospray
#endif

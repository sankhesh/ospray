// Copyright 2009 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
#ifdef OSPRAY_ENABLE_VOLUMES

// ospray
#include "VolumetricModel.h"
#include "transferFunction/TransferFunction.h"

namespace ospray {

VolumetricModel::VolumetricModel(api::ISPCDevice &device, Volume *_volume)
    : AddStructShared(device.getIspcrtDevice(), device), volumeAPI(_volume)
{
  managedObjectType = OSP_VOLUMETRIC_MODEL;
}

VolumetricModel::~VolumetricModel()
{
  if (vklIntervalContext.host)
    vklRelease2(vklIntervalContext);
}

std::string VolumetricModel::toString() const
{
  return "ospray::VolumetricModel";
}

void VolumetricModel::commit()
{
  volume = hasParam("volume") ? (Volume *)getParamObject("volume") : volumeAPI;
  if (!volume)
    throw std::runtime_error(toString() + " received NULL 'volume'");

  auto *transferFunction =
      (TransferFunction *)getParamObject("transferFunction", nullptr);

  if (transferFunction == nullptr)
    throw std::runtime_error(toString() + " must have 'transferFunction'");

  // create value selector using transfer function and pass to volume
  if (volume->vklVolume.host) {
    if (vklIntervalContext.host) {
      vklRelease2(vklIntervalContext);
      vklIntervalContext.host = nullptr;
      vklIntervalContext.device = nullptr;
    }

    // TODO: For SYCL we won't have the interval context for now, because
    // they're not on the GPU yet
    vklIntervalContext = vklNewIntervalIteratorContext(volume->vklSampler);
    std::vector<range1f> valueRanges =
        transferFunction->getPositiveOpacityValueRanges();
    if (valueRanges.empty()) {
      // volume made completely transparent and could be removed, which is
      // awkward here
      // set an "empty" interesting value range instead, which will lead to a
      // quick out during volume iteration
      valueRanges.push_back(range1f(neg_inf, neg_inf));
    }
    VKLData valueRangeData = vklNewData(getISPCDevice().getVklDevice(),
        valueRanges.size(),
        VKL_BOX1F,
        valueRanges.data());
    vklSetData2(vklIntervalContext, "valueRanges", valueRangeData);
    vklRelease(valueRangeData);
    vklCommit2(vklIntervalContext);

    // Pass interval context to ISPC
    getSh()->vklIntervalContext = vklIntervalContext;
  }

  // Finish getting/setting other appearance information
  volumeBounds = volume->bounds;

  // Initialize shared structure
  getSh()->volume = getVolume()->getSh();
  getSh()->transferFunction = transferFunction->getSh();
  getSh()->boundingBox = volumeBounds;
  getSh()->densityScale = getParam<float>("densityScale", 1.f);
  getSh()->anisotropy = getParam<float>("anisotropy", 0.f);
  getSh()->gradientShadingScale = getParam<float>("gradientShadingScale", 0.f);
  getSh()->userID = getParam<uint32>("id", RTC_INVALID_GEOMETRY_ID);
}

RTCGeometry VolumetricModel::embreeGeometryHandle() const
{
  return volume->embreeGeometry;
}

box3f VolumetricModel::bounds() const
{
  return volumeBounds;
}

Ref<Volume> VolumetricModel::getVolume() const
{
  return volume;
}

OSPTYPEFOR_DEFINITION(VolumetricModel *);

} // namespace ospray
#endif

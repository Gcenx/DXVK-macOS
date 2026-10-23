#pragma once

#include <cstdint>

#include "dxso_common.h"
#include "dxso_decoder.h"

#include "../d3d9/d3d9_caps.h"

namespace dxvk {

  enum class DxsoBindingType : uint32_t {
    ConstantBuffer,
    Image,
  };

  enum class DxsoConstantBufferType : uint32_t {
    Float,
    Int,
    Bool
  };

  enum DxsoConstantBuffers : uint32_t {
    VSConstantBuffer = 0,
    VSFloatConstantBuffer = 0,
    VSIntConstantBuffer = 1,
    VSBoolConstantBuffer = 2,
    VSClipPlanes     = 3,
    VSFixedFunction  = 4,
    VSVertexBlendData = 5,
    VSCount,

    PSConstantBuffer = 0,
    PSFixedFunction  = 1,
    PSShared         = 2,
    PSCount
  };

  constexpr uint32_t computeResourceSlotId(
        DxsoProgramType shaderStage,
        DxsoBindingType bindingType,
        uint32_t        bindingIndex) {
    const uint32_t stageOffset = (DxsoConstantBuffers::VSCount + caps::MaxTexturesVS) * uint32_t(shaderStage);

    if (bindingType == DxsoBindingType::ConstantBuffer)
      return bindingIndex + stageOffset;
    else // if (bindingType == DxsoBindingType::Image)
      return bindingIndex + stageOffset + (shaderStage == DxsoProgramType::PixelShader ? DxsoConstantBuffers::PSCount : DxsoConstantBuffers::VSCount);
  }

  // TODO: Intergrate into compute resource slot ID/refactor all of this?
  constexpr uint32_t getSWVPBufferSlot() {
    return DxsoConstantBuffers::VSCount + caps::MaxTexturesVS + DxsoConstantBuffers::PSCount + caps::MaxTexturesPS + 1; // From last pixel shader slot, above.
  }

  // Number of additional slots per sampler. A sampler can be declared as
  // up to five image variables (2D, 2D shadow, cube, cube shadow, 3D).
  // Vulkan allows them to alias one binding, but MoltenVK maps every
  // variable of a binding to the same Metal index, which Metal rejects.
  // Shaders therefore use a separate sampler descriptor in the regular
  // slot, and one sampled image descriptor per variant in these slots,
  // which also keeps the sampler count within Metal's limit of 16.
  constexpr uint32_t SamplerAliasSlotCount = 5;

  constexpr uint32_t computeSamplerAliasSlotId(
        DxsoProgramType shaderStage,
        uint32_t        samplerIndex,
        uint32_t        aliasIndex) {
    const uint32_t samplerId = shaderStage == DxsoProgramType::PixelShader
      ? samplerIndex
      : samplerIndex + caps::MaxTexturesPS + 1; // PS has one extra (DMAP) sampler, see getSWVPBufferSlot

    return getSWVPBufferSlot() + 1 + samplerId * SamplerAliasSlotCount + aliasIndex;
  }

  uint32_t RegisterLinkerSlot(DxsoSemantic semantic);

}
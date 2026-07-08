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

  // De-aliased sampler bindings (d3d9.deAliasedSamplers): drivers such as
  // MoltenVK cannot express multiple differently-typed image variables
  // aliased at one binding slot, so each d3d9 sampler is given a block of
  // SamplerVariantCount consecutive slots, one per texture-type variant.
  // computeResourceSlotId() returns the FIRST slot of the block; add the
  // variant index for a specific one. The slot layout always reserves the
  // full block so that both modes share one layout.
  // Variant order for color matches DxsoSamplerType: 2D=0, 3D=1, Cube=2.
  constexpr uint32_t SamplerVariantCount     = 5;
  constexpr uint32_t SamplerVariantDepth2D   = 3;
  constexpr uint32_t SamplerVariantDepthCube = 4;

  constexpr uint32_t samplerTypeVariant(uint32_t samplerType, bool depth) {
    return !depth ? samplerType
      : (samplerType == 2u /* SamplerTypeTextureCube */ ? SamplerVariantDepthCube : SamplerVariantDepth2D);
  }

  constexpr uint32_t computeResourceSlotId(
        DxsoProgramType shaderStage,
        DxsoBindingType bindingType,
        uint32_t        bindingIndex) {
    const uint32_t stageOffset = (DxsoConstantBuffers::VSCount + caps::MaxTexturesVS * SamplerVariantCount) * uint32_t(shaderStage);

    if (bindingType == DxsoBindingType::ConstantBuffer)
      return bindingIndex + stageOffset;
    else // if (bindingType == DxsoBindingType::Image)
      return bindingIndex * SamplerVariantCount + stageOffset + (shaderStage == DxsoProgramType::PixelShader ? DxsoConstantBuffers::PSCount : DxsoConstantBuffers::VSCount);
  }

  // TODO: Intergrate into compute resource slot ID/refactor all of this?
  constexpr uint32_t getSWVPBufferSlot() {
    return DxsoConstantBuffers::VSCount + caps::MaxTexturesVS * SamplerVariantCount + DxsoConstantBuffers::PSCount + caps::MaxTexturesPS * SamplerVariantCount + 1; // From last pixel shader slot, above.
  }

  uint32_t RegisterLinkerSlot(DxsoSemantic semantic);

}
//===-- nes_emu/Device/Sram.h - Sram class declaration ----------*- C++ -*-===//
//
// This file is distributed under the Boost Software License. See LICENSE.TXT
// for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains the declaration of the SRAM class, which is emulate the
/// memmapped SRAM device.
///
//===----------------------------------------------------------------------===//

#ifndef NES_EMU_DEVICE_SRAM_H
#define NES_EMU_DEVICE_SRAM_H

//==============================================================================
//= Dependencies
//==============================================================================
// Local/Private Headers
#include "nes_emu/Bus.h"
#include "nes_emu/Device/MemoryMappedDevice.h"

// External headers

// System headers
#include <array>

namespace nes_emu {
template <size_t N> class Sram : public MemoryMappedDevice {
public:
  std::optional<std::errc> map(Bus16 *bus, uint_fast16_t address) override {
    return bus->mapMemory(this, address, N, this->mem_.data());
  }
  constexpr uint8_t *data() noexcept { return this->mem_.data(); }
  constexpr size_t size() const noexcept { return N; }

private:
  std::array<uint8_t, N> mem_{};
};
} // namespace nes_emu

#endif // NES_EMU_DEVICE_SRAM_H

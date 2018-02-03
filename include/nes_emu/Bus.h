//===-- nes_emu/Bus.h - Bus class declaration -------------------*- C++ -*-===//
//
// This file is distributed under the Boost Software License. See LICENSE.TXT
// for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains the declaration of the Bus class, which is emulate the
/// system bus.
///
//===----------------------------------------------------------------------===//

#ifndef NES_EMU_BUS_H
#define NES_EMU_BUS_H

//==============================================================================
//= Dependencies
//==============================================================================
// Local/Private Headers

// External headers

// System headers
#include <cstddef>      // size_t
#include <cstdint>      // uint_fast16_t
#include <memory>       // unique_ptr
#include <optional>     // optional
#include <system_error> // errc

namespace nes_emu {
class Device;

enum class BusAccessKind { kNone, kRead, kWrite };

template <size_t address_bits> class Bus {

public:
  using AddressType = uint_fast16_t;
  using ErrorCallback = void (*)(AddressType addr, BusAccessKind op);
  explicit Bus(ErrorCallback cb) noexcept
      : notify_error_(cb), map_table_{nullptr} {}
  ~Bus() noexcept = default;
  // disallow copy
  Bus(const Bus &) = delete;
  Bus &operator=(const Bus &) = delete;
  // allow move
  Bus(Bus &&) noexcept = default;
  Bus &operator=(Bus &&) noexcept = default;

  std::optional<std::errc> mapMemory(Device *dev, AddressType address,
                                     size_t bytes, void *mem);
  void read(AddressType address, size_t bytes, void *buffer) const;
  uint8_t read8(AddressType address);
  uint16_t read16(AddressType address);
  uint32_t read32(AddressType address);
  uint64_t read64(AddressType address);
  void write(const void *buffer, size_t bytes, AddressType destination);
  void write8(AddressType address, const uint8_t &value);
  void write16(AddressType address, const uint16_t &value);
  void write32(AddressType address, const uint32_t &value);
  void write64(AddressType address, const uint64_t &value);
  void dumpMap() const;

private:
  struct Map {
    explicit Map(Device *owner, void *memory, AddressType address, size_t bytes)
        : Owner(owner), Memory(static_cast<uint8_t *>(memory)),
          Address(address), Bytes(bytes) {}
    Device *Owner;
    uint8_t *Memory;
    AddressType Address;
    size_t Bytes;
  };
  static constexpr auto kAddressBits = address_bits;
  static constexpr auto kPageSizeBits = 10;
  static constexpr AddressType kPageSize = 1 << kPageSizeBits;
  static constexpr std::uintptr_t kPageMask = kPageSize - 1;
  ErrorCallback notify_error_;
  std::unique_ptr<Map> map_table_[1ULL << (kAddressBits - kPageSizeBits)];
};

using Bus16 = Bus<16>;

} // namespace nes_emu

#endif // NES_EMU_BUS_H

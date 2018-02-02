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
#include "nes_emu/Device.h"

// External headers

// System headers
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>
#include <optional>
#include <system_error>

namespace nes_emu {
class Map {
public:
  Map(Device *owner, uint8_t *memory, uint_fast16_t address, size_t bytes)
      : owner_(owner), memory_(memory), address_(address), bytes_(bytes) {}
  Device *owner_;
  uint8_t *memory_;
  uint_fast16_t address_;
  size_t bytes_;
};

enum class BusAccessKind { None, Read, Write };

class Bus16 {
public:
  using ErrorCallback = void (*)(uint_fast16_t addr, BusAccessKind op);
  explicit Bus16(ErrorCallback cb) noexcept
      : notify_error_(cb), map_table_{nullptr} {}
  ~Bus16() noexcept = default;
  // disallow copy
  Bus16(const Bus16 &) = delete;
  Bus16 &operator=(const Bus16 &) = delete;
  // allow move
  Bus16(Bus16 &&) noexcept = default;
  Bus16 &operator=(Bus16 &&) noexcept = default;

  std::optional<std::errc> mapMemory(Device *dev, uint_fast16_t address,
                                     size_t bytes, void *mem) {
    auto page_begin = address >> page_size_bits;
    auto page_end = (address + bytes - 1) >> page_size_bits;
    auto mem_addr = static_cast<uint8_t *>(mem);
    // chack already exists
    // TODO(tenmyo): extract function
    for (auto page = page_begin; page <= page_end; ++page) {
      if (this->map_table_[page]) {
        return std::errc::file_exists;
      }
    }
    // map
    auto offset = address - (page_begin << page_size_bits);
    for (auto page = page_begin; page <= page_end; ++page) {
      this->map_table_[page] = std::make_unique<Map>(
          dev, mem_addr, (page << page_size_bits) + offset,
          std::min(bytes, this->page_size));
      bytes -= this->page_size;
      mem_addr += this->page_size;
      offset = 0;
    }
    return std::nullopt;
  }

  void read(uint_fast16_t address, uint_fast16_t bytes, uint8_t *buffer) {
    decltype(bytes) readed_bytes = 0;
    while (readed_bytes < bytes) {
      auto reading_address = address + readed_bytes;
      auto page = reading_address >> this->page_size_bits;
      const auto &map = this->map_table_[page];
      if (map) {
        auto offset = reading_address - map->address_;
        auto reading_bytes = std::min(bytes - offset, map->bytes_);
        if ((reading_address < map->address_) ||
            (map->address_ + map->bytes_ < reading_address + reading_bytes)) {
          if (this->notify_error_ != nullptr) {
            this->notify_error_(address + reading_bytes - 1,
                                BusAccessKind::Read);
          }
          return;
        }
        memcpy(buffer + readed_bytes, this->map_table_[page]->memory_ + offset,
               reading_bytes);
        readed_bytes += reading_bytes;
      } else {
        if (this->notify_error_ != nullptr) {
          this->notify_error_(address, BusAccessKind::Read);
        }
        return;
      }
    }
  }
  uint8_t read8(uint_fast16_t address);
  uint16_t read16(uint_fast16_t address);
  uint32_t read32(uint_fast16_t address);
  uint64_t read64(uint_fast16_t address);
  void write(const void *buffer, size_t bytes, uint_fast16_t destination);
  void write8(uint_fast16_t address, const uint8_t &value);
  void write16(uint_fast16_t address, const uint16_t &value);
  void write32(uint_fast16_t address, const uint32_t &value);
  void write64(uint_fast16_t address, const uint64_t &value);
  void dumpMap() const {
    std::cout << "dump map(" << this->page_size << ")\n";
    std::cout << "--------\n";
    for (const auto &map : this->map_table_) {
      if (map) {
        std::cout << std::hex << map->address_ << "\t" << std::hex
                  << map->bytes_ << std::endl;
      }
    }
  }

private:
  static constexpr auto address_bits = 16;
  static constexpr auto page_size_bits = 10;
  static constexpr uint_fast16_t page_size = 1 << page_size_bits;
  static constexpr std::uintptr_t page_mask = page_size - 1;
  ErrorCallback notify_error_;
  std::unique_ptr<Map> map_table_[1ULL << (address_bits - page_size_bits)];
};

} // namespace nes_emu

#endif // NES_EMU_BUS_H

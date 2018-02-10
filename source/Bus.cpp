//===-- nes_emu/Bus.cpp - Bus class implements  -----------------*- C++ -*-===//
//
// This file is distributed under the Boost Software License. See LICENSE.TXT
// for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains the implements of the Bus class, which is emulate the
/// system bus.
///
//===----------------------------------------------------------------------===//

//==============================================================================
//= Dependencies
//==============================================================================
// Main module header
#include "nes_emu/Bus.h"

// Local/Private headers
#include "nes_emu/Device.h"

// External headers

// System headers
#include <algorithm>
#include <cstring>
#include <iostream>

namespace nes_emu {

template <size_t address_bits> Bus<address_bits>::~Bus() noexcept = default;

template <size_t address_bits>
std::optional<std::errc> Bus<address_bits>::mapMemory(Device *dev,
                                                      AddressType address,
                                                      size_t bytes, void *mem) {
  auto page_begin = address >> this->kPageSizeBits;
  auto page_end = (address + bytes - 1) >> this->kPageSizeBits;
  auto mem_addr = static_cast<uint8_t *>(mem);
  // chack already exists
  for (auto page = page_begin; page <= page_end; ++page) {
    if (this->map_table_[page]) {
      return std::errc::file_exists;
    }
  }
  // map
  auto offset = address - (page_begin << this->kPageSizeBits);
  for (auto page = page_begin; page <= page_end; ++page) {
    this->map_table_[page] = std::make_unique<MemoryMap>(
        dev, mem_addr, (page << this->kPageSizeBits) + offset,
        std::min(bytes, this->kPageSize));
    bytes -= this->kPageSize;
    mem_addr += this->kPageSize;
    offset = 0;
  }
  return std::nullopt;
}

template <size_t address_bits>
void Bus<address_bits>::read(AddressType address, size_t bytes,
                             void *buffer) const noexcept {
  decltype(bytes) readed_bytes = 0;
  auto p = static_cast<uint8_t *>(buffer);
  while (readed_bytes < bytes) {
    auto reading_address = address + readed_bytes;
    auto page = reading_address >> this->kPageSizeBits;
    const auto &map = this->map_table_[page];
    if (map) {
      auto offset = reading_address - map->Address;
      auto reading_bytes = std::min(bytes - readed_bytes - offset, map->Bytes);
      if ((reading_address < map->Address) ||
          (map->Address + map->Bytes < reading_address + reading_bytes)) {
        if (this->notify_error_ != nullptr) {
          this->notify_error_(map->Address + offset, BusAccessKind::kRead);
        }
        return;
      }
      std::memcpy(p + readed_bytes, map->Memory + offset, reading_bytes);
      readed_bytes += reading_bytes;
    } else {
      if (this->notify_error_ != nullptr) {
        this->notify_error_(address, BusAccessKind::kRead);
      }
      return;
    }
  }
}

template <size_t address_bits>
void Bus<address_bits>::write(const void *buffer, size_t bytes,
                              AddressType destination) noexcept {
  decltype(bytes) written_bytes = 0;
  auto p = static_cast<const uint8_t *>(buffer);
  while (written_bytes < bytes) {
    auto writing_address = destination + written_bytes;
    auto page = writing_address >> this->kPageSizeBits;
    auto &map = this->map_table_[page];
    if (map) {
      auto offset = writing_address - map->Address;
      auto writing_bytes = std::min(bytes - written_bytes - offset, map->Bytes);
      if ((writing_address < map->Address) ||
          (map->Address + map->Bytes < writing_address + writing_bytes)) {
        if (this->notify_error_ != nullptr) {
          this->notify_error_(map->Address + offset, BusAccessKind::kWrite);
        }
        return;
      }
      std::memcpy(map->Memory + offset, p + written_bytes, writing_bytes);
      written_bytes += writing_bytes;
    } else {
      if (this->notify_error_ != nullptr) {
        this->notify_error_(destination, BusAccessKind::kWrite);
      }
      return;
    }
  }
}

template <size_t address_bits>
void Bus<address_bits>::dumpMap() const noexcept {
  std::cout << "dump map(" << this->kPageSize << ")\n";
  std::cout << "--------\n";
  for (const auto &map : this->map_table_) {
    if (map) {
      std::cout << std::hex << map->Address << "\t" << std::hex << map->Bytes
                << std::endl;
    }
  }
}

template class Bus<16>;

} // namespace nes_emu

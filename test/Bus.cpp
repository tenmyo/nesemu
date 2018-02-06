// Gtest
#include <gtest/gtest.h>

// Target module header
#include "nes_emu/Bus.h"

// Local/Private headers
#include "nes_emu/Device/Sram.h"

// External headers

// System headers
#include <cstring> // memset, memcmp

namespace nes_emu {

namespace {
class Bus16Test : public ::testing::Test {
protected:
  virtual void SetUp() override {
    this->p1_ = this->sram1_.data();
    this->p2_ = this->sram2_.data();
    memset(this->p1_, 0x01, this->sram1_.size());
    memset(this->p2_, 0x23, this->sram2_.size());
  }
  virtual void TearDown() override {}
  Bus16 bus_{[&](Bus16::AddressType addr, BusAccessKind op) -> void {
    ++this->cnt_;
    this->addr_ = addr;
    this->op_ = op;
  }};
  Bus16::AddressType addr_ = 0;
  BusAccessKind op_ = BusAccessKind::kNone;
  int cnt_ = 0;
  Sram<0x400> sram1_;
  Sram<0x400> sram2_;
  uint8_t *p1_;
  uint8_t *p2_;
};
} // namespace
#define EXPECT_BUS_ERROR(cnt, addr, op)                                        \
  EXPECT_EQ(this->cnt_, cnt);                                                  \
  EXPECT_EQ(this->addr_, addr);                                                \
  EXPECT_EQ(this->op_, op);

TEST_F(Bus16Test, MapToExists) {
  // Setup
  ASSERT_FALSE(this->sram1_.map(&this->bus_, 0));
  // Do
  auto ret = this->sram1_.map(&this->bus_, 0);
  // Verify
  EXPECT_TRUE(ret);
  EXPECT_EQ(ret.value(), std::errc::file_exists);
}
TEST_F(Bus16Test, MapToMiddle) {
  // Setup
  ASSERT_FALSE(this->sram1_.map(&this->bus_, 0x100));
  uint8_t buf[0x400 + 1] = {0};
  uint_fast16_t addr = 0x100;
  uint_fast16_t bytes = this->sram1_.size();
  // Do
  this->bus_.read(addr, bytes, buf);
  // Verify
  EXPECT_BUS_ERROR(0, 0, BusAccessKind::kNone);
  EXPECT_EQ(std::memcmp(buf, this->p1_, bytes), 0);
  EXPECT_NE(buf[bytes], this->p1_[bytes]);
}

TEST_F(Bus16Test, ReadForNotRegisterd) {
  // Setup
  uint8_t buf[2 * 1024];
  uint_fast16_t addr = 0x100;
  // Do
  this->bus_.read(addr, 1, buf);
  // Verify
  EXPECT_BUS_ERROR(1, addr, BusAccessKind::kRead);
}
TEST_F(Bus16Test, ReadForRegisterdOne) {
  // Setup
  ASSERT_FALSE(this->sram1_.map(&this->bus_, 0x8000));
  uint8_t buf[2 * 1024] = {0};
  uint_fast16_t addr = 0x8000;
  uint_fast16_t bytes = 0x100;
  // Do
  this->bus_.read(addr, bytes, buf);
  // Verify
  EXPECT_BUS_ERROR(0, 0, BusAccessKind::kNone);
  EXPECT_EQ(std::memcmp(buf, this->p1_, bytes), 0);
  EXPECT_EQ(buf[bytes], 0);
}
TEST_F(Bus16Test, ReadOver) {
  // Setup
  ASSERT_FALSE(this->sram1_.map(&this->bus_, 0x100));
  uint8_t buf[1 * 0x500] = {0};
  uint_fast16_t addr = 0x100;
  uint_fast16_t bytes = this->sram1_.size() + 2;
  // Do
  this->bus_.read(addr, bytes, buf);
  // Verify
  EXPECT_BUS_ERROR(1, addr + this->sram1_.size(), BusAccessKind::kRead);
}
TEST_F(Bus16Test, ReadAcrossTwoMap) {
  // Setup
  ASSERT_FALSE(this->sram1_.map(&this->bus_, 0x400));
  ASSERT_FALSE(this->sram2_.map(&this->bus_, 0x800));
  uint8_t buf[0x400 * 2] = {0};
  uint_fast16_t addr = 0x400;
  uint_fast16_t bytes = this->sram1_.size() + 0x100;
  // Do
  this->bus_.read(addr, bytes, buf);
  // Verify
  EXPECT_BUS_ERROR(0, 0, BusAccessKind::kNone);
  EXPECT_EQ(std::memcmp(buf, this->p1_, this->sram1_.size()), 0);
  EXPECT_EQ(std::memcmp(buf + this->sram1_.size(), this->p2_, 0x100), 0);
  EXPECT_EQ(buf[this->sram1_.size() + 0x100], 0);
}

TEST_F(Bus16Test, WriteForNotRegisterd) {
  // Setup
  const uint8_t buf[2] = {0};
  uint_fast16_t addr = 0x100;
  // Do
  this->bus_.write(buf, sizeof(buf), addr);
  // Verify: error
  EXPECT_BUS_ERROR(1, addr, BusAccessKind::kWrite);
}
TEST_F(Bus16Test, WriteForRegisterdOne) {
  // Setup
  ASSERT_FALSE(this->sram1_.map(&this->bus_, 0x8000));
  const uint8_t buf[0x200] = {0};
  uint_fast16_t addr = 0x8000;
  size_t bytes = 0x100;
  // Do
  this->bus_.write(buf, bytes, addr);
  // Verify
  EXPECT_BUS_ERROR(0, 0, BusAccessKind::kNone);
  EXPECT_EQ(std::memcmp(buf, this->p1_, bytes), 0);
  EXPECT_EQ(this->p1_[bytes], 0x01);
}
TEST_F(Bus16Test, WriteOver) {
  // Setup
  ASSERT_FALSE(this->sram1_.map(&this->bus_, 0x100));
  uint8_t buf[0x500] = {0};
  uint_fast16_t addr = 0x100;
  size_t bytes = this->sram1_.size() + 2;
  // Do
  this->bus_.write(buf, bytes, addr);
  // Verify
  EXPECT_BUS_ERROR(1, addr + this->sram1_.size(), BusAccessKind::kWrite);
}

TEST_F(Bus16Test, WriteReadForRegisterdOne) {
  // Setup
  ASSERT_FALSE(this->sram1_.map(&this->bus_, 0x8000));
  const uint8_t buf[0x200] = {0};
  uint8_t buf2[0x200];
  memset(buf2, 0xce, sizeof(buf));
  uint_fast16_t addr = 0x8000;
  size_t bytes = 0x100;
  // Do
  this->bus_.write(buf, bytes, addr);
  // Verify
  EXPECT_BUS_ERROR(0, 0, BusAccessKind::kNone);
  EXPECT_EQ(std::memcmp(buf, this->p1_, bytes), 0);
  EXPECT_EQ(this->p1_[bytes], 0x01);
  // Do
  this->bus_.read(addr, bytes, buf2);
  // Verify
  EXPECT_BUS_ERROR(0, 0, BusAccessKind::kNone);
  EXPECT_EQ(std::memcmp(buf, buf2, bytes), 0);
  EXPECT_EQ(buf2[bytes], 0xce);
}
TEST_F(Bus16Test, WriteReadAcrossTwoMap) {
  // Setup
  ASSERT_FALSE(this->sram1_.map(&this->bus_, 0x400));
  ASSERT_FALSE(this->sram2_.map(&this->bus_, 0x800));
  const uint8_t buf[0x500 + 2] = {0};
  uint8_t buf2[0x500 + 2];
  memset(buf2, 0xce, sizeof(buf));
  uint_fast16_t addr = 0x400;
  uint_fast16_t bytes = this->sram1_.size() + 0x100;
  // Do
  this->bus_.write(buf, bytes, addr);
  // Verify: no error
  EXPECT_BUS_ERROR(0, 0, BusAccessKind::kNone);
  // Verify: written just the given bytes
  EXPECT_EQ(std::memcmp(buf, this->p1_, this->sram1_.size()), 0);
  EXPECT_EQ(std::memcmp(buf + this->sram1_.size(), this->p2_, 0x100), 0);
  EXPECT_EQ(this->p2_[0x100], 0x23);
  // Do
  this->bus_.read(addr, bytes, buf2);
  // Verify
  EXPECT_BUS_ERROR(0, 0, BusAccessKind::kNone);
  EXPECT_EQ(std::memcmp(buf, buf2, bytes), 0);
  EXPECT_EQ(buf2[bytes], 0xce);
}

TEST_F(Bus16Test, WriteRead8) {
  // Setup
  ASSERT_FALSE(this->sram1_.map(&this->bus_, 0x8000));
  uint_fast16_t addr = 0x8000;
  // Do
  this->bus_.write8(addr, 0x01);
  auto ret = this->bus_.read8(addr);
  // Verify: no error
  EXPECT_BUS_ERROR(0, 0, BusAccessKind::kNone);
  EXPECT_EQ(ret, 0x01);
}
TEST_F(Bus16Test, WriteRead16) {
  // Setup
  ASSERT_FALSE(this->sram1_.map(&this->bus_, 0x8000));
  uint_fast16_t addr = 0x8000;
  // Do
  this->bus_.write16(addr, 0x0123);
  auto ret = this->bus_.read16(addr);
  // Verify: no error
  EXPECT_BUS_ERROR(0, 0, BusAccessKind::kNone);
  EXPECT_EQ(ret, 0x0123);
}
TEST_F(Bus16Test, WriteRead32) {
  // Setup
  ASSERT_FALSE(this->sram1_.map(&this->bus_, 0x8000));
  uint_fast16_t addr = 0x8000;
  // Do
  this->bus_.write32(addr, 0x01234567);
  auto ret = this->bus_.read32(addr);
  // Verify: no error
  EXPECT_BUS_ERROR(0, 0, BusAccessKind::kNone);
  EXPECT_EQ(ret, 0x01234567);
}
TEST_F(Bus16Test, WriteRead64) {
  // Setup
  ASSERT_FALSE(this->sram1_.map(&this->bus_, 0x8000));
  uint_fast16_t addr = 0x8000;
  // Do
  this->bus_.write64(addr, 0x0123456789abcdef);
  auto ret = this->bus_.read64(addr);
  // Verify: no error
  EXPECT_BUS_ERROR(0, 0, BusAccessKind::kNone);
  EXPECT_EQ(ret, 0x0123456789abcdef);
}
} // namespace nes_emu

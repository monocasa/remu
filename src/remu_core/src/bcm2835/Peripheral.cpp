#include "remu/bcm2835/Peripheral.h"
#include "remu/Memory.h"
#include "remu/Ui.h"

#include "oshal/Exception.h"

namespace remu { namespace bcm2835 {

/**
 * Initialises peripherials
 */
Peripheral::Peripheral(Ui &ui, Memory &mem)
  : IoRegion(AUX_BASE, 0x1000)
  , ui(ui)
  , mem(mem)
  , spi1_enable(false)
  , spi2_enable(false)
  , irq_tx(false)
  , irq_rx(false)
  , uart_enable(false)
  , uart_bits(7)
  , uart_dlab(0)
  , uart_rts(false)
  , uart_cntl_reg(0x00)
{
  mem.addRegion(this);
}

Peripheral::~Peripheral()
{
  mem.removeRegion(this);
}

/**
 * Handles a write to a peripherial register
 * @param addr  Port number
 * @param data  Data received
 */
void Peripheral::writeIo(uint64_t addr, uint64_t data, unsigned int size)
{
  if (size != sizeof(uint32_t))
  {
    throw oshal::Exception("Unknown peripheral write to addr %08lx:%dB <- %lx", addr, size, data);
  }

  switch (addr)
  {
    case AUX_ENABLES:
    {
      /* Enables / disables peripherials */
      uart_enable = (data & 0x1) != 0;
      spi1_enable = (data & 0x2) != 0;
      spi2_enable = (data & 0x4) != 0;
      return;
    }
    case AUX_MU_IER_REG:
    {
      if (uart_dlab)
      {
        /* MSB of baud rate register */
        uart_baud_rate = (uart_baud_rate & 0xFF) | ((data >> 8) & 0xFF);
      }
      else
      {
        /* Enables / disables interrupts */
        irq_rx = (data & 0x1) != 0;
        irq_tx = (data & 0x2) != 0;
      }
      return;
    }
    case AUX_MU_IIR_REG:
    {
      /* We don't actually emulate the FIFOs so this is a nop */
      return;
    }
    case AUX_MU_LCR_REG:
    {
      /* TODO: implement other bits */
      uart_bits = data & 0x1 ? 8 : 7;
      return;
    }
    case AUX_MU_BAUD_REG:
    {
      uart_baud_rate_counter = data & 0xFF;
      return;
    }
    case AUX_MU_CNTL_REG:
    {
      uart_cntl_reg = data;
      return;
    }
    case AUX_MU_MCR_REG:
    {
      uart_rts = data & 0x02;
      return;
    }
    case AUX_MU_IO_REG:
    {
      if (uart_dlab)
      {
        /* LSB of baud rate register */
        uart_baud_rate = (uart_baud_rate & 0xFF00) | (data & 0xFF);
      }
      else
      {
        /* Dump output to stdout */
        ui.info("%c", data & 0xFF);
      }
      return;
    }
    default:
    {
      throw oshal::Exception("remu::bmc2835::Peripheral write to unknown address (%08lx, 0x%lx, %d)", addr, data, size);
    }
  }

  ui.error("Unsupported peripherial write: %08x", addr);
}

/**
 * Handles a read from a peripherial port
 * @param addr Port address
 * @return     Read value
 */
uint64_t Peripheral::readIo(uint64_t addr, unsigned int size)
{
  if (size != sizeof(uint32_t))
  {
    throw oshal::Exception("Unknown peripheral read from %08lx:%dB", addr, size);
  }

  switch (addr)
  {
    case AUX_ENABLES:
    {
      /* Returns a bitmask of enabled peripherials */
      return (uart_enable ? 0x1 : 0x0) |
             (spi1_enable ? 0x2 : 0x0) |
             (spi2_enable ? 0x4 : 0x0);
    }
    case AUX_MU_IER_REG:
    {
      /* Returns a bitmask of interrupt status */
      return (irq_rx ? 0x1 : 0x0) |
             (irq_tx ? 0x2 : 0x0);
    }
    case AUX_MU_LSR_REG:
    {
      /* Transmitter always ready */
      return 0x60;
    }
    case AUX_MU_IO_REG:
    {
      if (uart_dlab)
      {
        /* LSB of baud rate register */
        return uart_baud_rate & 0xFF;
      }
      else
      {
        /* Always read 0 */
        return 0x00;
      }
    }
    default:
    {
      throw oshal::Exception("remu::bmc2835::Peripheral read from unknown address (%08lx, %d)", addr, size);
    }
  }

  ui.error("Unsupported peripherial read: %08x", addr);
  return 0;
}

}} /*namespace remu::bcm2835*/


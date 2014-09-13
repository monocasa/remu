#include "bcm2835/gpio.h"
#include "emulationexception.h"
#include "memory.h"
#include "ui.h"

namespace remu { namespace bcm2835 {

/**
 * Initialises memory for the gpio registers
 * @param emu  Reference to the emulator structure
 */
Gpio::Gpio(Ui &ui, Memory &mem)
  : IoRegion(GPIO_BASE, GPIO_REG_BANK_SIZE)
  , ui(ui)
  , mem(mem)
{
  gpio_port_t default_port = { 0, 0 };
  for(int i = 0; i < GPIO_PORT_COUNT; i++) {
    ports.push_back(default_port); 
  }

  mem.addRegion(this);
}

Gpio::~Gpio()
{
  mem.removeRegion(this);
}

/**
 * Handles reading from the gpio registers
 * @param address GPIO register address
 * @param size    Access size in bytes
 */
uint64_t Gpio::readIo(uint64_t address, unsigned int size) 
{
  uint32_t reg = 0, offset = 0;
  uint8_t base;
  size_t i;

  if (size != sizeof(uint32_t))
  {
    throw EmulationException("Gpio has only implemented 32 bit reads:  addr=%08lx size=%d", address, size);
  }

  switch (address)
  {
    case GPIO_FSEL0 ... GPIO_FSEL5:
    {
      base = (address & 0x000000FF) / 4 * 10;

      for (i = 0; i < 10; i++)
      {
        reg = reg | (ports[base + i].func << (3 * i));
      }

      return reg;
    }
    case GPIO_SET1:
    case GPIO_LEV1:
      offset = 32;
    case GPIO_SET0:
    case GPIO_LEV0:
    {
      for (i = 0; i < 32; i++)
      {
        reg = reg | (ports[i + offset].state << i);
      }
      return reg;
    }
    case GPIO_CLR1:
      offset = 32;
    case GPIO_CLR0:
    {
      for (i = 0; i < 32; i++)
      {
        reg = reg | (ports[i + offset].state << i);
      }

      return ~reg;
    }
  }

  ui.error("GPIO unimplemented 0x%08x", address);
  return 0;
}

/**
 * Handles writing to the gpio registers
 * @param address GPIO register address
 * @param val     Value to be written
 * @param size    Access size in bytes
 */
void Gpio::writeIo(uint64_t address, uint64_t val, unsigned int size)
{
  uint32_t offset = 0;
  size_t i;

  if (size != sizeof(uint32_t))
  {
    throw EmulationException("Gpio has only implemented 32 bit writes:  addr=%08lx val=%lx size=%d", address, val, size);
  }

  switch (address)
  {
    case GPIO_FSEL0 ... GPIO_FSEL5:
    {
      uint8_t base = (address & 0x000000FF) / 4 * 10;

      for (i = 0; i < 10; i++)
      {
        ports[base + i].func = val & 0x00000007;
        val = val >> 3;
      }
      return;
    }
    case GPIO_SET1:
      offset = 32;
    case GPIO_SET0:
    {
      for (i = 0; i < 32; i++)
      {
        if (val & 1)
        {
          /* Set GPIO port state */
          ports[offset + i].state = 1;

          dispatchGpioWrite(i, 1);
        }
        val = val >> 1;
      }
      return;
    }
    case GPIO_CLR1:
      offset = 32;
    case GPIO_CLR0:
    {
      for (i = 0; i < 32; i++)
      {
        if (val & 1)
        {
          /* Set GPIO port state */
          ports[offset + i].state = 0;

          dispatchGpioWrite(i, 0);
        }
        val = val >> 1;
      }
      return;
    }
  }

  ui.error("GPIO unimplemented 0x%08x", address);
}

}} /*namespace remu::bcm2835*/


#include "common.h"

/**
 * Initialises memory for the gpio registers
 * @param emu  Reference to the emulator structure
 */
Gpio::Gpio(Emulator* emu)
  : emu(emu)
  , ports(new gpio_port_t[GPIO_PORT_COUNT])
{
  memset(ports, 0, sizeof(gpio_port_t) * GPIO_PORT_COUNT);
}

/**
 * Frees the gpio ports memory
 */
Gpio::~Gpio()
{
  delete ports;
}

/**
 * Handles reading from the gpio registers
 * @param gpio Reference to gpio structure
 * @param address GPIO register address
 */
uint32_t
gpio_read_port(Gpio* gpio, uint32_t address)
{
  uint32_t reg = 0, offset = 0;
  uint8_t base;
  size_t i;

  /* Align the address */
  address &= ~0x3;
  assert(gpio_is_port(address));

  switch (address)
  {
    case GPIO_FSEL0 ... GPIO_FSEL5:
    {
      base = (address & 0x000000FF) / 4 * 10;

      for (i = 0; i < 10; i++)
      {
        reg = reg | (gpio->ports[base + i].func << (3 * i));
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
        reg = reg | (gpio->ports[i + offset].state << i);
      }
      return reg;
    }
    case GPIO_CLR1:
      offset = 32;
    case GPIO_CLR0:
    {
      for (i = 0; i < 32; i++)
      {
        reg = reg | (gpio->ports[i + offset].state << i);
      }

      return ~reg;
    }
  }

  gpio->emu->error("GPIO unimplemented 0x%08x", address);
  return 0;
}

/**
 * Handles writing to the gpio registers
 * @param gpio Reference to gpio structire
 * @param address GPIO register address
 * @param val  Value to be written
 */
void
gpio_write_port(Gpio* gpio, uint32_t address, uint32_t val)
{
  uint32_t offset = 0;
  size_t i;

  /* Align the address */
  address &= ~0x3;
  assert(gpio_is_port(address));

  /* If nes controller is enabled, write to controller */
  switch (address)
  {
    case GPIO_FSEL0 ... GPIO_FSEL5:
    {
      uint8_t base = (address & 0x000000FF) / 4 * 10;

      for (i = 0; i < 10; i++)
      {
        gpio->ports[base + i].func = val & 0x00000007;
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
          gpio->ports[offset + i].state = 1;

          /* Catch writes */
          if (gpio->emu->isNesEnabled() && offset == 0)
          {
            nes_gpio_write(&gpio->emu->nes, i, 1);
          }
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
          gpio->ports[offset + i].state = 0;

          /* Catch writes */
          if (gpio->emu->isNesEnabled() && offset == 0)
          {
            nes_gpio_write(&gpio->emu->nes, i, 0);
          }
        }
        val = val >> 1;
      }
      return;
    }
  }

  gpio->emu->error("GPIO unimplemented 0x%08x", address);
}

/**
 * Checks whether a given address is a GPIO port
 */
int
gpio_is_port(uint32_t addr)
{
  addr &= ~0x3;
  if (addr < GPIO_BASE || GPIO_UDCLK1 < addr)
  {
    return 0;
  }

  switch (addr)
  {
    case GPIO_FSEL0 ... GPIO_FSEL5:
    case GPIO_SET0 ... GPIO_SET1:
    case GPIO_CLR0 ... GPIO_CLR1:
    case GPIO_LEV0 ... GPIO_LEV1:
    case GPIO_EDS0 ... GPIO_EDS1:
    case GPIO_REN0 ... GPIO_REN1:
    case GPIO_FEN0 ... GPIO_FEN1:
    case GPIO_HEN0 ... GPIO_HEN1:
    case GPIO_LEN0 ... GPIO_LEN1:
    case GPIO_AREN0 ... GPIO_AREN1:
    case GPIO_AFEN0 ... GPIO_AFEN1:
    case GPIO_PUD:
    case GPIO_UDCLK0 ... GPIO_UDCLK1:
      return 1;
    default:
      return 0;
  }
}


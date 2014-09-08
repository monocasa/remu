#include "common.h"

namespace remu {

/**
 * Initialises the mailbox
 */
Mbox::Mbox(Emulator *emu)
  : emu(emu)
  , last_channel(0x0)
{ }

/**
 * Reads data from a mailbox port
 * @param mbox Mailbox structure
 * @param addr Port address
 */
uint32_t
mbox_read(Mbox *mbox, uint32_t addr)
{
  addr &= ~0x3;
  assert(mbox_is_port(addr));

  /* Check which port is being read */
  switch (addr)
  {
    case MBOX_READ:
    {
      /* Always return 0 + last channel id */
      switch (mbox->last_channel)
      {
        case 1:
        {
          /* Return non zero after a failed request */
          return mbox->last_channel | (mbox->emu->fb.error ? ~0xF : 0x0);
        }
        default:
        {
          return mbox->last_channel;
        }
      }
      break;
    }
    case MBOX_STATUS:
    {
      /* Bit 31 == 0: ready to receive
         Bit 30 == 0: ready to send */
      return 0;
    }
  }

  mbox->emu->error("Mailbox unimplemented 0x%08x", addr);
  return 0;
}

/**
 * Writes data to a mailbox port
 * @param mbox Mailbox structure
 * @param addr Port address
 * @param val  Value to be written
 */
void
mbox_write(Mbox *mbox, uint32_t addr, uint32_t val)
{
  uint8_t channel;
  uint32_t data;

  addr &= ~0x3;
  assert(mbox_is_port(addr));

  /* Retrieve data & channel */
  channel = val & 0xF;
  data = val & ~0xF;

  /* Save the channel of the last request */
  mbox->last_channel = channel;

  /* Check which port is being written */
  switch (addr)
  {
    case MBOX_WRITE:
    {
      switch (channel)
      {
        case 1:   /* Framebuffer */
        {
          fb_request(&mbox->emu->fb, data);
          return;
        }
        default:
        {
          mbox->emu->error("Wrong channel 0x%x", channel);
          return;
        }
      }
      break;
    }
  }

  mbox->emu->error("Mailbox unimplemented 0x%08x", addr);
}

} /*namespace remu*/


#ifndef REMU_BCM2835_FRAMEBUFFER_H
#define REMU_BCM2835_FRAMEBUFFER_H

class Gpio;
class KeyListener;

/**
 * Framebuffer request structure
 */
typedef union
{
  uint32_t data[10];
  struct
  {
    uint32_t phys_width;
    uint32_t phys_height;
    uint32_t virt_width;
    uint32_t virt_height;
    uint32_t pitch;
    uint32_t depth;
    uint32_t off_x;
    uint32_t off_y;
    uint32_t addr;
    uint32_t size;
  } fb;
} framebuffer_req_t;

/**
 * Framebuffer data
 */
typedef struct _framebuffer_t
{
  /* Emulator reference */
  Emulator     *emu;
  Gpio         *gpio;

  /* KeyListener */
  KeyListener  *key_listener;

  /* Framebuffer */
  uint8_t*      framebuffer;
  size_t        fb_bpp;
  size_t        fb_pitch;
  size_t        fb_size;
  uint32_t      fb_address;
  uint16_t      fb_palette[256];

  /* Flag if set if query is malformed */
  int           error;

  /* Window */
  SDL_Surface*  surface;
  uint32_t      width;
  uint32_t      height;
  uint32_t      depth;
} framebuffer_t;

static inline
void fb_set_key_listener(framebuffer_t *fb, KeyListener *key_listener)
{
    fb->key_listener = key_listener;
}

void fb_init(framebuffer_t*, Emulator*, Gpio*);
void fb_create_window(framebuffer_t*, uint32_t width, uint32_t height);
void fb_destroy(framebuffer_t*);
void fb_tick(framebuffer_t*);
void fb_dump(framebuffer_t*);
void fb_request(framebuffer_t*, uint32_t address);
void fb_write_word(framebuffer_t*, uint32_t address, uint16_t data);
void fb_write_dword(framebuffer_t*, uint32_t address, uint32_t data);
int  fb_is_buffer(framebuffer_t*, uint32_t address);

#endif /* REMU_BCM2835_FRAMEBUFFER_H */


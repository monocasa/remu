#ifndef REMU_EMULATOR_H
#define REMU_EMULATOR_H

struct EmulatorOptions
{
  EmulatorOptions()
	: image( nullptr )
    , mem_size( 0 )
    , start_addr( 0 )
    , graphics( 0 )
    , usage( 0 )
    , quiet( 0 )
    , nes_enabled( 0 )
    , gpio_test_offset( 0 )
  { }

  const char   *image;

  size_t        mem_size;
  uint32_t      start_addr;
  int           graphics;
  int           usage;
  int           quiet;
  int           nes_enabled;
  int           gpio_test_offset;
};

/**
 * Emulator state
 */
class Emulator
{
public:
  Emulator(const EmulatorOptions &opt);

  virtual ~Emulator();

  void init();
  void load();
  bool isRunning() const;
  uint64_t getSystemTimer() const;
  void tick();

  void info(const char *fmt, ...);
  void error(const char *fmt, ...);
  void fatal(const char *fmt, ...) __attribute__((noreturn));

  void dump();

private:
  uint64_t getTime() const;

public:
  int           terminated;

  /* Kernel image */
  const char   *image;

  /* Error handling */
  jmp_buf       err_jmp;
  char*         err_msg;

  /* Arguments */
  size_t        mem_size;
  uint32_t      start_addr;
  int           graphics;
  int           usage;
  int           quiet;
  int           nes_enabled;
  int           gpio_test_offset;

  /* Modules */
  framebuffer_t fb;
  memory_t      memory;
  cpu_t         cpu;
  gpio_t        gpio;
  mbox_t        mbox;
  peripheral_t  pr;
  vfp_t         vfp;
  nes_t         nes;

  /* System Timer */
  uint64_t      system_timer_base;

  /* Refresh */
  uint64_t      last_refresh;
};

#endif /* REMU_EMULATOR_H */


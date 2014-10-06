#ifndef REMU_UTIL_FILE_H
#define REMU_UTIL_FILE_H

#include "remu/util/BitOps.h"
#include "remu/EmulationException.h"

#include <cstdio>
#include <string>

namespace remu { namespace util {

class File
{
public:
  enum Endian {
    BIG,
    LITTLE
  };

  File(const std::string &path)
    : path(path)
    , file(fopen(path.c_str(), "rb"))
    , endian(SYS_ENDIAN)
  { }

  ~File() {
    if (file) {
      fclose(file);
    }
  }

  bool isOpen() {
    return (nullptr != file);
  }

  void setEndian(Endian endian) {
    this->endian = endian;
  }

  void seek(uint64_t offset) {
    fseek(file, offset, SEEK_SET);
  }

  void readBuffer(uint8_t *buffer, const size_t size) {
    if (1 != fread(buffer, size, 1, file)) {
      throw EmulationException("Attempting to read beyond end of file:  %s", path.c_str());
    }
  }

  template<typename T>
  T read() {
    T value;

    if (1 != fread(&value, sizeof(T), 1, file)) {
      throw EmulationException("Attempting to read beyond end of file:  %s", path.c_str());
    }

    if (endian != SYS_ENDIAN) {
      value = byteSwap<T>(value);
    }

    return value;
  }

  size_t size() {
    long current = ftell(file);

    fseek(file, 0, SEEK_END);

    long last = ftell(file);

    fseek(file, current, SEEK_SET);

    return last;
  }

private:
  static const Endian SYS_ENDIAN = LITTLE;

  std::string path;
  FILE* file;
  Endian endian;
};

}} /*namespace remu::util*/

#endif /*REMU_UTIL_FILE_H*/


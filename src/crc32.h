#ifndef __CRC32_H_
#define __CRC32_H_

// Считает crc32 от файла
uint32_t crc32f(const char* filename);
// Считает crc32 от строки
uint32_t crc32(const char* string);

#endif
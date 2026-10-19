#ifndef __W25Q64_H
#define __W25Q64_H

#include <stdint.h>

#define W25Q64_SECTOR_SIZE	4096UL

void W25Q64_Init(void);
void W25Q64_ReadID(uint8_t *mid, uint8_t *type, uint8_t *capacity);
void W25Q64_ReadData(uint32_t addr, uint8_t *buf, uint32_t len);
void W25Q64_PageProgram(uint32_t addr, const uint8_t *buf, uint16_t len);
void W25Q64_SectorErase(uint32_t addr);
void W25Q64_WaitReady(void);
uint8_t W25Q64_IsBusy(void);

#endif

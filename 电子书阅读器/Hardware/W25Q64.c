#include "stm32f10x.h"
#include "MySPI.h"
#include "W25Q64.h"
#include "W25Q64_Ins.h"

void W25Q64_Init(void)
{
	MySPI_Init();
	W25Q64_WaitReady();
}

void W25Q64_ReadID(uint8_t *mid, uint8_t *type, uint8_t *capacity)
{
	MySPI_Start();
	MySPI_SwapByte(W25Q64_JEDEC_ID);
	*mid = MySPI_SwapByte(W25Q64_DUMMY_BYTE);
	*type = MySPI_SwapByte(W25Q64_DUMMY_BYTE);
	*capacity = MySPI_SwapByte(W25Q64_DUMMY_BYTE);
	MySPI_Stop();
}

static void W25Q64_WriteEnable(void)
{
	MySPI_Start();
	MySPI_SwapByte(W25Q64_WRITE_ENABLE);
	MySPI_Stop();
}

static uint8_t W25Q64_ReadStatus1(void)
{
	uint8_t status;
	MySPI_Start();
	MySPI_SwapByte(W25Q64_READ_STATUS_REGISTER_1);
	status = MySPI_SwapByte(W25Q64_DUMMY_BYTE);
	MySPI_Stop();
	return status;
}

uint8_t W25Q64_IsBusy(void)
{
	return (W25Q64_ReadStatus1() & 0x01) ? 1 : 0;
}

void W25Q64_WaitReady(void)
{
	uint32_t timeout = 1000000;
	while ((W25Q64_ReadStatus1() & 0x01) && timeout)
	{
		timeout--;
	}
}

void W25Q64_ReadData(uint32_t addr, uint8_t *buf, uint32_t len)
{
	uint32_t i;

	MySPI_Start();
	MySPI_SwapByte(W25Q64_READ_DATA);
	MySPI_SwapByte((uint8_t)(addr >> 16));
	MySPI_SwapByte((uint8_t)(addr >> 8));
	MySPI_SwapByte((uint8_t)(addr));
	for (i = 0; i < len; i++)
	{
		buf[i] = MySPI_SwapByte(W25Q64_DUMMY_BYTE);
	}
	MySPI_Stop();
}

void W25Q64_PageProgram(uint32_t addr, const uint8_t *buf, uint16_t len)
{
	uint16_t i;

	if (len > 256) len = 256;

	W25Q64_WriteEnable();
	MySPI_Start();
	MySPI_SwapByte(W25Q64_PAGE_PROGRAM);
	MySPI_SwapByte((uint8_t)(addr >> 16));
	MySPI_SwapByte((uint8_t)(addr >> 8));
	MySPI_SwapByte((uint8_t)(addr));
	for (i = 0; i < len; i++)
	{
		MySPI_SwapByte(buf[i]);
	}
	MySPI_Stop();

	W25Q64_WaitReady();
}

void W25Q64_SectorErase(uint32_t addr)
{
	W25Q64_WriteEnable();
	MySPI_Start();
	MySPI_SwapByte(W25Q64_SECTOR_ERASE_4KB);
	MySPI_SwapByte((uint8_t)(addr >> 16));
	MySPI_SwapByte((uint8_t)(addr >> 8));
	MySPI_SwapByte((uint8_t)(addr));
	MySPI_Stop();

	W25Q64_WaitReady();
}

#ifndef __DEV_FLASH_H__
#define __DEV_FLASH_H__

#include "stm32f4xx_hal.h"

// W25Q32 指令集宏定义
#define W25Q32_CMD_READ_ID         0x9F
#define W25Q32_CMD_READ_DATA       0x03
#define W25Q32_CMD_FAST_READ       0x0B
#define W25Q32_CMD_PAGE_PROGRAM    0x02
#define W25Q32_CMD_SECTOR_ERASE    0x20
#define W25Q32_CMD_CHIP_ERASE      0xC7
#define W25Q32_CMD_WRITE_ENABLE    0x06
#define W25Q32_CMD_READ_STATUS1    0x05

// 片选引脚定义
#define W25Q32_CS_GPIO_Port   GPIOE
#define W25Q32_CS_Pin         GPIO_PIN_3

#define W25Q32_CS_LOW()   HAL_GPIO_WritePin(W25Q32_CS_GPIO_Port, W25Q32_CS_Pin, GPIO_PIN_RESET)
#define W25Q32_CS_HIGH()  HAL_GPIO_WritePin(W25Q32_CS_GPIO_Port, W25Q32_CS_Pin, GPIO_PIN_SET)

// API 函数声明
uint32_t W25Q32_ReadID(void);
void W25Q32_ReadData(uint32_t addr, uint8_t *buf, uint16_t len);
void W25Q32_PageProgram(uint32_t addr, const uint8_t *buf, uint16_t len);
void W25Q32_SectorErase(uint32_t addr);
void W25Q32_ChipErase(void);

#endif // __DEV_FLASH_H__

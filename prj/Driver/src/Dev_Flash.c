#include "Dev_Flash.h"
#include "stm32f4xx_hal.h"

// W25Q32 指令集
#define W25Q32_CMD_READ_ID         0x9F // 读取芯片ID
#define W25Q32_CMD_READ_DATA       0x03 // 读取数据
#define W25Q32_CMD_FAST_READ       0x0B // 快速读取
#define W25Q32_CMD_PAGE_PROGRAM    0x02 // 页编程
#define W25Q32_CMD_SECTOR_ERASE    0x20 // 扇区擦除
#define W25Q32_CMD_CHIP_ERASE      0xC7 // 整片擦除
#define W25Q32_CMD_WRITE_ENABLE    0x06 // 写使能
#define W25Q32_CMD_READ_STATUS1    0x05 // 读取状态寄存器1

// SPI3 句柄
extern SPI_HandleTypeDef hspi3;




/**
 * @description: 等待 W25Q32 处于空闲状态
 * @return {*}
 */
static void W25Q32_WaitBusy(void)
{
    uint8_t cmd = W25Q32_CMD_READ_STATUS1;
    uint8_t status;
    do {
        W25Q32_CS_LOW();
        HAL_SPI_Transmit_DMA(&hspi3, &cmd, 1);
        while (hspi3.State != HAL_SPI_STATE_READY) {}
        HAL_SPI_Receive_DMA(&hspi3, &status, 1);
        while (hspi3.State != HAL_SPI_STATE_READY) {}
        W25Q32_CS_HIGH();
    } while (status & 0x01);
}

// 写使能
static void W25Q32_WriteEnable(void)
{
    uint8_t cmd = W25Q32_CMD_WRITE_ENABLE;
    W25Q32_CS_LOW();
    HAL_SPI_Transmit_DMA(&hspi3, &cmd, 1);
    while (hspi3.State != HAL_SPI_STATE_READY) {}
    W25Q32_CS_HIGH();
}

// 读取芯片ID
uint32_t W25Q32_ReadID(void)
{
    uint8_t cmd = W25Q32_CMD_READ_ID;
    uint8_t id[3];
    W25Q32_CS_LOW();
    HAL_SPI_Transmit_DMA(&hspi3, &cmd, 1);
    while (hspi3.State != HAL_SPI_STATE_READY) {}
    HAL_SPI_Receive_DMA(&hspi3, id, 3);
    while (hspi3.State != HAL_SPI_STATE_READY) {}
    W25Q32_CS_HIGH();
    return (id[0] << 16) | (id[1] << 8) | id[2];
}

// 读取数据
/**
 * @description: 读取指定地址的数据
 * @param {uint32_t} addr 地址
 * @note 地址必须是24位，范围0x000000 - 0xFFFFFF
 * @param {uint8_t} *buf 指向用于存储读取数据的缓冲区
 * @param {uint16_t} len 要读取的数据长度，最大为256字节
 * @return {*}
 */
void W25Q32_ReadData(uint32_t addr, uint8_t *buf, uint16_t len)
{
    uint8_t cmd[4];
    cmd[0] = W25Q32_CMD_READ_DATA;
    cmd[1] = (addr >> 16) & 0xFF;
    cmd[2] = (addr >> 8) & 0xFF;
    cmd[3] = addr & 0xFF;
    W25Q32_CS_LOW();
    HAL_SPI_Transmit_DMA(&hspi3, cmd, 4);
    while (hspi3.State != HAL_SPI_STATE_READY) {}
    HAL_SPI_Receive_DMA(&hspi3, buf, len);
    while (hspi3.State != HAL_SPI_STATE_READY) {}
    W25Q32_CS_HIGH();
}

// 页编程（最多256字节）
/**
 * @description: 将数据写入指定页
 * @param {uint32_t} addr 页地址
 * @note 页大小为256字节，地址必须是256字节对齐
 * @param {uint8_t} *buf 指向要写入的数据缓冲区
 * @note 该函数会自动使能写操作，并等待写入完成
 * @param {uint16_t} len 要写入的数据长度，最大为256字节
 * @note 如果 len 大于256字节，只会写入前256字节，其余数据将被忽略
 * @note 如果 len 小于256字节，剩余部分将被填充为0xFF
 * @note 该函数会自动处理片选信号
 * @return {*}
 */
void W25Q32_PageProgram(uint32_t addr, const uint8_t *buf, uint16_t len)
{
    W25Q32_WriteEnable();
    uint8_t cmd[4];
    cmd[0] = W25Q32_CMD_PAGE_PROGRAM;
    cmd[1] = (addr >> 16) & 0xFF;
    cmd[2] = (addr >> 8) & 0xFF;
    cmd[3] = addr & 0xFF;
    W25Q32_CS_LOW();
    HAL_SPI_Transmit_DMA(&hspi3, cmd, 4);
    while (hspi3.State != HAL_SPI_STATE_READY) {}
    HAL_SPI_Transmit_DMA(&hspi3, (uint8_t *)buf, len);
    while (hspi3.State != HAL_SPI_STATE_READY) {}
    W25Q32_CS_HIGH();
    W25Q32_WaitBusy();
}

/**
 * @description: 擦除指定扇区
 * @param {uint32_t} addr 扇区地址
 * @note 扇区大小为4KB，地址必须是4KB对齐
 * @return {*}
 */
void W25Q32_SectorErase(uint32_t addr)
{
    W25Q32_WriteEnable();
    uint8_t cmd[4];
    cmd[0] = W25Q32_CMD_SECTOR_ERASE;
    cmd[1] = (addr >> 16) & 0xFF;
    cmd[2] = (addr >> 8) & 0xFF;
    cmd[3] = addr & 0xFF;
    W25Q32_CS_LOW();
    HAL_SPI_Transmit_DMA(&hspi3, cmd, 4);
    while (hspi3.State != HAL_SPI_STATE_READY) {}
    W25Q32_CS_HIGH();
    W25Q32_WaitBusy();
}


/**
 * @description: 擦除整个芯片
 * @return {*}
 */
void W25Q32_ChipErase(void)
{
    W25Q32_WriteEnable();
    uint8_t cmd = W25Q32_CMD_CHIP_ERASE;
    W25Q32_CS_LOW();
    HAL_SPI_Transmit_DMA(&hspi3, &cmd, 1);
    while (hspi3.State != HAL_SPI_STATE_READY) {}
    W25Q32_CS_HIGH();
    W25Q32_WaitBusy();
}


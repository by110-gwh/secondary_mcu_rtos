#include "w25q.h"
#include "qspi.h"

#include "FreeRTOS.h"
#include "task.h"

#define W25X_SECTOR_ERASE 0x20
#define W25X_PAGE_PROGRAM 0x02
#define W25X_FAST_READ_DATA 0x0B
#define W25X_WRITE_ENABLE 0x06
#define W25X_READ_STATUS_REG1 0x05
#define W25X_READ_STATUS_REG2 0x35
#define W25X_READ_STATUS_REG3 0x15

/**********************************************************************************************************
*函 数 名: w25q_read_sr
*功能说明: 读取W25QXX的状态寄存器，W25QXX一共有3个状态寄存器
*形    参: 状态寄存器编号
*返 回 值: 状态寄存器的值
**********************************************************************************************************/
uint8_t w25q_read_sr(uint8_t regno)
{  
	uint8_t byte, command; 
    switch(regno) {
    case 1:
        //读状态寄存器1指令
        command = W25X_READ_STATUS_REG1;
        break;
    case 2:
        //读状态寄存器2指令
        command = W25X_READ_STATUS_REG2;
        break;
    case 3:
        //读状态寄存器3指令
        command = W25X_READ_STATUS_REG3;
        break;
    default:
        command = W25X_READ_STATUS_REG1;
        break;
    }
    spi_send_cmd(command, 0, 0, 0, 1);
	qspi_receive(&byte, 1);
	return byte;
}
 
/**********************************************************************************************************
*函 数 名: w25q_write_enable
*功能说明: W25QXX写使能
*形    参: 无
*返 回 值: 无
**********************************************************************************************************/
void w25q_write_enable(void)
{
    spi_send_cmd(W25X_WRITE_ENABLE, 0, 0, 0, 0);
} 

/**********************************************************************************************************
*函 数 名: w25q_read
*功能说明: 在指定地址开始读取指定长度的数据
*形    参: 数据存储区 开始读取的地址 要读取的字节数
*返 回 值: 无
**********************************************************************************************************/
void w25q_read(uint8_t* pBuffer, uint32_t addr, uint16_t cnt)
{
    spi_send_cmd(W25X_FAST_READ_DATA, addr, 1, 8, cnt);
	qspi_receive(pBuffer,cnt); 
}  

/**********************************************************************************************************
*函 数 名: w25q_write_page
*功能说明: 在指定地址开始写入最大256字节的数据
*形    参: 数据存储区 开始写入的地址 要写入的字节数
*返 回 值: 无
**********************************************************************************************************/
void w25q_write_page(uint8_t *pBuffer, uint32_t addr, uint16_t cnt)
{
    //写使能
	w25q_write_enable();
    spi_send_cmd(W25X_PAGE_PROGRAM, addr, 1, 0, cnt);
	qspi_transmit(pBuffer, cnt);	         	   
    //等待写入结束
	w25q_wait_busy();
} 


/**********************************************************************************************************
*函 数 名: w25q_write
*功能说明: 无检验写SPI FLASH
*形    参: 数据存储区 开始写入的地址 要写入的字节数
*返 回 值: 无
**********************************************************************************************************/
void w25q_write(uint8_t *pBuffer, uint32_t addr, uint16_t cnt)
{ 			 		 
	uint16_t pageremain;
    //单页剩余的字节数
	pageremain = 256 - addr % 256;
    //不大于256个字节
	if (cnt <= pageremain)
        pageremain = cnt;
	while(1) {
		w25q_write_page(pBuffer, addr, pageremain);
        //写入结束了
		if (cnt == pageremain)
            break;
	 	else {
			pBuffer += pageremain;
			addr += pageremain;	

            //减去已经写入了的字节数
			cnt -= pageremain;			  
			if(cnt > 256)
                //一次可以写入256个字节
                pageremain = 256;
			else
                //不够256个字节了
                pageremain = cnt;
		}
	}
} 

/**********************************************************************************************************
*函 数 名: w25q_erase_sector
*功能说明: 擦除一个扇区
*形    参: 要擦除地址
*返 回 值: 无
**********************************************************************************************************/
void w25q_erase_sector(uint32_t addr)   
{
	w25q_write_enable();
	w25q_wait_busy();
    spi_send_cmd(W25X_SECTOR_ERASE, addr, 1, 0, 0);
    //等待擦除完成
	w25q_wait_busy();
}

/**********************************************************************************************************
*函 数 名: w25q_erase
*功能说明: 擦除扇区
*形    参: 要擦除地址 长度
*返 回 值: 无
**********************************************************************************************************/
void w25q_erase(uint32_t addr, uint32_t cnt)   
{
    uint16_t pageremain;
    //单扇区剩余的字节数
	pageremain = 4096 - addr % 4096;
    //不大于4096个字节
	if (cnt <= pageremain)
        pageremain = cnt;
	while(1) {
		w25q_erase_sector(addr);
        //擦除结束了
		if (cnt == pageremain)
            break;
	 	else {
			addr += pageremain;	

            //减去已经擦除的字节数
			cnt -= pageremain;			  
			if(cnt > 256)
                //一次可以擦除4096个字节
                pageremain = 4096;
			else
                //不够4096个字节了
                pageremain = cnt;
		}
	}
}

/**********************************************************************************************************
*函 数 名: w25q_wait_busy
*功能说明: 等待w25q操作完成
*形    参: 无
*返 回 值: 无
**********************************************************************************************************/
void w25q_wait_busy(void)
{   
    //等待BUSY位清空
    while((w25q_read_sr(1) & 0x01) == 0x01)
        vTaskDelay(1);
}

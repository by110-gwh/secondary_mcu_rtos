#include "qspi.h"
#include "stm32f7xx_hal.h"

static QSPI_HandleTypeDef hqspi;

/**********************************************************************************************************
*函 数 名: qspi_send_cmd
*功能说明: QSPI发送命令
*形    参: 要发送的指令 发送到的目的地址 有无地址 空指令周期数 数据数量
*返 回 值: 无
**********************************************************************************************************/
void spi_send_cmd(uint8_t cmd, uint32_t addr, uint8_t has_addr, uint8_t dummt_cycles, uint32_t count)
{
	QSPI_CommandTypeDef s_command;
    s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    s_command.Instruction = cmd;
    if (has_addr)
        s_command.AddressMode = QSPI_ADDRESS_1_LINE;
    else
        s_command.AddressMode = QSPI_ADDRESS_NONE;
        
    s_command.AddressSize = QSPI_ADDRESS_24_BITS;
    s_command.Address = addr;
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    s_command.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
    s_command.AlternateBytes = 0;
    if (count)
        s_command.DataMode = QSPI_DATA_1_LINE;
    else
        s_command.DataMode = QSPI_DATA_NONE;
    s_command.NbData = count;
    s_command.DummyCycles = dummt_cycles;
    s_command.DdrMode = QSPI_DDR_MODE_DISABLE;
    s_command.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    HAL_QSPI_Command(&hqspi, &s_command, 5000);
}

/**********************************************************************************************************
*函 数 名: qspi_init_of_memory
*功能说明: QSPI flash初始化内存映射
*形    参: 无
*返 回 值: 无
**********************************************************************************************************/
void qspi_init_of_memory(void)
{
    QSPI_CommandTypeDef s_command;
    QSPI_MemoryMappedTypeDef s_mem_mapped_cfg;
    uint8_t stareg2;
 
    //读状态寄存器2
    spi_send_cmd(0x35, 0, 0, 0, 1);
    HAL_QSPI_Receive(&hqspi, &stareg2, 5000);
    
    //若flash未使能QSPI
    if ((stareg2 & 0x02) == 0) {
        //flash写使能
        spi_send_cmd(0x06, 0, 0, 0, 0);
        
        //flash使能QSPI
        spi_send_cmd(0x31, 0, 0, 0, 1);
        stareg2 |= 0x02;
        HAL_QSPI_Transmit(&hqspi, &stareg2, 5000);
    }

    //把flash重新映射到0x90000000
    s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    s_command.Instruction = 0xeb;
    s_command.AddressMode = QSPI_ADDRESS_4_LINES;
    s_command.AddressSize = QSPI_ADDRESS_24_BITS;
    s_command.Address = 0;
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_4_LINES;
    s_command.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
    s_command.AlternateBytes = 0xF0;
    s_command.DataMode = QSPI_DATA_4_LINES;
    s_command.NbData = 0;
    s_command.DummyCycles = 4;
    s_command.DdrMode = QSPI_DDR_MODE_DISABLE;
    s_command.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    
    s_mem_mapped_cfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE;
    s_mem_mapped_cfg.TimeOutPeriod = 0;

    HAL_QSPI_MemoryMapped(&hqspi, &s_command, &s_mem_mapped_cfg);
}

/**********************************************************************************************************
*函 数 名: qspi_init
*功能说明: QSPI初始化
*形    参: 无
*返 回 值: 无
**********************************************************************************************************/
void qspi_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    //时钟使能
    __HAL_RCC_QSPI_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    
    //复位QSPI
    __HAL_RCC_QSPI_FORCE_RESET();
    HAL_Delay(1);
    __HAL_RCC_QSPI_RELEASE_RESET();
    
    //配置GPIO
    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_QUADSPI;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_QUADSPI;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_QUADSPI;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF10_QUADSPI;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    //配置QSPI
    hqspi.Instance = QUADSPI;
    hqspi.Init.ClockPrescaler = 1;
    hqspi.Init.FifoThreshold = 4;
    hqspi.Init.SampleShifting = QSPI_SAMPLE_SHIFTING_HALFCYCLE;
    hqspi.Init.FlashSize = 22;
    hqspi.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_5_CYCLE;;
    hqspi.Init.ClockMode = QSPI_CLOCK_MODE_0;
    hqspi.Init.FlashID = QSPI_FLASH_ID_1;
    hqspi.Init.DualFlash = QSPI_DUALFLASH_DISABLE;
    HAL_QSPI_Init(&hqspi);
}

/**********************************************************************************************************
*函 数 名: qspi_deinit
*功能说明: QSPI flash复位
*形    参: 无
*返 回 值: 无
**********************************************************************************************************/
void qspi_deinit(void)
{
    HAL_QSPI_DeInit(&hqspi);
    __HAL_RCC_QSPI_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOE, GPIO_PIN_2);
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_2|GPIO_PIN_6);
    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13);
}

/**********************************************************************************************************
*函 数 名: qspi_receive
*功能说明: QSPI接收指定长度的数据
*形    参: 接收数据缓冲区首地址 要传输的数据长度
*返 回 值: 0：正常 其他：错误代码
**********************************************************************************************************/
uint8_t qspi_receive(uint8_t* buf,uint32_t datalen)
{
    if (HAL_QSPI_Receive(&hqspi, buf, 5000) == HAL_OK)
        return 0;
    else
        return 1;
}

/**********************************************************************************************************
*函 数 名: qspi_transmit
*功能说明: QSPI发送指定长度的数据
*形    参: 发送数据缓冲区首地址 要传输的数据长度
*返 回 值: 0：正常 其他：错误代码
**********************************************************************************************************/
uint8_t qspi_transmit(uint8_t* buf,uint32_t datalen)
{
    if (HAL_QSPI_Transmit(&hqspi, buf, 5000) == HAL_OK)
        return 0;
    else
        return 1;
}

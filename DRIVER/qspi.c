#include "qspi.h"
#include "stm32f7xx_hal.h"

static QSPI_HandleTypeDef hqspi;

/**********************************************************************************************************
*函 数 名: qspi_init
*功能说明: QSPI flash初始化
*形    参: 无
*返 回 值: 无
**********************************************************************************************************/
void qspi_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    QSPI_CommandTypeDef s_command;
    QSPI_MemoryMappedTypeDef s_mem_mapped_cfg;
    uint8_t stareg2;
    
    //时钟使能
    __HAL_RCC_QSPI_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    
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
    hqspi.Init.ClockPrescaler = 255;
    hqspi.Init.FifoThreshold = 4;
    hqspi.Init.SampleShifting = QSPI_SAMPLE_SHIFTING_HALFCYCLE;
    hqspi.Init.FlashSize = 22;
    hqspi.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_1_CYCLE;
    hqspi.Init.ClockMode = QSPI_CLOCK_MODE_0;
    hqspi.Init.FlashID = QSPI_FLASH_ID_1;
    hqspi.Init.DualFlash = QSPI_DUALFLASH_DISABLE;
    HAL_QSPI_Init(&hqspi);
 
    //读状态寄存器2
    s_command.InstructionMode    = QSPI_INSTRUCTION_1_LINE;
    s_command.Instruction        = 0x35;
    s_command.AddressMode        = QSPI_ADDRESS_NONE;
    s_command.AddressSize        = QSPI_ADDRESS_24_BITS;
    s_command.Address            = 0;
    s_command.AlternateByteMode  = QSPI_ALTERNATE_BYTES_NONE;
    s_command.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
    s_command.AlternateBytes     = 0;
    s_command.DataMode           = QSPI_DATA_1_LINE;
    s_command.NbData             = 1;
    s_command.DummyCycles        = 0;
    s_command.DdrMode            = QSPI_DDR_MODE_DISABLE;
    s_command.DdrHoldHalfCycle   = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command.SIOOMode           = QSPI_SIOO_INST_EVERY_CMD;
    HAL_QSPI_Command(&hqspi,&s_command,5000);
    HAL_QSPI_Receive(&hqspi, &stareg2, 5000);
    
    //若flash未使能QSPI
    if ((stareg2 & 0x02) == 0) {
        //flash写使能
        s_command.InstructionMode    = QSPI_INSTRUCTION_1_LINE;
        s_command.Instruction        = 0x06;
        s_command.AddressMode        = QSPI_ADDRESS_NONE;
        s_command.AddressSize        = QSPI_ADDRESS_24_BITS;
        s_command.Address            = 0;
        s_command.AlternateByteMode  = QSPI_ALTERNATE_BYTES_NONE;
        s_command.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
        s_command.AlternateBytes     = 0;
        s_command.DataMode           = QSPI_DATA_NONE;
        s_command.NbData             = 0;
        s_command.DummyCycles        = 0;
        s_command.DdrMode            = QSPI_DDR_MODE_DISABLE;
        s_command.DdrHoldHalfCycle   = QSPI_DDR_HHC_ANALOG_DELAY;
        s_command.SIOOMode           = QSPI_SIOO_INST_EVERY_CMD;
        HAL_QSPI_Command(&hqspi,&s_command,5000);
        
        //flash使能QSPI
        stareg2 |= 0x02;
        s_command.InstructionMode    = QSPI_INSTRUCTION_1_LINE;
        s_command.Instruction        = 0x31;
        s_command.AddressMode        = QSPI_ADDRESS_NONE;
        s_command.AddressSize        = QSPI_ADDRESS_24_BITS;
        s_command.Address            = 0;
        s_command.AlternateByteMode  = QSPI_ALTERNATE_BYTES_NONE;
        s_command.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
        s_command.AlternateBytes     = 0;
        s_command.DataMode           = QSPI_DATA_1_LINE;
        s_command.NbData             = 1;
        s_command.DummyCycles        = 0;
        s_command.DdrMode            = QSPI_DDR_MODE_DISABLE;
        s_command.DdrHoldHalfCycle   = QSPI_DDR_HHC_ANALOG_DELAY;
        s_command.SIOOMode           = QSPI_SIOO_INST_EVERY_CMD;
        HAL_QSPI_Command(&hqspi,&s_command,5000);
        HAL_QSPI_Transmit(&hqspi, &stareg2, 5000);
    }

    //把flash重新映射到0x90000000
    s_command.InstructionMode    = QSPI_INSTRUCTION_1_LINE;
    s_command.Instruction        = 0xeb;
    s_command.AddressMode        = QSPI_ADDRESS_4_LINES;
    s_command.AddressSize        = QSPI_ADDRESS_24_BITS;
    s_command.Address            = 0;
    s_command.AlternateByteMode  = QSPI_ALTERNATE_BYTES_4_LINES;
    s_command.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
    s_command.AlternateBytes     = 0xF0;
    s_command.DataMode           = QSPI_DATA_4_LINES;
    s_command.NbData             = 0;
    s_command.DummyCycles        = 4;
    s_command.DdrMode            = QSPI_DDR_MODE_DISABLE;
    s_command.DdrHoldHalfCycle   = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command.SIOOMode           = QSPI_SIOO_INST_EVERY_CMD;
    
    s_mem_mapped_cfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE;
    s_mem_mapped_cfg.TimeOutPeriod     = 0;

    HAL_QSPI_MemoryMapped(&hqspi, &s_command, &s_mem_mapped_cfg);
}

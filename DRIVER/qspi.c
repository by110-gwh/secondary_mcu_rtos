#include "qspi.h"
#include "stm32f7xx_hal.h"

static QSPI_HandleTypeDef hqspi;

/**********************************************************************************************************
*�� �� ��: qspi_send_cmd
*����˵��: QSPI��������
*��    ��: Ҫ���͵�ָ�� ���͵���Ŀ�ĵ�ַ ���޵�ַ ��ָ�������� ��������
*�� �� ֵ: ��
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
*�� �� ��: qspi_init_of_memory
*����˵��: QSPI flash��ʼ���ڴ�ӳ��
*��    ��: ��
*�� �� ֵ: ��
**********************************************************************************************************/
void qspi_init_of_memory(void)
{
    QSPI_CommandTypeDef s_command;
    QSPI_MemoryMappedTypeDef s_mem_mapped_cfg;
    uint8_t stareg2;
 
    //��״̬�Ĵ���2
    spi_send_cmd(0x35, 0, 0, 0, 1);
    HAL_QSPI_Receive(&hqspi, &stareg2, 5000);
    
    //��flashδʹ��QSPI
    if ((stareg2 & 0x02) == 0) {
        //flashдʹ��
        spi_send_cmd(0x06, 0, 0, 0, 0);
        
        //flashʹ��QSPI
        spi_send_cmd(0x31, 0, 0, 0, 1);
        stareg2 |= 0x02;
        HAL_QSPI_Transmit(&hqspi, &stareg2, 5000);
    }

    //��flash����ӳ�䵽0x90000000
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
*�� �� ��: qspi_init
*����˵��: QSPI��ʼ��
*��    ��: ��
*�� �� ֵ: ��
**********************************************************************************************************/
void qspi_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    //ʱ��ʹ��
    __HAL_RCC_QSPI_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    
    //��λQSPI
    __HAL_RCC_QSPI_FORCE_RESET();
    HAL_Delay(1);
    __HAL_RCC_QSPI_RELEASE_RESET();
    
    //����GPIO
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
    
    //����QSPI
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
*�� �� ��: qspi_deinit
*����˵��: QSPI flash��λ
*��    ��: ��
*�� �� ֵ: ��
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
*�� �� ��: qspi_receive
*����˵��: QSPI����ָ�����ȵ�����
*��    ��: �������ݻ������׵�ַ Ҫ��������ݳ���
*�� �� ֵ: 0������ �������������
**********************************************************************************************************/
uint8_t qspi_receive(uint8_t* buf,uint32_t datalen)
{
    if (HAL_QSPI_Receive(&hqspi, buf, 5000) == HAL_OK)
        return 0;
    else
        return 1;
}

/**********************************************************************************************************
*�� �� ��: qspi_transmit
*����˵��: QSPI����ָ�����ȵ�����
*��    ��: �������ݻ������׵�ַ Ҫ��������ݳ���
*�� �� ֵ: 0������ �������������
**********************************************************************************************************/
uint8_t qspi_transmit(uint8_t* buf,uint32_t datalen)
{
    if (HAL_QSPI_Transmit(&hqspi, buf, 5000) == HAL_OK)
        return 0;
    else
        return 1;
}

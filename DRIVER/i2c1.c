#include "i2c1.h"
#include "stm32f7xx_hal.h"
#include "time_cnt.h"

#include "FreeRTOS.h"
#include "queue.h"

#define I2C_EVENT_SUCCESSFUL 1 << 0
#define I2C_EVENT_ERROR 1 << 1

//I2C��Ϣ��־����
static QueueHandle_t i2c_queue;
//I2C������
I2C_HandleTypeDef hi2c1;

//һЩ����������
void HAL_I2C1_MemTxCpltCallback(I2C_HandleTypeDef *hi2c);
void HAL_I2C1_MemRxCpltCallback(I2C_HandleTypeDef *hi2c);
void HAL_I2C1_ErrorCallback(I2C_HandleTypeDef *hi2c);
void HAL_I2C1_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c);
void HAL_I2C1_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c);

/**********************************************************************************************************
*�� �� ��: i2c1_fail_recover
*����˵��: I2C1����ָ�
*��    ��: ��
*�� �� ֵ: ��
**********************************************************************************************************/
static void i2c1_fail_recover(void)
{
    int nRetry = 0;
    if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_7) == 0 || HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_8) == 0) {
        //����I2C����Ϊ��©���
        GPIO_InitTypeDef GPIO_InitStruct = {0};
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8 | GPIO_PIN_7, GPIO_PIN_SET);
        GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_7;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
        
        do{
            //����CLK����
            delay_us(10);
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);
            delay_us(10);
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);
        //�ж�SDA�Ƿ�ָ�
        }while(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_7) == 0 && nRetry++ < 70);
        
        //����SDA
        delay_us(10);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
        //����CLK
        delay_us(10);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);
        //�ָ�SDa
        delay_us(10);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
        
        //�ָ�GPIO����
        GPIO_InitStruct.Pin = GPIO_PIN_7 | GPIO_PIN_8;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    }
    //���³�ʼ��I2C
	HAL_I2C_Init(&hi2c1);
}

/**********************************************************************************************************
*�� �� ��: i2c1_init
*����˵��: I2C1��ʼ��
*��    ��: ��
*�� �� ֵ: ��
**********************************************************************************************************/
void i2c1_init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	
	//����ʱ��ʹ��
	__HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_I2C1_CLK_ENABLE();
	
	//IO��ʼ��
    GPIO_InitStruct.Pin = GPIO_PIN_7 | GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    //I2C1����ָ�
    i2c1_fail_recover();
    
	//I2C��ʼ��
	hi2c1.Instance = I2C1;
    hi2c1.Init.Timing = 0x6000030D;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	HAL_I2C_Init(&hi2c1);
    
    //I2C�жϻص�����ע��
    hi2c1.MemRxCpltCallback = HAL_I2C1_MemRxCpltCallback;
    hi2c1.MemTxCpltCallback = HAL_I2C1_MemTxCpltCallback;
    hi2c1.ErrorCallback = HAL_I2C1_ErrorCallback;
    hi2c1.MasterRxCpltCallback = HAL_I2C1_MasterRxCpltCallback;
    hi2c1.MasterTxCpltCallback = HAL_I2C1_MasterTxCpltCallback;
	
    //ʹ��ģ���˲���
    HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE);
    //���������˲���
    HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0);
    
	//I2C�жϳ�ʼ��
    HAL_NVIC_SetPriority(I2C1_EV_IRQn, 8, 0);
    HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
    HAL_NVIC_SetPriority(I2C1_ER_IRQn, 15, 0);
    HAL_NVIC_EnableIRQ(I2C1_ER_IRQn);
	
	i2c_queue = xQueueCreate(1, sizeof(uint8_t));
}

/**********************************************************************************************************
*�� �� ��: i2c1_transmit
*����˵��: i2c1����
*��    ��: 7bit�ӻ���ַ �Ƿ�Ϊд д������ д������
*�� �� ֵ: д��״̬��0���ɹ���-1������
**********************************************************************************************************/
int i2c1_transmit(uint8_t slave_address, uint8_t is_write, uint8_t *pdata, uint8_t count)
{
    uint8_t res;
    if (is_write) {
        HAL_I2C_Master_Transmit_IT(&hi2c1, slave_address << 1, pdata, count);
    } else {
        HAL_I2C_Master_Receive_IT(&hi2c1, slave_address << 1, pdata, count);
    }
    
    xQueueReceive(i2c_queue, (void *) &res, portMAX_DELAY);
	if (res & I2C_EVENT_ERROR) {
        i2c1_fail_recover();
		return -1;
    }
	return 0;
}

/**********************************************************************************************************
*�� �� ��: i2c1_single_write
*����˵��: �жϷ�ʽ���е����Ĵ���д��
*��    ��: 7bit�ӻ���ַ �Ĵ�����ַ д������
*�� �� ֵ: д��״̬��0���ɹ���-1������
**********************************************************************************************************/
int i2c1_single_write(uint8_t slave_address, uint8_t reg_address, uint8_t reg_data)
{
	uint8_t res;
	if (HAL_I2C_Mem_Write_IT(&hi2c1, slave_address << 1, reg_address, I2C_MEMADD_SIZE_8BIT, &reg_data, 1) != HAL_OK)
		return -1;
	
	xQueueReceive(i2c_queue, (void *) &res, portMAX_DELAY);
	if (res & I2C_EVENT_ERROR) {
        i2c1_fail_recover();
		return -1;
    }
	return 0;
}

/**********************************************************************************************************
*�� �� ��: i2c1_single_read
*����˵��: �жϷ�ʽ���е����Ĵ�����ȡ
*��    ��: 7bit�ӻ���ַ �Ĵ�����ַ ��ȡ����
*�� �� ֵ: ��ȡ״̬��0���ɹ���-1������
**********************************************************************************************************/
int i2c1_single_read(uint8_t slave_address, uint8_t reg_address, uint8_t *reg_data)
{
	uint8_t res;
	if (HAL_I2C_Mem_Read_IT(&hi2c1, slave_address << 1, reg_address, I2C_MEMADD_SIZE_8BIT, reg_data, 1) != HAL_OK)
		return -1;
	
	xQueueReceive(i2c_queue, (void *) &res, portMAX_DELAY);
	if (res & I2C_EVENT_ERROR) {
        i2c1_fail_recover();
		return -1;
    }
	return 0;
}

/**********************************************************************************************************
*�� �� ��: i2c1_multi_read
*����˵��: �жϷ�ʽ���ж���Ĵ�����ȡ
*��    ��: 7bit�ӻ���ַ �Ĵ�����ַ ��ȡ���� ��ȡ����
*�� �� ֵ: ��ȡ״̬��0���ɹ���-1������
**********************************************************************************************************/
int i2c1_multi_read(uint8_t slave_address, uint8_t reg_address, uint8_t *reg_data, uint8_t read_cnt)
{
	uint8_t res;
	if (HAL_I2C_Mem_Read_IT(&hi2c1, slave_address << 1, reg_address, I2C_MEMADD_SIZE_8BIT, reg_data, read_cnt) != HAL_OK)
		return -1;
	
	xQueueReceive(i2c_queue, (void *) &res, portMAX_DELAY);
	if (res & I2C_EVENT_ERROR) {
        i2c1_fail_recover();
		return -1;
    }
	
	return 0;
}

/**********************************************************************************************************
*�� �� ��: i2c1_multi_write
*����˵��: �жϷ�ʽ���ж���Ĵ���д��
*��    ��: 7bit�ӻ���ַ �Ĵ�����ַ ��ȡ���� ��ȡ����
*�� �� ֵ: ��ȡ״̬��0���ɹ���-1������
**********************************************************************************************************/
int i2c1_multi_write(uint8_t slave_address, uint8_t reg_address, uint8_t *reg_data, uint8_t read_cnt)
{
	uint8_t res;
	if (HAL_I2C_Mem_Write_IT(&hi2c1, slave_address << 1, reg_address, I2C_MEMADD_SIZE_8BIT, reg_data, read_cnt) != HAL_OK)
		return -1;
	
	xQueueReceive(i2c_queue, (void *) &res, portMAX_DELAY);
	if (res & I2C_EVENT_ERROR) {
        i2c1_fail_recover();
		return -1;
    }
	
	return 0;
}

/**********************************************************************************************************
*�� �� ��: HAL_I2C_MemTxCpltCallback
*����˵��: HAL��I2C�ص�����
*��    ��: I2C���
*�� �� ֵ: ��
**********************************************************************************************************/
void HAL_I2C1_MemTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	uint8_t data;
	BaseType_t xResult;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	data = I2C_EVENT_SUCCESSFUL;
	xResult = xQueueSendFromISR(i2c_queue, &data, &xHigherPriorityTaskWoken);
	if(xResult == pdPASS)
		  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/**********************************************************************************************************
*�� �� ��: HAL_I2C_MemRxCpltCallback
*����˵��: HAL��I2C�ص�����
*��    ��: I2C���
*�� �� ֵ: ��
**********************************************************************************************************/
void HAL_I2C1_MemRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	uint8_t data;
	BaseType_t xResult;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	data = I2C_EVENT_SUCCESSFUL;
	xResult = xQueueSendFromISR(i2c_queue, &data, &xHigherPriorityTaskWoken);
	if(xResult == pdPASS)
		  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/**********************************************************************************************************
*�� �� ��: HAL_I2C_ErrorCallback
*����˵��: HAL��I2C�ص�����
*��    ��: I2C���
*�� �� ֵ: ��
**********************************************************************************************************/
void HAL_I2C1_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
	uint8_t data;
	BaseType_t xResult;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	data = I2C_EVENT_ERROR;
	xResult = xQueueSendFromISR(i2c_queue, &data, &xHigherPriorityTaskWoken);
	if(xResult == pdPASS)
		  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/**********************************************************************************************************
*�� �� ��: HAL_I2C_MasterTxCpltCallback
*����˵��: HAL��I2C�ص�����
*��    ��: I2C���
*�� �� ֵ: ��
**********************************************************************************************************/
void HAL_I2C1_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	uint8_t data;
	BaseType_t xResult;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	data = I2C_EVENT_SUCCESSFUL;
	xResult = xQueueSendFromISR(i2c_queue, &data, &xHigherPriorityTaskWoken);
	if(xResult == pdPASS)
		  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/**********************************************************************************************************
*�� �� ��: HAL_I2C_MasterRxCpltCallback
*����˵��: HAL��I2C�ص�����
*��    ��: I2C���
*�� �� ֵ: ��
**********************************************************************************************************/
void HAL_I2C1_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	uint8_t data;
	BaseType_t xResult;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	data = I2C_EVENT_SUCCESSFUL;
	xResult = xQueueSendFromISR(i2c_queue, &data, &xHigherPriorityTaskWoken);
	if(xResult == pdPASS)
		  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

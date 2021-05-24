#include "debug_uart.h"
#include "stm32f7xx_hal.h"
#include <stdio.h>

#include "FreeRTOS.h"
#include "queue.h"

//���ھ��
UART_HandleTypeDef huart1;
//���Ͷ���
QueueHandle_t tx_queue;

//����״̬
//bit15��	������ɱ�־
//bit14��	���յ�0x0d
//bit13~0��	���յ�����Ч�ֽ���Ŀ
//����״̬���
uint16_t USART_RX_STA;
//���ջ���,���USART_REC_LEN���ֽ�.
uint8_t USART_RX_BUF[USART_REC_LEN];

//���������FreeRTOS��port.c�п����ҵ�
extern uint32_t vPortGetIPSR(void);

/**********************************************************************************************************
*�� �� ��: debug_uart_init
*����˵��: ���Դ��ڳ�ʼ��
*��    ��: ��
*�� �� ֵ: ��
**********************************************************************************************************/
void debug_uart_init()
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	//�������Ͷ���
	tx_queue = xQueueCreate(64, sizeof(uint8_t));
	
	//���ʱ��ʹ��
	__HAL_RCC_USART1_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	//IO��ʼ��
    GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	//���ڳ�ʼ��
	huart1.Instance = USART1;
	huart1.Init.BaudRate = 115200;
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	HAL_UART_Init(&huart1);

	//�����е���ʼ��
	HAL_NVIC_SetPriority(USART1_IRQn, 7, 0);
	HAL_NVIC_EnableIRQ(USART1_IRQn);
	
	//���������ж�
	__HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);
}

/**********************************************************************************************************
*�� �� ��: fputc
*����˵��: �ض�����printf����
*��    ��: Ҫ���͵��ֽ� �豸�ļ�
*�� �� ֵ: ������ɵ��ֽ�
**********************************************************************************************************/
int fputc(int ch, FILE *f)
{
	if (vPortGetIPSR()) {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		//���������д��Ҫ���͵�����
        while (xQueueSendFromISR(tx_queue, &ch, &xHigherPriorityTaskWoken) != pdPASS);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    } else {
		//���������д��Ҫ���͵�����
        xQueueSend(tx_queue, &ch, 1);
    }
	//ʹ�ܷ��Ϳ��ж�
	__HAL_UART_ENABLE_IT(&huart1, UART_IT_TXE);
	return ch;
}

/**********************************************************************************************************
*�� �� ��: USART1_IRQHandler
*����˵��: ����1�жϺ���
*��    ��: ��
*�� �� ֵ: ��
**********************************************************************************************************/
void USART1_IRQHandler(void)
{
	uint8_t Res;
	//�����ж�
	if(__HAL_UART_GET_FLAG(&huart1, USART_ISR_RXNE)) {
		//��ȡ��������
		Res = huart1.Instance->RDR;
		//����δ���
		if ((USART_RX_STA & 0x8000) == 0) {
			//���յ���0x0d
			if (USART_RX_STA & 0x4000) {
				if (Res != 0x0a)
					//���մ���,���¿�ʼ
					USART_RX_STA = 0;
				else
					//���������
					USART_RX_STA |= 0x8000;
			//��û�յ�0X0D
			} else {
				if (Res == 0x0d)
					USART_RX_STA |= 0x4000;
				else {
					USART_RX_BUF[USART_RX_STA & 0X3FFF] = Res;
					USART_RX_STA++;
					if (USART_RX_STA > (USART_REC_LEN - 1))
						//�������ݴ���,���¿�ʼ����
						USART_RX_STA = 0;
				}
			}
		}
	} else if(__HAL_UART_GET_FLAG(&huart1, USART_ISR_TXE)) {
		
		BaseType_t xTaskWokenByReceive = pdFALSE;
		//���Ͷ�������������Ҫ����
		if (xQueueReceiveFromISR(tx_queue, (void *) &Res, &xTaskWokenByReceive) == pdPASS)
			huart1.Instance->TDR = Res;
		else
			//�����ݷ��;͹رշ����ж�
			__HAL_UART_DISABLE_IT(&huart1, UART_IT_TXE);
	}
}

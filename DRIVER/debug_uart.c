#include "debug_uart.h"
#include "stm32f7xx_hal.h"
#include <stdio.h>

#include "FreeRTOS.h"
#include "queue.h"

//串口句柄
UART_HandleTypeDef huart1;
//发送队列
QueueHandle_t tx_queue;

//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目
//接收状态标记
uint16_t USART_RX_STA;
//接收缓冲,最大USART_REC_LEN个字节.
uint8_t USART_RX_BUF[USART_REC_LEN];

//这个函数在FreeRTOS的port.c中可以找到
extern uint32_t vPortGetIPSR(void);

/**********************************************************************************************************
*函 数 名: debug_uart_init
*功能说明: 调试串口初始化
*形    参: 无
*返 回 值: 无
**********************************************************************************************************/
void debug_uart_init()
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	//创建发送队列
	tx_queue = xQueueCreate(64, sizeof(uint8_t));
	
	//相关时钟使能
	__HAL_RCC_USART1_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	//IO初始化
    GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	//串口初始化
	huart1.Instance = USART1;
	huart1.Init.BaudRate = 115200;
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	HAL_UART_Init(&huart1);

	//串口中单初始化
	HAL_NVIC_SetPriority(USART1_IRQn, 7, 0);
	HAL_NVIC_EnableIRQ(USART1_IRQn);
	
	//开启接受中断
	__HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);
}

/**********************************************************************************************************
*函 数 名: fputc
*功能说明: 重定义至printf函数
*形    参: 要发送的字节 设备文件
*返 回 值: 发送完成的字节
**********************************************************************************************************/
int fputc(int ch, FILE *f)
{
	if (vPortGetIPSR()) {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		//向队列里面写入要发送的数据
        while (xQueueSendFromISR(tx_queue, &ch, &xHigherPriorityTaskWoken) != pdPASS);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    } else {
		//向队列里面写入要发送的数据
        xQueueSend(tx_queue, &ch, 1);
    }
	//使能发送空中断
	__HAL_UART_ENABLE_IT(&huart1, UART_IT_TXE);
	return ch;
}

/**********************************************************************************************************
*函 数 名: USART1_IRQHandler
*功能说明: 串口1中断函数
*形    参: 无
*返 回 值: 无
**********************************************************************************************************/
void USART1_IRQHandler(void)
{
	uint8_t Res;
	//接收中断
	if(__HAL_UART_GET_FLAG(&huart1, USART_ISR_RXNE)) {
		//读取串口数据
		Res = huart1.Instance->RDR;
		//接收未完成
		if ((USART_RX_STA & 0x8000) == 0) {
			//接收到了0x0d
			if (USART_RX_STA & 0x4000) {
				if (Res != 0x0a)
					//接收错误,重新开始
					USART_RX_STA = 0;
				else
					//接收完成了
					USART_RX_STA |= 0x8000;
			//还没收到0X0D
			} else {
				if (Res == 0x0d)
					USART_RX_STA |= 0x4000;
				else {
					USART_RX_BUF[USART_RX_STA & 0X3FFF] = Res;
					USART_RX_STA++;
					if (USART_RX_STA > (USART_REC_LEN - 1))
						//接收数据错误,重新开始接收
						USART_RX_STA = 0;
				}
			}
		}
	} else if(__HAL_UART_GET_FLAG(&huart1, USART_ISR_TXE)) {
		
		BaseType_t xTaskWokenByReceive = pdFALSE;
		//发送队列中有数据需要发送
		if (xQueueReceiveFromISR(tx_queue, (void *) &Res, &xTaskWokenByReceive) == pdPASS)
			huart1.Instance->TDR = Res;
		else
			//无数据发送就关闭发送中断
			__HAL_UART_DISABLE_IT(&huart1, UART_IT_TXE);
	}
}

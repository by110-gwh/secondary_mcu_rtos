#include "motor_encode.h"
#include "stm32f7xx_hal.h"

static QueueHandle_t motor_encode_lf_queue;
static QueueHandle_t motor_encode_lb_queue;
static QueueHandle_t motor_encode_rf_queue;
static QueueHandle_t motor_encode_rb_queue;

static TIM_HandleTypeDef htim3;
static TIM_HandleTypeDef htim5;

static uint16_t lf_last_cnt;
static uint16_t lf_last_updata_cnt;
static uint16_t lb_last_cnt;
static uint16_t lb_last_updata_cnt;
static uint16_t rf_last_cnt;
static uint16_t rf_last_updata_cnt;
static uint16_t rb_last_cnt;
static uint16_t rb_last_updata_cnt;
static uint32_t tim3_updata_cnt;
static uint32_t tim5_updata_cnt;

/**********************************************************************************************************
*函 数 名: motor_encode_init
*功能说明: 电机编码器接口初始化
*形    参: 无
*返 回 值: 无
**********************************************************************************************************/
void motor_encode_init()
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_IC_InitTypeDef sConfigIC = {0};
	
    //创建通讯队列
    motor_encode_lf_queue = xQueueCreate(16, sizeof(int32_t));
    motor_encode_lb_queue = xQueueCreate(16, sizeof(int32_t));
    motor_encode_rf_queue = xQueueCreate(16, sizeof(int32_t));
    motor_encode_rb_queue = xQueueCreate(16, sizeof(int32_t));
    
    //时钟使能
    __HAL_RCC_TIM5_CLK_ENABLE();
    __HAL_RCC_TIM3_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	
    //GPIO配置
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM5;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    //定时器3输入捕获使能
    htim3.Instance = TIM3;
    htim3.Init.Prescaler = 108-1;
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = 10000;
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
	HAL_TIM_Base_Init(&htim3);
	
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig);
    
    sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_FALLING;
    sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
    sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
    sConfigIC.ICFilter = 0;
    HAL_TIM_IC_ConfigChannel(&htim3, &sConfigIC, TIM_CHANNEL_1);
    HAL_TIM_IC_ConfigChannel(&htim3, &sConfigIC, TIM_CHANNEL_2);
    HAL_TIM_IC_ConfigChannel(&htim3, &sConfigIC, TIM_CHANNEL_3);
    HAL_TIM_IC_ConfigChannel(&htim3, &sConfigIC, TIM_CHANNEL_4);
    
    HAL_TIM_Base_Start_IT(&htim3);
    HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_1);
//    HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_2);
    HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_3);
//    HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_4);
    
    //定时器5输入捕获使能
    htim5.Instance = TIM5;
    htim5.Init.Prescaler = 108-1;
    htim5.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim5.Init.Period = 10000;
    htim5.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim5.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
	HAL_TIM_Base_Init(&htim5);
	
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    HAL_TIMEx_MasterConfigSynchronization(&htim5, &sMasterConfig);
    
    sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_FALLING;
    sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
    sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
    sConfigIC.ICFilter = 0;
    HAL_TIM_IC_ConfigChannel(&htim5, &sConfigIC, TIM_CHANNEL_1);
    HAL_TIM_IC_ConfigChannel(&htim5, &sConfigIC, TIM_CHANNEL_2);
    HAL_TIM_IC_ConfigChannel(&htim5, &sConfigIC, TIM_CHANNEL_3);
    HAL_TIM_IC_ConfigChannel(&htim5, &sConfigIC, TIM_CHANNEL_4);
    
    HAL_TIM_Base_Start_IT(&htim5);
    HAL_TIM_IC_Start_IT(&htim5, TIM_CHANNEL_1);
//    HAL_TIM_IC_Start_IT(&htim5, TIM_CHANNEL_2);
    HAL_TIM_IC_Start_IT(&htim5, TIM_CHANNEL_3);
//    HAL_TIM_IC_Start_IT(&htim5, TIM_CHANNEL_4);
    
    //中断控制器使能
    __HAL_TIM_CLEAR_FLAG(&htim3, TIM_CHANNEL_1);
    __HAL_TIM_CLEAR_FLAG(&htim3, TIM_CHANNEL_3);
    __HAL_TIM_CLEAR_FLAG(&htim3, TIM_FLAG_UPDATE);
    HAL_NVIC_SetPriority(TIM3_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(TIM3_IRQn);
    HAL_NVIC_SetPriority(TIM5_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(TIM5_IRQn);
}

/**********************************************************************************************************
*函 数 名: motor_encode_get
*功能说明: 获取电机编码器值
*形    参: 左前编码器值 左后编码器值 右前编码器值 右后编码器值
*返 回 值: 无
**********************************************************************************************************/
void motor_encode_get(float *encode_lf, float *encode_lb, float *encode_rf, float *encode_rb)
{
    static float last_encode_lf;
    static float last_encode_lb;
    static float last_encode_rf;
    static float last_encode_rb;
    int motor_encode;
    int motor_encode_sum;
    int motor_encode_cnt;
    
    //右前方电机
    motor_encode_sum = 0;
    motor_encode_cnt = 0;
    while (xQueueReceive(motor_encode_rf_queue, &motor_encode, 0) == pdPASS) {
        if (motor_encode == 0xFFFFFFFF) {
            last_encode_rf = 0;
            break;
        }
        motor_encode_sum += motor_encode;
        motor_encode_cnt++;
    }
    if (motor_encode_cnt) {
        motor_encode_sum /= motor_encode_cnt;
        last_encode_rf = 1000000.0 / motor_encode_sum;
    }
    //右后方电机
    motor_encode_sum = 0;
    motor_encode_cnt = 0;
    while (xQueueReceive(motor_encode_rb_queue, &motor_encode, 0) == pdPASS) {
        if (motor_encode == 0xFFFFFFFF) {
            last_encode_rb = 0;
            break;
        }
        motor_encode_sum += motor_encode;
        motor_encode_cnt++;
    }
    if (motor_encode_cnt) {
        motor_encode_sum /= motor_encode_cnt;
        last_encode_rb = 1000000.0 / motor_encode_sum;
    }
    //左前方电机
    motor_encode_sum = 0;
    motor_encode_cnt = 0;
    while (xQueueReceive(motor_encode_lf_queue, &motor_encode, 0) == pdPASS) {
        if (motor_encode == 0xFFFFFFFF) {
            last_encode_lf = 0;
            break;
        }
        motor_encode_sum += motor_encode;
        motor_encode_cnt++;
    }
    if (motor_encode_cnt) {
        motor_encode_sum /= motor_encode_cnt;
        last_encode_lf = 1000000.0 / motor_encode_sum;
    }
    //左后方电机
    motor_encode_sum = 0;
    motor_encode_cnt = 0;
    while (xQueueReceive(motor_encode_lb_queue, &motor_encode, 0) == pdPASS) {
        if (motor_encode == 0xFFFFFFFF) {
            last_encode_lb = 0;
            break;
        }
        motor_encode_sum += motor_encode;
        motor_encode_cnt++;
    }
    if (motor_encode_cnt) {
        motor_encode_sum /= motor_encode_cnt;
        last_encode_lb = 1000000.0 / motor_encode_sum;
    }
    
    *encode_lf = last_encode_lf;
    *encode_lb = last_encode_lb;
    *encode_rf = last_encode_rf;
    *encode_rb = last_encode_rb;
}

/**********************************************************************************************************
*函 数 名: TIM3_IRQHandler
*功能说明: 定时器3中断函数
*形    参: 无
*返 回 值: 无
**********************************************************************************************************/
void TIM3_IRQHandler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    uint16_t current_cnt;
    //定时器通道1捕获中断
	if (__HAL_TIM_GET_FLAG(&htim3, TIM_FLAG_CC1) != RESET) {
        int32_t motor_speed_lf;
        current_cnt = __HAL_TIM_GetCompare(&htim3, TIM_CHANNEL_1);
        //计算时间差
        if (__HAL_TIM_GET_FLAG(&htim3, TIM_IT_UPDATE) && current_cnt < 5000) {
            motor_speed_lf = (tim3_updata_cnt - lf_last_updata_cnt + 1) * 10000 + current_cnt - lf_last_cnt;
        } else {
            motor_speed_lf = (tim3_updata_cnt - lf_last_updata_cnt) * 10000 + current_cnt - lf_last_cnt;
        }
        if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_7) == GPIO_PIN_RESET)
            motor_speed_lf = -motor_speed_lf;
        lf_last_cnt = current_cnt;
        lf_last_updata_cnt = tim3_updata_cnt;
        //通过队列发送编码器值
        xQueueSendFromISR(motor_encode_lf_queue, &motor_speed_lf, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        __HAL_TIM_CLEAR_FLAG(&htim3, TIM_FLAG_CC1);
    }
    //定时器通道3捕获中断
	if (__HAL_TIM_GET_FLAG(&htim3, TIM_FLAG_CC3) != RESET) {
        int32_t motor_speed_lb;
        current_cnt = __HAL_TIM_GetCompare(&htim3, TIM_CHANNEL_3);
        //计算时间差
        if (__HAL_TIM_GET_FLAG(&htim3, TIM_IT_UPDATE) && current_cnt < 5000) {
            motor_speed_lb = (tim3_updata_cnt - lb_last_updata_cnt + 1) * 10000 + current_cnt - lb_last_cnt;
        } else {
            motor_speed_lb = (tim3_updata_cnt - lb_last_updata_cnt) * 10000 + current_cnt - lb_last_cnt;
        }
        if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) == GPIO_PIN_RESET)
            motor_speed_lb = -motor_speed_lb;
        lb_last_cnt = current_cnt;
        lb_last_updata_cnt = tim3_updata_cnt;
        //通过队列发送编码器值
        xQueueSendFromISR(motor_encode_lb_queue, &motor_speed_lb, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        __HAL_TIM_CLEAR_FLAG(&htim3, TIM_FLAG_CC3);
    }
    //定时器溢出中断
	if (__HAL_TIM_GET_FLAG(&htim3, TIM_FLAG_UPDATE) != RESET) {
        tim3_updata_cnt++;
        if (tim3_updata_cnt - lb_last_updata_cnt >= 4) {
            //通过队列发送编码器值
            uint64_t send_data = 0xFFFFFFFF;
            xQueueSendFromISR(motor_encode_lb_queue, &send_data, &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
        if (tim3_updata_cnt - lf_last_updata_cnt >= 4) {
            //通过队列发送编码器值
            uint64_t send_data = 0xFFFFFFFF;
            xQueueSendFromISR(motor_encode_lf_queue, &send_data, &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
        __HAL_TIM_CLEAR_FLAG(&htim3, TIM_FLAG_UPDATE);
    }
}


/**********************************************************************************************************
*函 数 名: TIM5_IRQHandler
*功能说明: 定时器5中断函数
*形    参: 无
*返 回 值: 无
**********************************************************************************************************/
void TIM5_IRQHandler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    uint16_t current_cnt;
    //定时器通道1捕获中断
	if (__HAL_TIM_GET_FLAG(&htim5, TIM_FLAG_CC1) != RESET) {
        int32_t motor_speed_rf;
        current_cnt = __HAL_TIM_GetCompare(&htim5, TIM_CHANNEL_1);
        //计算时间差
        if (__HAL_TIM_GET_FLAG(&htim5, TIM_IT_UPDATE) && current_cnt < 5000) {
            motor_speed_rf = (tim5_updata_cnt - rf_last_updata_cnt + 1) * 10000 + current_cnt - rf_last_cnt;
        } else {
            motor_speed_rf = (tim5_updata_cnt - rf_last_updata_cnt) * 10000 + current_cnt - rf_last_cnt;
        }
        if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1) == GPIO_PIN_SET)
            motor_speed_rf = -motor_speed_rf;
        rf_last_cnt = current_cnt;
        rf_last_updata_cnt = tim5_updata_cnt;
        //通过队列发送编码器值
        xQueueSendFromISR(motor_encode_rf_queue, &motor_speed_rf, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        __HAL_TIM_CLEAR_FLAG(&htim5, TIM_FLAG_CC1);
    }
    //定时器通道3捕获中断
	if (__HAL_TIM_GET_FLAG(&htim5, TIM_FLAG_CC3) != RESET) {
        int32_t motor_speed_rb;
        current_cnt = __HAL_TIM_GetCompare(&htim5, TIM_CHANNEL_3);
        //计算时间差
        if (__HAL_TIM_GET_FLAG(&htim5, TIM_IT_UPDATE) && current_cnt < 5000) {
            motor_speed_rb = (tim5_updata_cnt - rb_last_updata_cnt + 1) * 10000 + current_cnt - rb_last_cnt;
        } else {
            motor_speed_rb = (tim5_updata_cnt - rb_last_updata_cnt) * 10000 + current_cnt - rb_last_cnt;
        }
        if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3) == GPIO_PIN_SET)
            motor_speed_rb = -motor_speed_rb;
        rb_last_cnt = current_cnt;
        rb_last_updata_cnt = tim5_updata_cnt;
        //通过队列发送编码器值
        xQueueSendFromISR(motor_encode_rb_queue, &motor_speed_rb, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        __HAL_TIM_CLEAR_FLAG(&htim5, TIM_FLAG_CC3);
    }
    //定时器溢出中断
	if (__HAL_TIM_GET_FLAG(&htim5, TIM_FLAG_UPDATE) != RESET) {
        tim5_updata_cnt++;
        if (tim5_updata_cnt - rb_last_updata_cnt >= 4) {
            //通过队列发送编码器值
            uint64_t send_data = 0xFFFFFFFF;
            xQueueSendFromISR(motor_encode_rb_queue, &send_data, &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
        if (tim5_updata_cnt - rf_last_updata_cnt >= 4) {
            //通过队列发送编码器值
            uint64_t send_data = 0xFFFFFFFF;
            xQueueSendFromISR(motor_encode_rf_queue, &send_data, &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
        __HAL_TIM_CLEAR_FLAG(&htim5, TIM_FLAG_UPDATE);
    }
}


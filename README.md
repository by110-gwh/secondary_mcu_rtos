# robot cotroller
# 小柴
GDUT robot team control board MCU program template with RTOS 
----------
#USMART组件
####USMART是正点原子开发的stm32调试工具，可以通过串口调用相应函数并执行。 
**1、USMART组件移植** 
　　USMART组件共五个文件分别为 
　　usmart.c－－－－－－－－－负责与外部互交等 
　　usmart.h－－－－－－－－－包含了用户配置宏定义 
　　usmart_config.c－－－－－－调用函数管理 
　　usmart_str.c、usmart_str.h －命令参数解析 
将c文件添加进工程、h文件添加包含路径 
**2、使用** 
（1）在main函数中进行初始化 
　　　usmart_dev.init(u8 sysclk);//sysclk为系统时钟（f4-168，f1-72） 
（2）在usmart_config中对调用函数进行管理 
　　　例：(void*)usmartled,“void USMART_LED(uint8_t flag)”, 
　　　　　//第一个参数为函数指针，第二个参数为函数声明 
（3）串口使用 
　　　1）波特率 
　　　2）发送新行（看代码是通过判断0xa开始接受,0x0d结束接受） 
　　　　发送USMART_LED(1) 

----------
#AHRS陀螺仪姿态解算组件
####ahrs是参考无名飞控的mpu6050姿态解算算法
**使用方法：** 
1.变量**Pitch、Roll、Yaw**（位于MODULE/ahrs.h中）是解算好的姿态角。
2.通过USMART组件调用函数gyro_calibration()实现校准 
3.通过USMART组件调用函数accel_calibration(校准方向)进行加速计六面校准 
　　　如主控上面分别朝向上下左右前后依次调用accel_calibration(1)、accel_calibration(2)、accel_calibration(3)、accel_calibration(4)、accel_calibration(5)、accel_calibration(6)、
###注意：校准时务必让主控处于静止状态

----------
#舵机控制组件
####舵机控制组件简称舵控，是通过上位机对舵机进行动作组编排，下载到单片机上运行的组件
**使用方法：** 
1.上位机部分参见幻尔科技的上位机
2.单片机运行动作组：
　　　调用函数：void steering_action_task_create(uint8_t action_group, uint16_t run_cnt)运行动作组。（位于TASK/steering_action_task.h）
　　　形参: 动作组编号 重复运行次数
3.动作组执行完毕后steering_action_task_exit将会置1。（位于TASK/steering_action_task.h）
4.动作组停止运行：
　　　将变量steering_action_task_exit置1，动作组将会停止运行。（位于TASK/steering_action_task.h）

----------
#陀螺仪温控组件
####陀螺仪温控组件用于控制加热电路，使陀螺仪一直处于50摄氏度的温度下工作
代码位于TASK/imu_temp_task.c 
陀螺仪当前温度：tempDataFilter（位于MODULE/imu.h）
PID参数可在TASK/imu_temp_task.c中调整
 
----------
#参数编程组件
####参数保存组件用于保存单片机掉电保存的数据
代码位于MODULE/TASK/paramer_save.c
使用方法：
　　　1.向paramer_save_t添加要保存的变量
　　　2.在void save_paramer_init()函数中添加该变量的初始化代码
　　　3.保存的时候修改该变量
　　　4.调用write_save_paramer()函数保存到flash中
　　　5.单片机程序开始执行的时候，该变量会从flash中读出




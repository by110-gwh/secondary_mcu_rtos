#include "usmart.h"
#include "usmart_str.h"

		void angle_control_pid_set(uint8_t p, uint8_t i, uint8_t d);
		void gyro_control_pid_set(uint8_t p, uint8_t i, uint8_t d);
		void HAL_Delay(uint32_t Delay);
//函数名列表初始化(用户自己添加)
//用户直接在这里输入要执行的函数名及其查找串
struct _m_usmart_nametab usmart_nametab[] = {
//如果使能了读写操作
#if USMART_USE_WRFUNS==1
	(void*)read_addr,"u32 read_addr(u32 addr)",
	(void*)write_addr,"void write_addr(u32 addr,u32 val)",
	(void*)HAL_Delay,"void HAL_Delay(uint32_t Delay)",

#endif		   
							
};						  

//函数控制管理器初始化得到各个受控函数的名字得到函数总数量
struct _m_usmart_dev usmart_dev = {
	usmart_nametab,
	usmart_init,
	usmart_cmd_rec,
	usmart_exe,
	usmart_scan,
	//函数数量
	sizeof(usmart_nametab)/sizeof(struct _m_usmart_nametab),
	//参数数量
	0,
	//函数ID
	0,
	//参数显示类型,0,10进制;1,16进制
	1,
	//参数类型.bitx:,0,数字;1,字符串
	0,
	//每个参数的长度暂存表,需要MAX_PARM个0初始化
	0,
	//函数的参数,需要PARM_LEN个0初始化
	0,
};   




















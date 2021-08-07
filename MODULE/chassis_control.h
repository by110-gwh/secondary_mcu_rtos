/************************************************************************
  *
  * FileName   : chassis_control.h
  * Version    : v1.0
  * Author     : 桃子
  * Date       : 2021-08-06
  * Description:
  * Function List:
  	1. ....
  	   <version>:
  <modify staff>:
  		  <data>:
   <description>:
  	2. ...
*******************************************************************************/



#ifndef __CHASSIS_CONTROL_H_
#define __CHASSIS_CONTROL_H_
#include "math.h"
#define ABS(x) ( (x)>0?(x):-(x) )
typedef enum
{
    hor = 0, //horizontal
    ver //vertically
} direct_t;
#include "tim_control.h"
void move_by_encoder(direct_t direct, int val);
void direct_move(direct_t direct, int line_num);
void turn_angle(int target_angle);
#endif





#include "pid.h"

#define ABS(X)  (((X) > 0)? (X) : -(X))

/**********************************************************************************************************
*�� �� ��: pid_control
*����˵��: pid����������
*��    ��: pid���������ݽṹ�� pid����������
*�� �� ֵ: �����
**********************************************************************************************************/
float pid_control(pid_data_t *data, pid_paramer_t *para)
{
	float controller_dt;
    //��·ֱ������ڴ�ֵ
	if (data->short_circuit_flag) {
		data->control_output = data->expect;
		return data->control_output;
	}
	//��ȡdt
	Get_Time_Period(&data->pid_controller_dt);
	controller_dt = data->pid_controller_dt.Time_Delta / 1000000.0;
	//��һ�μ�����ʱ�佫���ּ��ʱ��ܴ�����
	if (controller_dt < 0.001f)
		return 0;
	//�����ϴ�ƫ��
	data->last_err = data->err;
	//������ȥ�����õ�ƫ��			  
	data->err = data->expect - data->feedback;
	//����ƫ��΢��
	data->dis_err = data->err - data->last_err;
	//�Զ���ƫ��΢�ִ���
	if (data->err_callback)
		data->err_callback(data, para);
	//�����޷�
	if (para->integrate_max) {
		if (data->integrate >= para->integrate_max)
			data->integrate = para->integrate_max;
		if (data->integrate <= -para->integrate_max)
			data->integrate = -para->integrate_max;
	}
    data->integrate += para->ki * data->err * controller_dt;
	//���������
	data->control_output = para->kp * data->err
		+ data->integrate
		+ para->kd * data->dis_err;
	//������޷�
	if (para->control_output_limit) {
		if (data->control_output >= para->control_output_limit)
			data->control_output = para->control_output_limit;
		if (data->control_output <= -para->control_output_limit)
			data->control_output = -para->control_output_limit;
	}
	//���������
	return data->control_output;
}

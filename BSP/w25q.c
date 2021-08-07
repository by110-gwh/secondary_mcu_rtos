#include "w25q.h"
#include "qspi.h"

#include "FreeRTOS.h"
#include "task.h"

#define W25X_SECTOR_ERASE 0x20
#define W25X_PAGE_PROGRAM 0x02
#define W25X_FAST_READ_DATA 0x0B
#define W25X_WRITE_ENABLE 0x06
#define W25X_READ_STATUS_REG1 0x05
#define W25X_READ_STATUS_REG2 0x35
#define W25X_READ_STATUS_REG3 0x15

/**********************************************************************************************************
*�� �� ��: w25q_read_sr
*����˵��: ��ȡW25QXX��״̬�Ĵ�����W25QXXһ����3��״̬�Ĵ���
*��    ��: ״̬�Ĵ������
*�� �� ֵ: ״̬�Ĵ�����ֵ
**********************************************************************************************************/
uint8_t w25q_read_sr(uint8_t regno)
{  
	uint8_t byte, command; 
    switch(regno) {
    case 1:
        //��״̬�Ĵ���1ָ��
        command = W25X_READ_STATUS_REG1;
        break;
    case 2:
        //��״̬�Ĵ���2ָ��
        command = W25X_READ_STATUS_REG2;
        break;
    case 3:
        //��״̬�Ĵ���3ָ��
        command = W25X_READ_STATUS_REG3;
        break;
    default:
        command = W25X_READ_STATUS_REG1;
        break;
    }
    spi_send_cmd(command, 0, 0, 0, 1);
	qspi_receive(&byte, 1);
	return byte;
}
 
/**********************************************************************************************************
*�� �� ��: w25q_write_enable
*����˵��: W25QXXдʹ��
*��    ��: ��
*�� �� ֵ: ��
**********************************************************************************************************/
void w25q_write_enable(void)
{
    spi_send_cmd(W25X_WRITE_ENABLE, 0, 0, 0, 0);
} 

/**********************************************************************************************************
*�� �� ��: w25q_read
*����˵��: ��ָ����ַ��ʼ��ȡָ�����ȵ�����
*��    ��: ���ݴ洢�� ��ʼ��ȡ�ĵ�ַ Ҫ��ȡ���ֽ���
*�� �� ֵ: ��
**********************************************************************************************************/
void w25q_read(uint8_t* pBuffer, uint32_t addr, uint16_t cnt)
{
    spi_send_cmd(W25X_FAST_READ_DATA, addr, 1, 8, cnt);
	qspi_receive(pBuffer,cnt); 
}  

/**********************************************************************************************************
*�� �� ��: w25q_write_page
*����˵��: ��ָ����ַ��ʼд�����256�ֽڵ�����
*��    ��: ���ݴ洢�� ��ʼд��ĵ�ַ Ҫд����ֽ���
*�� �� ֵ: ��
**********************************************************************************************************/
void w25q_write_page(uint8_t *pBuffer, uint32_t addr, uint16_t cnt)
{
    //дʹ��
	w25q_write_enable();
    spi_send_cmd(W25X_PAGE_PROGRAM, addr, 1, 0, cnt);
	qspi_transmit(pBuffer, cnt);
    //�ȴ�д�����
	w25q_wait_busy();
} 


/**********************************************************************************************************
*�� �� ��: w25q_write
*����˵��: �޼���дSPI FLASH
*��    ��: ���ݴ洢�� ��ʼд��ĵ�ַ Ҫд����ֽ���
*�� �� ֵ: ��
**********************************************************************************************************/
void w25q_write(uint8_t *pBuffer, uint32_t addr, uint16_t cnt)
{
	uint16_t pageremain;
    //��ҳʣ����ֽ���
	pageremain = 256 - addr % 256;
    //������256���ֽ�
	if (cnt <= pageremain)
        pageremain = cnt;
	while(1) {
		w25q_write_page(pBuffer, addr, pageremain);
        //д�������
		if (cnt == pageremain)
            break;
	 	else {
			pBuffer += pageremain;
			addr += pageremain;	

            //��ȥ�Ѿ�д���˵��ֽ���
			cnt -= pageremain;			  
			if(cnt > 256)
                //һ�ο���д��256���ֽ�
                pageremain = 256;
			else
                //����256���ֽ���
                pageremain = cnt;
		}
	}
} 

/**********************************************************************************************************
*�� �� ��: w25q_erase_sector
*����˵��: ����һ������
*��    ��: Ҫ������ַ
*�� �� ֵ: ��
**********************************************************************************************************/
void w25q_erase_sector(uint32_t addr)   
{
	w25q_write_enable();
	w25q_wait_busy();
    spi_send_cmd(W25X_SECTOR_ERASE, addr, 1, 0, 0);
    //�ȴ��������
	w25q_wait_busy();
}

/**********************************************************************************************************
*�� �� ��: w25q_erase
*����˵��: ��������
*��    ��: Ҫ������ַ ����
*�� �� ֵ: ��
**********************************************************************************************************/
void w25q_erase(uint32_t addr, uint32_t cnt)
{
    uint16_t pageremain;
    //������ʣ����ֽ���
	pageremain = 4096 - addr % 4096;
    //������4096���ֽ�
	if (cnt <= pageremain)
        pageremain = cnt;
	while(1) {
		w25q_erase_sector(addr);
        //����������
		if (cnt == pageremain)
            break;
	 	else {
			addr += pageremain;

            //��ȥ�Ѿ��������ֽ���
			cnt -= pageremain;
			if(cnt > 256)
                //һ�ο��Բ���4096���ֽ�
                pageremain = 4096;
			else
                //����4096���ֽ���
                pageremain = cnt;
		}
	}
}

/**********************************************************************************************************
*�� �� ��: w25q_wait_busy
*����˵��: �ȴ�w25q�������
*��    ��: ��
*�� �� ֵ: ��
**********************************************************************************************************/
void w25q_wait_busy(void)
{
    //�ȴ�BUSYλ���
    while((w25q_read_sr(1) & 0x01) == 0x01)
        vTaskDelay(1);
}

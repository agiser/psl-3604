//----------------------------------------------------------------------------

//������ ��������� �����, ������������ ����

//----------------------------------------------------------------------------

#ifndef PORT_H
#define PORT_H

#include "wakeport.h"
#include "data.h"

//----------------------------- ���������: -----------------------------------

#define BAUD_RATE       19200  //�������� ������, ���
#define FRAME_SIZE         16  //������������ ������ ������, ����

#define PAR_COUNT          23  //���������� ����������
#define PAR_NON           255  //������ ��� ������������� ����������

//������� ���������� ��� ������ CMD_SET_PAR � CMD_GET_PAR:
const uint8_t ParIdx[PAR_COUNT] =
{
  PAR_LOCK, //Lock controls
  PAR_OVP,  //OVP threshold
  PAR_OCP,  //OCP threshold
  PAR_OPP,  //OPP threshold
  PAR_DEL,  //OVP/OCP delay
  PAR_OTP,  //OTP threshold
  PAR_FNL,  //Fan start temperature
  PAR_FNH,  //Fan full speed temperature
  PAR_TRC,  //Track (OFF/AUTOLOCK/ON)
  PAR_CON,  //Confirm (OFF/ON)
  PAR_POW,  //Display power (OFF/ON)
  PAR_SET,  //Display setpoint when regulated (OFF/ON)
  PAR_GET,  //Always display maesured values(OFF/ON)
  PAR_APV,  //Display average/peak V (AVERAGE/PEAK HIGH/PEAK LOW)
  PAR_APC,  //Display average/peak I (AVERAGE/PEAK HIGH/PEAK LOW)
  PAR_PRC,  //Current preview (OFF/ON)
  PAR_DNP,  //Down programmer (OFF/ON)
  PAR_OUT,  //Restore out state (OFF/ON)
  PAR_SND,  //Sound (OFF/ALARM/ON)
  PAR_ENR,  //Encoder reverse (OFF/ON)
  PAR_SPL,  //Splash screen (OFF/ON)
  PAR_INF,  //Firmware version info
  PAR_TIM   //Timer interval
};

//----------------------------------------------------------------------------
//------------------------------ ����� TPort ---------------------------------
//----------------------------------------------------------------------------

class TPort
{
private:
public:
  TWakePort *WakePort;
  TPort(void);
  void Execute(void);
};

//----------------------------------------------------------------------------
//----------------------------- ���� ������: ---------------------------------
//----------------------------------------------------------------------------

#define CMD_SET_VI 6 //��������� ���������� � ����

  //TX: word V, word I, byte S
  //RX: byte Err

  //V = 0..VMAX - ����������, x0.01 �
  //I = 0..IMAX - ���, x0.001 �
  //S = 0 - ����� ��������, 1 - ����� �������
  //Err = ERR_NO

#define CMD_GET_VI 7 //������ �������������� ���������� � ����

  //TX:
  //RX: byte Err, word V, word I

  //V = 0..VMAX - ����������, x0.01 �
  //I = 0..IMAX - ���, x0.001 �
  //Err = ERR_NO

#define CMD_GET_STAT 8 //������ ������� ���������

  //TX:
  //RX: byte Err, byte S

  //S.0 = 1 - ����� �������
  //S.1 = 1 - CV
  //S.2 = 1 - CC
  //S.3 = 1 - OVP
  //S.4 = 1 - OCP
  //S.5 = 1 - OPP
  //S.6 = 1 - OTP
  //Err = ERR_NO

#define CMD_GET_VI_AVG 9 //������ �������� ����������� ���������� � ����

  //TX:
  //RX: byte Err, word VA, word IA

  //VA = 0..VMAX - ����������, x0.01 �
  //IA = 0..IMAX - ���, x0.001 �
  //Err = ERR_NO

#define CMD_GET_VI_FAST 10 //������ ����������� ����������� ���������� � ����

  //TX:
  //RX: byte Err, word VF, word IF

  //VF = 0..VMAX - ����������, x0.01 �
  //IF = 0..IMAX - ���, x0.001 �
  //Err = ERR_NO

#define CMD_SET_VIP_MAX 11 //��������� ����. ����������, ���� � ��������

  //TX: word VM, word IM, word PM
  //RX: byte Err

  //VM = 1000..9999 - ����������, x0.01 �
  //IM = 1000..9999 - ���, x0.001 �
  //PM = 10..9999 - ��������, x0.1 ��
  //Err = ERR_NO

#define CMD_GET_VIP_MAX 12 //������ ����. ����������, ���� � ��������

  //TX:
  //RX: byte Err, word VM, word IM, word PM

  //VM = 1000..9999 - ����������, x0.01 �
  //IM = 1000..9999 - ���, x0.001 �
  //PM = 10..9999 - ��������, x0.1 ��
  //Err = ERR_NO

#define CMD_SET_PRE 13 //������ �������

  //TX: byte N, word V, word I
  //RX: byte Err

  //N = 0..9 - ����� �������
  //V = 0..VMAX - ����������, x0.01 �
  //I = 0..IMAX - ���, x0.001 �
  //Err = ERR_NO, ERR_PA

#define CMD_GET_PRE 14 //������ �������

  //TX: byte N
  //RX: byte Err, word V, word I

  //N = 0..9 - ����� �������
  //V = 0..VMAX - ����������, x0.01 �
  //I = 0..IMAX - ���, x0.001 �
  //Err = ERR_NO, ERR_PA

#define CMD_SET_PAR 15 //��������� ���������

  //TX: byte N, word P
  //RX: byte Err

  //N - ����� ��������� (��. ������� ����������)
  //P - �������� ��������� (��. ������� ����������)
  //Err = ERR_NO, ERR_PA

#define CMD_GET_PAR 16 //������ ���������

  //TX: byte N
  //RX: byte Err, word P

  //N - ����� ��������� (��. ������� ����������)
  //P - �������� ��������� (��. ������� ����������)
  //Err = ERR_NO, ERR_PA

#define CMD_GET_FAN 17 //������ �������� ����������� � �����������

  //TX:
  //RX: byte Err, byte S, word T

  //S = 0..100 - �������� �����������, %
  //T = 0..999 - �����������, x0.1�C
  //Err = ERR_NO

#define CMD_SET_DAC 18 //��������� ���� ���

  //TX: word DACV, word DACI
  //RX: byte Err

  //DACV = 0..65520 - ��� ��� ����������
  //DACI = 0..65520 - ��� ��� ����
  //Err = ERR_NO

#define CMD_GET_ADC 19 //������ ���� ���

  //TX:
  //RX: byte Err, word ADCV, word ADCI

  //ADCV = 0..65520 - ��� ��� ����������
  //ADCI = 0..65520 - ��� ��� ����
  //Err = ERR_NO

#define CMD_SET_CAL 20 //��������� �������������� ������������

  //TX: byte N, word K
  //RX: byte Err

  //N - ����� ������������ (��. ������� �������������)
  //K - �������� ������������ (��. ������� �������������)
  //Err = ERR_NO, ERR_PA

#define CMD_GET_CAL 21 //������ �������������� ������������

  //TX: byte N
  //RX: byte Err, word K

  //N - ����� ������������ (��. ������� �������������)
  //K - �������� ������������ (��. ������� �������������)
  //Err = ERR_NO, ERR_PA

//----------------------------------------------------------------------------

#endif

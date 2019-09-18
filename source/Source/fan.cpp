//----------------------------------------------------------------------------

//������ ���������� ������������

//----------------------- ������������ �������: ------------------------------

//���������� ����� �������� ������ (����������, ����, ��������, �����������)
//��������� � ����� PB14 (����� ������� TIM15 CH1). �������� ������� - �������.
//������� ��� ��������������� ����������� ���������, ����� � ��� ����� �������
//��������� ����������� ���������� ����������� ��� ��������.
//��� ���������� ������������ �������� ��� �� 0 �� 100, ��� �������������
//�������� �������� �� 0 �� 100%. �������� ������ �������� �� ���������� V2,
//������� ������ ���� ������������ ���������� ��������� �����������. �������
//����� ����� �������� ��� ���������� ����� �������, ����� 100-� ��������
//��������������� ������������ ���������� ����������� FAN_V_MAX.

//----------------------------------------------------------------------------

#include "main.h"
#include "fan.h"
#include "data.h"
#include "therm.h"

//------------------------------- ���������: ---------------------------------

#define V2        22.5 //������� ���������� V2, �
#define FAN_MAX_V 12.0 //������������ ���������� ������� �����������, �
#define FAN_MAX_PWM ((uint16_t)(FAN_MAX_SPEED * V2 / FAN_MAX_V))

enum { FAN_OFF, FAN_RUN }; //��������� �����������

//----------------------------------------------------------------------------
//------------------------------ ����� TFan: ---------------------------------
//----------------------------------------------------------------------------

//----------------------------- �����������: ---------------------------------

TFan::TFan(void)
{
  RCC->APB2ENR |= RCC_APB2ENR_TIM15EN;   //��������� ������������ TIM15
  TIM15->PSC = 0;                        //���������
  TIM15->ARR = FAN_MAX_PWM;              //������
  TIM15->CCR1 = 0;                       //��� PWM
  TIM15->CCMR1 =
    TIM_CCMR1_OC1M_0 * 6 |               //PWM mode 1
    TIM_CCMR1_OC1PE  * 1;                //CCR1 preload enable
  TIM15->CCER =
    TIM_CCER_CC1P    * 0 |               //high active
    TIM_CCER_CC1E    * 1;                //OC1 enable
  TIM15->EGR = TIM_EGR_UG;               //update regs from shadow regs
  TIM15->BDTR = TIM_BDTR_MOE;            //main output enable
  TIM15->CR1 = TIM_CR1_CEN;              //���������� �������

  AFIO->MAPR2 |= AFIO_MAPR2_TIM15_REMAP; //remap TIM15
  Pin_FAN.Init(AF_PP_10M);               //��������� ����
  RunTimer = 0;
  SetSpeed(0);
  State = FAN_OFF;
}

//------------------------ ���������� ������������: --------------------------

void TFan::Control(int16_t tl, int16_t th, int16_t tget)
{
  //�������� ��������� ��������:
  if(th == TEMP_FAIL || tget == TEMP_FAIL)
  {
    Speed = FAN_MAX_SPEED;
  }
  //���������������� ����������:
  else if(tl < th)
  {
    if(tget > th) Speed = FAN_MAX_SPEED;
    else if(tget < tl) Speed = 0;
    else Speed = FAN_MIN_SPEED + (int32_t)(tget - tl) * (FAN_MAX_SPEED - FAN_MIN_SPEED) / (th - tl);
  }
  //�������� ���������� � ������������:
  else
  {
    if(tget > tl) Speed = FAN_MAX_SPEED;
    else if(tget < th) Speed = 0;
  }

  switch(State)
  {
  case FAN_OFF:
    {
      if(Speed > FAN_MIN_SPEED)
      {
        Speed = FAN_STARTUP_SPEED;
        State = FAN_RUN;
        RunTimer = FAN_MIN_RUN;
      }
      else
      {
        Speed = 0;
      }
      break;
    }
  case FAN_RUN:
    {
      if(RunTimer)
      {
        RunTimer--;
        if(Speed < FAN_MIN_SPEED)
          Speed = FAN_MIN_SPEED;
      }
      else
      {
        if(Speed < FAN_MIN_SPEED)
        {
          Speed = 0;
          State = FAN_OFF;
        }
      }
      break;
    }
  }
  SetSpeed(Speed);
}

//-------------------- ��������� �������� �����������: -----------------------

void TFan::SetSpeed(uint8_t p)
{
  if(p > FAN_MAX_SPEED) p = FAN_MAX_SPEED;
  Speed = p;
  TIM15->CCR1 = p;
}

//--------------------- ������ �������� �����������: -------------------------

uint8_t TFan::GetSpeed(void)
{
  return(Speed);
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

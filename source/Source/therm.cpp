//----------------------------------------------------------------------------

//������ ��������� ���������� DS18B20

//----------------------- ������������ �������: ------------------------------

//������������ ������� ��������� DS18B20 (� 12-��������� ������),
//������� ��������� � ����������� ������� USART2_TX (PA2) �
//USART2_RX (PA3). ������������� � ���� ������� ��������� �������������
//�������� 4.7 ��� �� +3.3 �.
//����� �� ���� 1-Wire ������������ � �������������� USART2,
//������� ��������� ��������� ������� ������ � ����-�����.
//������/������ ������ � USART2 ������������ �� �����������.
//��������� ���� ���������� �� ���, ������ ����� ������� �� ������������.
//� ��������, ����� ������������ DMA, �� ����� � �� ������������ :)

//���� ������� ����� "single-wire half-duplex mode" ��� USART2, ��
//��� ���������� ���� 1-Wire ���������� ������ ������ USART2_TX (PA2).
//�� ��������� �� �������� ����� ��� ���� ���������� PA2 - PA3,
//���� ����� ����� �� ������������ (������� ����������������).
//����� ��������, ��� ���������� ������-�������� ����� ������ ���� �����
//���� ������ ��������, ��������, ��� ���������� ������������ ����������.

//����� �������, ��� � ����������� � ����� ������� ���������� �� �������
//����� ���� �� ��� ������� ���������� ������ 1-Wire. �������� �����
//���������� � ������� USART ���� ��������� ���������� ��������� �
//���������, �� �������� ������ � ����� ���������� ������ �� �����������.
//�� �� ��������� ���� ��������� ������� baud rate, ��� ��������� �������
//���������� ������� �������� USART � STM32 (����� ��� ����������� 1-Wire
//� ������� �������, �� ��� ������� ��������� ������������������ ����������
//�� ������ ���). ������� ���� - � ������ ������ �� ����� 1-Wire
//��� ������� �����.

//----------------------------------------------------------------------------

#include "main.h"
#include "therm.h"

//------------------------------ ���������: ----------------------------------

#define BR_RESET    10417 //�������� ����� ��� ������������ RESET
#define BR_TSLOT   166667 //�������� ����� ��� ������������ TIME SLOT
#define CONVERT_TM    800 //����� �������������� �����������, ��

#define BRR_RESET (APB1_CLOCK + BR_RESET / 2) / BR_RESET;
#define BRR_TSLOT (APB1_CLOCK + BR_TSLOT / 2) / BR_TSLOT;

//----------------------------------------------------------------------------
//--------------------- ����������� ����� TOwpAction: ------------------------
//----------------------------------------------------------------------------

//----------------------------- �����������: ---------------------------------

TOwpAction::TOwpAction(char data)
{
  Data = data;
  Result = OWP_NONE;
  Action = OWP_NONE;
}

//--------------------- �������� ���������� ��������: ------------------------

void TOwpAction::Execute(void)
{
  if((Action == OWP_READY) ||
     (Action == OWP_FAIL))
  {
    Value = DataRd;
    Result = Action;
    Action = OWP_NONE;
  }
}

//-------------------------- ���������� USART2: ------------------------------

volatile OwpAct_t TOwpAction::Action;
char TOwpAction::BitCounter;
char TOwpAction::DataRd;
char TOwpAction::DataWr;

void USART2_IRQHandler(void)
{
  //���������� USART �� ���������� ��������:
  if(USART2->SR & USART_SR_TC)
  {
    //������� ����� ����������:
    USART2->SR &= ~USART_SR_TC;
    //����������� �����:
    if(TOwpAction::Action == OWP_RESET)
    {
      TOwpAction::DataRd = USART2->DR;
      //������� presence ��������� � ����� �� ���� ����� D4..D6:
      if(((TOwpAction::DataRd & 0x70) != 0x70) &&
         //��� D7 ������ ���� ���������, ����� ��� ��������� �����
         //�� �����, � �� ����������� ����������:
         ((TOwpAction::DataRd & 0x80) == 0x80))
        TOwpAction::Action = OWP_READY;
          else TOwpAction::Action = OWP_FAIL;
    }
    //����������� ������/������:
    if(TOwpAction::Action == OWP_RW)
    {
      TOwpAction::DataRd >>= 1;
      //����������� ��� D1 (� �� D0), ��� ��� ��� ���� ������ ������
      //����� ����� ����� � ������� 15 ��� ����� ������ ����-�����:
      TOwpAction::DataRd |= ((USART2->DR & 2)? 0x80 : 0x00);
      if(++TOwpAction::BitCounter < 8)
      {
        TOwpAction::DataWr >>= 1;
        USART2->DR = (TOwpAction::DataWr & 1)? 0xFF : 0x00;
      }
      else
      {
        TOwpAction::Action = OWP_READY;
      }
    }
  }
}

//----------------------------------------------------------------------------
//---------------------------- ����� TOwpReset: ------------------------------
//----------------------------------------------------------------------------

//---------------------------- ������ ��������: ------------------------------

void TOwpReset::Start(void)
{
  DataWr = Data;
  USART2->BRR = BRR_RESET;
  USART2->DR = DataWr;
  Result = OWP_NONE;
  Action = OWP_RESET;
}

//----------------------------------------------------------------------------
//----------------------------- ����� TOwpRW: --------------------------------
//----------------------------------------------------------------------------

//---------------------------- ������ ��������: ------------------------------

void TOwpRW::Start(void)
{
  DataWr = Data;
  USART2->BRR = BRR_TSLOT;
  USART2->DR = (DataWr & 1)? 0xFF : 0x00;
  BitCounter = 0;
  Result = OWP_NONE;
  Action = OWP_RW;
}

//----------------------------------------------------------------------------
//---------------------------- ����� TOwpTask: -------------------------------
//----------------------------------------------------------------------------

//----------------------------- �����������: ---------------------------------

TOwpTask::TOwpTask(char maxact)
{
  MaxActions = maxact;
  Actions = new TOwpAction*[MaxActions];
  ActCount = 0;
  State = OWP_NONE;
}

//------------------------ ���������� ��������: ------------------------------

void TOwpTask::AddAction(TOwpAction *act)
{
  if(ActCount < MaxActions)
    Actions[ActCount++] = act;
}

//---------------- ������ ������������������ ��������: -----------------------

void TOwpTask::Start(void)
{
  Index = 0;
  Error = 0;
  State = OWP_ACT;
  Actions[Index]->Start();
}

//-------------------- ������� ���������� ��������: --------------------------

void TOwpTask::Execute(void)
{
  if(State == OWP_ACT)
  {
    if(Actions[Index]->Result == OWP_READY)
    {
      Index++;
      if(Index == ActCount)
        State = OWP_READY;
          else Actions[Index]->Start();
    }
    else if(Actions[Index]->Result == OWP_FAIL)
    {
      Error = 1;
      State = OWP_READY;
    }
    else
    {
      Actions[Index]->Execute();
    }
  }
}

//------------ �������� ���������� ������������������ ��������: --------------

bool TOwpTask::Done(void)
{
  if(State == OWP_READY)
  {
    State = OWP_NONE;
    return(1);
  }
  return(0);
}

//----------------------------- ������ ������: -------------------------------

bool TOwpTask::Fail(void)
{
  return(Error);
}

//----------------------------------------------------------------------------
//----------------------------- ����� TTherm: --------------------------------
//----------------------------------------------------------------------------

//----------------------------- �����������: ---------------------------------

TTherm::TTherm(void)
{
  //��������� ������:
  Pin_OWPO.Init(AF_OD_2M);
  Pin_OWPI.Init(IN_FLOAT);
  //��������� USART2:
  RCC->APB1ENR |= RCC_APB1ENR_USART2EN; //��������� ������������ USART2
  USART2->BRR = BRR_RESET;
  //USART2->CR3 = USART_CR3_HDSEL; //������ ����� ���� ��� USART2_TX (PA2)
  USART2->CR1 =
    USART_CR1_RE |   //���������� ���������
    USART_CR1_TE |   //���������� �����������
    USART_CR1_TCIE | //���������� ���������� �� ����� ��������
    USART_CR1_UE;    //���������� USART2
  //��������� ����������:
  NVIC_SetPriority(USART2_IRQn, 15);
  NVIC_EnableIRQ(USART2_IRQn);
  ThermTimer = new TSoftTimer(CONVERT_TM);
  ThermTimer->Oneshot = 1;

  OwpStartTherm = new TOwpTask(3);
  OwpStartTherm->AddAction(new TOwpReset());  //RESET
  OwpStartTherm->AddAction(new TOwpRW(0xCC)); //SKIP ROM
  OwpStartTherm->AddAction(new TOwpRW(0x44)); //START CONVERSION

  OwpReadTherm = new TOwpTask(5);
  OwpReadTherm->AddAction(new TOwpReset());   //RESET
  OwpReadTherm->AddAction(new TOwpRW(0xCC));  //SKIP ROM
  OwpReadTherm->AddAction(new TOwpRW(0xBE));  //READ SCRATCHPAD
  OwpReadTherm->AddAction(new TOwpRW());      //READ TL
  OwpReadTherm->AddAction(new TOwpRW());      //READ TH

  Update = 0;
  Value = TEMP_FAIL;
  OwpStartTherm->Start();          //������ ������� START
}

//-------------- ���������� �������� ��������� �����������: ------------------

void TTherm::Execute(void)
{
  OwpStartTherm->Execute();        //���������� ������� START
  if(OwpStartTherm->Done())        //���� ������� START ���������,
    ThermTimer->Start(CONVERT_TM); //������ �������
  if(ThermTimer->Over())           //���� �������� �����,
    OwpReadTherm->Start();         //������ ������� READ
  OwpReadTherm->Execute();         //���������� ������� READ
  if(OwpReadTherm->Done())         //���� ������� READ ���������,
  {
    if((OwpStartTherm->Fail()) ||  //���� ������,
       (OwpReadTherm->Fail()))
      Value = TEMP_FAIL;           //��� ������ �����������
        else Value = CalculateT(); //�������������� �����������
    Update = 1;                    //��������� ����� ����������
    OwpStartTherm->Start();        //������ ������� START
  }
  else
  {
    Update = 0;
  }
}

//---------------------- ���������� �����������: -----------------------------

int16_t TTherm::CalculateT(void)
{
  char tl = OwpReadTherm->Actions[3]->Value;
  char th = OwpReadTherm->Actions[4]->Value;
  int16_t temp = 10 * (tl | (th << 8)) / 16;
  if(temp < TEMP_MIN) temp = TEMP_MIN;
  if(temp > TEMP_MAX) temp = TEMP_MAX;
  return(temp);
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

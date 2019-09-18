//----------------------------------------------------------------------------

//������ ��������� �����

//----------------------------------------------------------------------------

#include "main.h"
#include "port.h"
#include "display.h"
#include "analog.h"
#include "fan.h"

//----------------------------------------------------------------------------
//------------------------------ ����� TPort ---------------------------------
//----------------------------------------------------------------------------

//----------------------------- �����������: ---------------------------------

TPort::TPort(void)
{
  WakePort = new TWakePort(BAUD_RATE, FRAME_SIZE);
}

//-------------------------- ���������� ������: ------------------------------

void TPort::Execute(void)
{
  char Command = WakePort->GetCmd(); //������ ���� �������� �������
  if(Command != CMD_NOP)             //���� ���� �������, ����������
  {
    switch(Command)
    {
    //����������� �������:  
    //��������� ������
    case CMD_ERR:                    
      {
        WakePort->AddByte(ERR_TX);
        break;
      }
    //���
    case CMD_ECHO:                   
      {
        char cnt = WakePort->GetRxCount();
        for(char i = 0; i < cnt; i++)
          WakePort->AddByte(WakePort->GetByte());
        break;
      }
    //������ ��������� �� ����������
    case CMD_INFO:                   
      {
        char Info[] = {DEVICE_NAME};
        char *s = Info;
        do WakePort->AddByte(*s++);
          while(*s); 
        break;
      }
    //����������� �������:
    //��������� ���������� � ����
    case CMD_SET_VI:
      {
        Data->MainData->Items[PAR_V]->Value = WakePort->GetWord();
        Data->MainData->Items[PAR_I]->Value = WakePort->GetWord();
        Data->MainData->Items[PAR_V]->Validate();
        Data->MainData->Items[PAR_I]->Validate();
        Data->OutOn = WakePort->GetByte();
        //��� ���������� �� ���������� V � I � EEPROM �� �����������
        Analog->ClrProtSt();
        Data->SetVI();
        WakePort->AddByte(ERR_NO);
        break;
      }
    //������ �������������� ���������� � ����
    case CMD_GET_VI:
      {
        WakePort->AddByte(ERR_NO);
        WakePort->AddWord(Data->MainData->Items[PAR_V]->Value);
        WakePort->AddWord(Data->MainData->Items[PAR_I]->Value);
        break;
      }
    //������ ������� ���������
    case CMD_GET_STAT:
      {
        WakePort->AddByte(ERR_NO);
        char state, s = 0;
        if(Analog->OutState()) s |= 0x01;
        state = Analog->GetCvCcSt();
        if(state & PS_CV) s |= 0x02;
        if(state & PS_CC) s |= 0x04;
        state = Analog->GetProtSt();
        if(state & PR_OVP) s |= 0x08;
        if(state & PR_OCP) s |= 0x10;
        if(state & PR_OPP) s |= 0x20;
        if(state & PR_OTP) s |= 0x40;
        WakePort->AddByte(s);
        break;
      }
    //������ �������� ����������� ���������� � ����
    case CMD_GET_VI_AVG:
      {
        WakePort->AddByte(ERR_NO);
        WakePort->AddWord(Analog->AdcV->Value);
        WakePort->AddWord(Analog->AdcI->Value);
        break;
      }
    //������ ����������� ����������� ���������� � ����
    case CMD_GET_VI_FAST:
      {
        WakePort->AddByte(ERR_NO);
        WakePort->AddWord(Analog->AdcV->FastValue);
        WakePort->AddWord(Analog->AdcI->FastValue);
        break;
      }
    //��������� ����. ����������, ���� � ��������
    case CMD_SET_VIP_MAX:
      {
        Data->TopData->Items[PAR_MAXV]->Value = WakePort->GetWord();
        Data->TopData->Items[PAR_MAXI]->Value = WakePort->GetWord();
        Data->TopData->Items[PAR_MAXP]->Value = WakePort->GetWord();
        Data->TopData->Items[PAR_MAXV]->Validate();
        Data->TopData->Items[PAR_MAXI]->Validate();
        Data->TopData->Items[PAR_MAXP]->Validate();
        Display->Off();
        Data->TopData->SaveToEeprom(PAR_MAXV);
        Data->TopData->SaveToEeprom(PAR_MAXI);
        Data->TopData->SaveToEeprom(PAR_MAXP);
        Display->On();
        Data->TrimParamsLimits();
        WakePort->AddByte(ERR_NO);
        break;
      }
    //������ ����. ����������, ���� � ��������
    case CMD_GET_VIP_MAX:
      {
        WakePort->AddByte(ERR_NO);
        WakePort->AddWord(Data->TopData->Items[PAR_MAXV]->Value);
        WakePort->AddWord(Data->TopData->Items[PAR_MAXI]->Value);
        WakePort->AddWord(Data->TopData->Items[PAR_MAXP]->Value);
        break;
      }
    //������ �������
    case CMD_SET_PRE:
      {
        char n = WakePort->GetByte();
        if(n < PRESETS)
        {
          uint16_t v = Data->MainData->Items[PAR_V]->Value;
          uint16_t i = Data->MainData->Items[PAR_I]->Value;
          Data->MainData->Items[PAR_V]->Value = WakePort->GetWord();
          Data->MainData->Items[PAR_I]->Value = WakePort->GetWord();
          Data->MainData->Items[PAR_V]->Validate();
          Data->MainData->Items[PAR_I]->Validate();
          Data->SavePreset(n);
          Data->MainData->Items[PAR_V]->Value = v;
          Data->MainData->Items[PAR_I]->Value = i;
          WakePort->AddByte(ERR_NO);
        }
        else
        {
          WakePort->AddByte(ERR_PA);
        }
        break;
      }
    //������ �������
    case CMD_GET_PRE:
      {
        char n = WakePort->GetByte();
        if(n < PRESETS)
        {
          WakePort->AddByte(ERR_NO);
          uint16_t v = Data->MainData->Items[PAR_V]->Value;
          uint16_t i = Data->MainData->Items[PAR_I]->Value;
          Data->ReadPreset(n);
          WakePort->AddWord(Data->MainData->Items[PAR_V]->Value);
          WakePort->AddWord(Data->MainData->Items[PAR_I]->Value);
          Data->MainData->Items[PAR_V]->Value = v;
          Data->MainData->Items[PAR_I]->Value = i;
        }
        else
        {
          WakePort->AddByte(ERR_PA);
        }
        break;
      }
    //��������� ���������
    case CMD_SET_PAR:
      {
        char n = WakePort->GetByte();
        //������������� ������� ���������:
        if(n < PAR_COUNT) n = ParIdx[n];
          else n = PAR_NON;
        if(n != PAR_NON)
        {
          Data->SetupData->Items[n]->Value = WakePort->GetWord();
          Data->SetupData->Items[n]->Validate();
          Data->SetupData->SaveToEeprom(n);
          Data->Apply(n);
        }
        WakePort->AddByte(ERR_NO);
        break;
      }
    //������ ���������
    case CMD_GET_PAR:
      {
        char n = WakePort->GetByte();
        //������������� ������� ���������:
        if(n < PAR_COUNT) n = ParIdx[n];
          else n = PAR_NON;
        if(n != PAR_NON)
        {
          WakePort->AddByte(ERR_NO);
          WakePort->AddWord(Data->SetupData->Items[n]->Value);
        }
        else
        {
          WakePort->AddByte(ERR_NO);
          WakePort->AddWord(0);
        }
        break;
      }
    //������ �������� ����������� � �����������
    case CMD_GET_FAN:
      {
        WakePort->AddByte(ERR_NO);
        WakePort->AddByte(Analog->GetSpeed());
        WakePort->AddWord(Analog->GetTemp());
        break;
      }
    //��������� ���� ���
    case CMD_SET_DAC:
      {
        uint16_t cv = WakePort->GetWord();
        uint16_t ci = WakePort->GetWord();
        if(cv > DACM) cv = DACM;
        if(ci > DACM) ci = DACM;
        Analog->DacV->SetCode(cv);
        Analog->DacI->SetCode(ci);
        WakePort->AddByte(ERR_NO);
        break;
      }
    //������ ���� ���
    case CMD_GET_ADC:
      {
        WakePort->AddByte(ERR_NO);
        WakePort->AddWord(Analog->AdcV->Code);
        WakePort->AddWord(Analog->AdcI->Code);
        break;
      }
    //��������� �������������� ������������
    case CMD_SET_CAL:
      {
        char n = WakePort->GetByte();
        if(n < CAL_CNT)
        {
          Analog->CalibData->Items[n]->Value = WakePort->GetWord();
          Analog->CalibData->Items[n]->Validate();
          Display->Off();
          Analog->CalibData->SaveToEeprom(n);
          Analog->CalibData->EeSection->Validate();
          Display->On();
          Analog->CalibAll();
          WakePort->AddByte(ERR_NO);
        }
        else
        {
          WakePort->AddByte(ERR_PA);
        }
        break;
      }
    //������ �������������� ������������
    case CMD_GET_CAL:
      {
        char n = WakePort->GetByte();
        if(n < CAL_CNT)
        {
          WakePort->AddByte(ERR_NO);
          WakePort->AddWord(Analog->CalibData->Items[n]->Value);
        }
        else
        {
          WakePort->AddByte(ERR_PA);
        }
        break;
      }
    //����������� �������
    default: 
      {
        WakePort->AddByte(ERR_PA);
      }      
    }
    WakePort->StartTx(Command);
  }
}

//----------------------------------------------------------------------------

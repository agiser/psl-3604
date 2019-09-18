//----------------------------------------------------------------------------

//������ ���������� �������������

//----------------------- ������������ �������: ------------------------------

//����� TAnalog �������� ����������� ��� ���������� ����� �����.
//�� ���������� �������������� ���� ��� � ��� � �������� � �������
//�������� ������� ����������. ��������� ������� ���������� ������ ���.
//������������ ���������� ���������� ������ ��������� OUT ON/OFF.
//��������� ������� ��������� ����������� � ������� ������ Therm �
//��������� ������������.

//----------------------------------------------------------------------------

#include "main.h"
#include "analog.h"
#include "display.h"
#include "sound.h"

//----------------------------------------------------------------------------
//----------------------------- ����� TScaler: -------------------------------
//----------------------------------------------------------------------------

#define SCALE ((1LL << (64 - 16 - 1)) / VMAX) //������� �������������

//---------- ���������� ������������� ������������� �� ���� ������: ----------

void TScaler::Calibrate(uint16_t p1, uint16_t c1,
                        uint16_t p2, uint16_t c2)
{
  uint16_t dp = p2 - p1;
  uint16_t dc = c2 - c1;
  Kx = (SCALE * dp + dc / 2) / dc;
  Sx = Kx * c1 - SCALE * p1;
}

//------------------- �������������� ���� � ��������: ------------------------

uint16_t TScaler::CodeToValue(uint16_t code)
{
  int32_t v = (Kx * code - Sx + SCALE / 2) / SCALE;
  if(v < 0) v = 0;
  if(v > VMAX) v = VMAX;
  return(v);
}

//------------------- �������������� �������� � ���: -------------------------

uint16_t TScaler::ValueToCode(uint16_t value)
{
  int32_t c = (SCALE * value + Sx + Kx / 2) / Kx;
  if(c < 0) c = 0;
  if(c > DAC_MAX_CODE) c = DAC_MAX_CODE;
  return(c);
}

//----------------------------------------------------------------------------
//---------------------------- ����� TAnalog: -------------------------------
//----------------------------------------------------------------------------

//----------------------------- �����������: ---------------------------------

TAnalog::TAnalog(void)
{
  Therm = new TTherm();
  TempUpdate = 0;
  Fan = new TFan();
  Pin_CC.Init(IN_PULL, PULL_DN);
  Pin_ON.Init(OUT_PP_2M, OUT_LO);
  Pin_PVG.Init(IN_PULL, PULL_UP);
  AdcV = new TAdc<ADC_CH_V, ADC_PIN_V>();
  AdcI = new TAdc<ADC_CH_I, ADC_PIN_I>();
  DacV = new TDac<DAC_CH_V>();
  DacI = new TDac<DAC_CH_I>();

  CCTimer = new TSoftTimer(CVCC_DEL);
  CCTimer->Force();
  CVTimer = new TSoftTimer(CVCC_DEL);
  CVTimer->Force();

  OvpTimer = new TSoftTimer();
  OcpTimer = new TSoftTimer();
  OppTimer = new TSoftTimer();
  ProtSt = PR_OK;

  CvCcSt = PS_UNREG;
  CvCcPre = PS_UNREG;
  Out = 0;

  OutBlinkTimer = new TSoftTimer();
  OutBlinkTimer->Oneshot = 1;
  OffTime = 0;

  //�������� �������:
  while(PWR->CSR & PWR_CSR_PVDO || !Pin_PVG);

  CalibData = new TParamList(CAL_CNT);
  //                            Type       Name   Min   Nom   Max
  CalibData->AddItem(new TParam(PT_V,     " P1 ", VP1L, VPD1, VP1H)); //CAL_VP1
  CalibData->AddItem(new TParam(PT_VC,    " C1 ",    1, VCD1, DACM)); //CAL_VC1
  CalibData->AddItem(new TParam(PT_V,     " P2 ", VP2L, VPD2, VP2H)); //CAL_VP2
  CalibData->AddItem(new TParam(PT_VC,    " C2 ",    1, VCD2, DACM)); //CAL_VC2
  CalibData->AddItem(new TParam(PT_I,     " P1 ", IP1L, IPD1, IP1H)); //CAL_IP1
  CalibData->AddItem(new TParam(PT_IC,    " C1 ",    1, ICD1, DACM)); //CAL_IC1
  CalibData->AddItem(new TParam(PT_I,     " P2 ", IP2L, IPD2, IP2H)); //CAL_IP2
  CalibData->AddItem(new TParam(PT_IC,    " C2 ",    1, ICD2, DACM)); //CAL_IC2
  CalibData->AddItem(new TParam(PT_NYDEF, "Stor",    0,    0,    2)); //CAL_STR
  CalibData->AddItem(new TParam(PT_VC,        "",    1, VMD1, ADCM)); //CAL_VM1
  CalibData->AddItem(new TParam(PT_VC,        "",    1, VMD2, ADCM)); //CAL_VM2
  CalibData->AddItem(new TParam(PT_IC,        "",    1, IMD1, ADCM)); //CAL_IM1
  CalibData->AddItem(new TParam(PT_IC,        "",    1, IMD2, ADCM)); //CAL_IM2
  CalibData->EeSection = new TCrcSection(CalibData->ItemsCount);
  CalibData->ReadFromEeprom();

  CalibAll();
  DacV->OnOff(0);
  DacI->OnOff(0);
  DacV->SetZero(ZV_VAL);
  DacI->SetZero(ZI_VAL);
}

//---------- ��������� �������� ���������� �������� MAX_V � MAX_I: -----------

void TAnalog::TrimParamsLimits(void)
{
  CalibData->Items[CAL_VP2]->Max = Data->TopData->Items[PAR_MAXV]->Value;
  //��������� ����, ���� �������� �������� �����:
  if(CalibData->Items[CAL_VP2]->Validate())
    CalibData->Items[CAL_VC2]->Value =
      DacV->ValueToCode(CalibData->Items[CAL_VP2]->Value);

  CalibData->Items[CAL_IP2]->Max = Data->TopData->Items[PAR_MAXI]->Value;
  //��������� ����, ���� �������� �������� �����:
  if(CalibData->Items[CAL_IP2]->Validate())
    CalibData->Items[CAL_IC2]->Value =
      DacV->ValueToCode(CalibData->Items[CAL_IP2]->Value);
}

//-------------------------- ������� ���������: ------------------------------

void TAnalog::Execute(void)
{
  Therm->Execute();
  AdcV->Execute();
  AdcI->Execute();
  CvCcControl();
  ThermalControl();
  Protection();
  Supervisor();
  OffTimer();
}

//----------------- �������� ���������� ��� ����� �����: ---------------------

void TAnalog::CalibAll(void)
{
  CalibDacV();
  CalibDacI();
  CalibAdcV();
  CalibAdcI();
}

//-------------------- �������� ���������� ��� DAC_V: ------------------------

void TAnalog::CalibDacV(void)
{
  DacV->Calibrate(CalibData->Items[CAL_VP1]->Value,
                  CalibData->Items[CAL_VC1]->Value,
                  CalibData->Items[CAL_VP2]->Value,
                  CalibData->Items[CAL_VC2]->Value);
}

//-------------------- �������� ���������� ��� DAC_I: ------------------------

void TAnalog::CalibDacI(void)
{
  DacI->Calibrate(CalibData->Items[CAL_IP1]->Value,
                  CalibData->Items[CAL_IC1]->Value,
                  CalibData->Items[CAL_IP2]->Value,
                  CalibData->Items[CAL_IC2]->Value);
}

//-------------------- �������� ���������� ��� ADC_V: ------------------------

//���� �������� ����� CAL_VM1, �������� ������ ����� ���� ADC_V.
//���� �������� ����� CAL_VM2, �������� ������ ����� ���� ADC_V.
//����� ������������ ������� �������� ����������.

void TAnalog::CalibAdcV(char p)
{
  if(p == CAL_VM1) CalibData->Items[CAL_VM1]->Value = AdcV->Code;
  if(p == CAL_VM2) CalibData->Items[CAL_VM2]->Value = AdcV->Code;
  AdcV->Calibrate(CalibData->Items[CAL_VP1]->Value,
                  CalibData->Items[CAL_VM1]->Value,
                  CalibData->Items[CAL_VP2]->Value,
                  CalibData->Items[CAL_VM2]->Value);
}

//-------------------- �������� ���������� ��� ADC_I: ------------------------

//���� �������� ����� CAL_IM1, �������� ������ ����� ���� ADC_I.
//���� �������� ����� CAL_IM2, �������� ������ ����� ���� ADC_I.
//����� ������������ ������� �������� ����������.

void TAnalog::CalibAdcI(char p)
{
  if(p == CAL_IM1) CalibData->Items[CAL_IM1]->Value = AdcI->Code;
  if(p == CAL_IM2) CalibData->Items[CAL_IM2]->Value = AdcI->Code;
  AdcI->Calibrate(CalibData->Items[CAL_IP1]->Value,
                  CalibData->Items[CAL_IM1]->Value,
                  CalibData->Items[CAL_IP2]->Value,
                  CalibData->Items[CAL_IM2]->Value);
  int16_t dp = 2 * AdcI->ValueToCode(0) - AdcI->ValueToCode(DP_VAL);
  DP_Code = (dp < 0)? 0 : dp;
}

//------------------------ ������������ ������: ------------------------------

inline void TAnalog::Protection(void)
{
  //OVP:
  if(AdcV->FastUpdate)
  {
    uint16_t vp = Data->SetupData->Items[PAR_OVP]->Value;
    //���� OVP = MAX_V, ������ ���������:
    if((vp < Data->SetupData->Items[PAR_OVP]->Max) && Out)
    {
      if(Analog->AdcV->FastValue >= vp)
      {
        if(OvpTimer->Over())
        {
          OutControl(0);
          Sound->ABell();
          ProtSt |= PR_OVP;
        }
      }
      else
      {
        OvpTimer->Start(Data->SetupData->Items[PAR_DEL]->Value);
      }
    }
  }
  //OCP:
  if(AdcI->FastUpdate)
  {
    uint16_t ip = Data->SetupData->Items[PAR_OCP]->Value;
    //���� OCP = MAX_I, ������ ���������:
    if((ip < Data->SetupData->Items[PAR_OCP]->Max) && Out)
    {
      if(Analog->AdcI->FastValue >= ip)
      {
        if(OcpTimer->Over())
        {
          OutControl(0);
          Sound->ABell();
          ProtSt |= PR_OCP;
        }
      }
      else
      {
        OcpTimer->Start(Data->SetupData->Items[PAR_DEL]->Value);
      }
    }
   //OPP:
    uint16_t pp = Data->SetupData->Items[PAR_OPP]->Value;
    //���� OPP = MAX_P, ������ ���������:
    if((pp < Data->SetupData->Items[PAR_OPP]->Max) && Out)
    {
      uint16_t pow = (uint32_t)Analog->AdcI->FastValue *
                               Analog->AdcV->FastValue / VI2P;
      if(pow >= pp)
      {
        if(OppTimer->Over())
        {
          OutControl(0);
          Sound->ABell();
          ProtSt |= PR_OPP;
        }
      }
      else
      {
        OppTimer->Start(Data->SetupData->Items[PAR_DEL]->Value);
      }
    }
  }
}

//------------------------- ���������� �������: ------------------------------

inline void TAnalog::Supervisor(void)
{
  static uint8_t PvgCnt = 0;
  if(TSysTimer::Tick)
  {
    if(Pin_PVG) PvgCnt = 0;
      else PvgCnt++;
  }
  if((PWR->CSR & PWR_CSR_PVDO) || (PvgCnt > PVG_PER))
  {
    Pin_ON = 0;
    Sound->Off();
    Display->Disable();
    TSysTimer::Delay_ms(1000);
    __disable_interrupt();
    NVIC_SystemReset();
  }
}

//------------------------- �������� ���������: ------------------------------

inline void TAnalog::ThermalControl(void)
{
  if(Therm->Update)
  {
    Temp = Therm->Value;
    int16_t Otp = Data->SetupData->Items[PAR_OTP]->Value;
    int16_t TfanL = Data->SetupData->Items[PAR_FNL]->Value;
    int16_t TfanH = Data->SetupData->Items[PAR_FNH]->Value;
    //������ ����������:
    if(Temp == TEMP_FAIL)
    {
      ProtSt &= ~PR_OTP;
    }
    else
    {
      //����������� ������ ������ OTP - DTAL:
      if(Temp > Otp - DTAL)
      {
        TfanH = TEMP_FAIL; //������ �������� �����������
        if(Out) Sound->ABell();
        //����������� ��������� ����� OTP:
        if(Temp > Otp)
        {
          OutControl(0);    //���������� ������
          ProtSt |= PR_OTP; //��������� ����� ������
        }
      }
      //����������� ������ ������ OTP - DTAL:
      else
      {
        ProtSt &= ~PR_OTP; //����� ����� ������
      }
    }
    Fan->Control(TfanL, TfanH, Temp); //���������� ������������
    TempUpdate = 1;
  }
  else
  {
    TempUpdate = 0;
  }
}

//-------------------------- ������ �����������: -----------------------------

int16_t TAnalog::GetTemp(void)
{
  return(Temp);
}

//---------------------- ������ �������� �����������: ------------------------

char TAnalog::GetSpeed(void)
{
  return(Fan->GetSpeed());
}

//----------------------- ������ ��������� ������: ---------------------------

char TAnalog::GetProtSt(void)
{
  return(ProtSt);
}

//------------------------- ����� OVP/OCP/OPP: -------------------------------

void TAnalog::ClrProtSt(void)
{
  ProtSt &= ~(PR_OVP | PR_OCP | PR_OPP);
}

//-------------------- ������������ ������� CV/CC: ---------------------------

inline void TAnalog::CvCcControl(void)
{
  if(TSysTimer::Tick)
  {
    char state = PS_UNREG;
    if(Out)
    {
      char prestate = PS_UNREG;
      //�������� ������� CC:
      if( Pin_CC) prestate |= PS_CC;
        else prestate |= PS_CV;
      //���� ���� ���������, ������ ��������:
      if(prestate != CvCcPre)
      {
        if(prestate & PS_CV) CVTimer->Start();
        if(prestate & PS_CC) CCTimer->Start();
        CvCcPre = prestate;
      }
      //������������ �������� ���������:
      if((prestate & PS_CC) || !CCTimer->Over()) state |= PS_CC;
      if((prestate & PS_CV) || !CVTimer->Over()) state |= PS_CV;
      //�������� ���������:
      if(!(CvCcSt & PS_CC) && (state & PS_CC)) Sound->Alarm();
      if(!(CvCcSt & PS_CV) && (state & PS_CV)) Sound->Alert();
    }
    CvCcSt = state;
    //��������� ������������:
    Display->LedCC = CvCcSt & PS_CC;
    Display->LedCV = CvCcSt & PS_CV;
  }
}

//------------------------ ������ ������� CV/CC: -----------------------------

char TAnalog::GetCvCcSt(void)
{
  return(CvCcSt);
}

//------------------------ �������� ��������� CV: ----------------------------

bool TAnalog::IsCV(void)
{
  return(CvCcSt & PS_CV);
}

//------------------------ �������� ��������� CC: ----------------------------

bool TAnalog::IsCC(void)
{
  return(CvCcSt & PS_CC);
}

//-------------------- ���������/���������� ������: --------------------------

void TAnalog::OutControl(bool on)
{
  //��������� ���:
  DacV->OnOff(on);
  DacI->OnOff(on);
  //���� ������� Down Programmer, �� Pin_ON �� ���������:
  if(Data->SetupData->Items[PAR_DNP]->Value)
    Pin_ON = 1;
      else Pin_ON = on;
  Out = on;
  Data->OutOn = on;
  if(!Out)
  {
    CvCcSt = PS_UNREG;
    Display->LedOut = 0;
    Display->LedCC = 0;
    Display->LedCV = 0;
  }
  else
  {
    CvCcSt = PS_CV;
    OvpTimer->Start(Data->SetupData->Items[PAR_DEL]->Value);
    OcpTimer->Start(Data->SetupData->Items[PAR_DEL]->Value);
    Display->LedOut = 1;
    TSysTimer::SecReset(); //����� ���������� �������
  }
}

//---------------------- ������ ��������� ������: ----------------------------

bool TAnalog::OutState(void)
{
  return(Out);
}

//-------------------- ������ �������������� ������: -------------------------

inline void TAnalog::OffTimer(void)
{
  if(OutBlinkTimer->Over() && Out) Display->LedOut = 1;
  if(TSysTimer::SecTick)
  {
    if(Out && OffTime)
    {
      OffTime--;
      Display->LedOut = 0;
      OutBlinkTimer->Start(OUT_BLINK_TIME);
      if(!OffTime)
      {
        Sound->Beep();
        OutControl(0);
      }
    }
  }
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

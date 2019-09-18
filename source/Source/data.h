//----------------------------------------------------------------------------

//������ ������, ������������ ����

//----------------------------------------------------------------------------

#ifndef DATA_H
#define DATA_H

//----------------------------------------------------------------------------

#include "display.h"
#include "eeprom.h"

//------------------------------- ���������: ---------------------------------

#define PRESETS 10 //���������� ��������
#define RING_V 160 //������ ���������� ������ V

#define DMAX   999 //����. ���������� �������� ��������, ��

#define TMIN   200 //���. ���������� �������� �����������, x0.1�C
#define TMAX   999 //����. ���������� �������� �����������, x0.1�C
#define TNOM   600 //����������� ����������� ������ OTP, x0.1�C
#define DTAL    30 //�������� ��� ������ ��������� ���������, x0.1�C
#define TFNL   450 //���. ����������� ��������� �����������, x0.1�C
#define TFNH   550 //���. ����������� ������ �������� �����������, x0.1�C

#define TIMMAX (18 * 60 * 60) //������������ ����� ������� 18 �����

//��������� ��������:
//(� TList ������ ����������� � ����� �� ������������������)

enum TopPars_t
{
  PAR_MAXV, //������������ ����������
  PAR_MAXI, //������������ ���
  PAR_MAXP, //������������ ��������
  PARS_TOP
};

//�������� ���������:
//(� TList ������ ����������� � ����� �� ������������������)

enum MainPars_t
{
  PAR_V,    //������������� ����������
  PAR_I,    //������������� ���
  PAR_FINE, //��������� FINE (OFF/ON)
  PARS_MAIN
};

//��������� ���������:
//(� TList ������ ����������� � ����� �� ������������������)

enum SetupData_t
{
  PAR_CALL, //Call preset
  PAR_STOR, //Store preset
  PAR_LOCK, //Lock controls
  PAR_OVP,  //OVP threshold
  PAR_OCP,  //OCP threshold
  PAR_OPP,  //OPP threshold
  PAR_DEL,  //OVP/OCP delay
  PAR_OTP,  //OTP threshold
  PAR_FNL,  //Fan start temperature
  PAR_FNH,  //Fan full speed temperature
  PAR_HST,  //Heatsink measured temperature
  PAR_TIM,  //Timer
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
  PAR_DEF,  //Load defaults
  PAR_CAL,  //Calibration (NO/YES/DEFAULT)
  PAR_ESC,  //Escape menu (NO/YES)
  PARS_SETUP
};

enum ParType_t //��� ���������
{
  PT_V,     //����������, x0.01 V
  PT_PRV,   //���������� ������, x0.01 V
  PT_I,     //���, x0.001 A
  PT_PRI,   //��� ������, x0.001 A
  PT_P,     //��������, x0.1 W
  PT_PRP,   //�������� ������, x0.1 W
  PT_VC,    //��� ����������
  PT_IC,    //��� ����
  PT_PRE,   //CALL/STORE (NOSAVE)
  PT_OFFON, //OFF/ON
  PT_FALN,  //OFF/ALARM/ON
  PT_APHPL, //AVERAGE/PEAK HIGH/PEAK LOW
  PT_DEL,   //��������, ��
  PT_T,     //�����������, x0.1�C
  PT_FIRM,  //Firmware Version (NOSAVE)
  PT_NY,    //����� ������ (NOSAVE)
  PT_NYDEF, //NO/YES/DEFAULT (NOSAVE)
  PT_TIM    //����� �������
};

enum OffOn_t { OFF, ON };
enum Track_t { TRCOFF, TRCAUTO, TRCON };
enum NoYes_t { NO, YES, DEFAULT };

#define PROT_FLAG 0x80 //���� ������������ ������
#define ON_FLAG 0x8000 //���� ��������� ������
#define VER ((uint16_t)(VERSION * 100 + 0.5))

//----------------------------------------------------------------------------
//----------------------------- ����� TParam: --------------------------------
//----------------------------------------------------------------------------

class TParam
{
private:
  char Name[DIGS + 1];
public:
  char Type;
  TParam(ParType_t type, const char *s,
         uint16_t min, uint16_t nom, uint16_t max);
  uint16_t Min;
  uint16_t Nom;
  uint16_t Max;
  uint16_t Value;
  void ShowName(void);
  void ShowValue(void);
  bool Savable(void);
  bool Validate(void);
  bool Edit(int16_t step);
};

//----------------------------------------------------------------------------
//-------------------------- ��������� ����� TList: --------------------------
//----------------------------------------------------------------------------

template<class T>
class TList
{
private:
  char ItemsMax;
public:
  TList(char max);
  T** Items;
  char ItemsCount;
  void AddItem(T *t);
};

//-------------------------- ���������� �������: -----------------------------

template<class T>
TList<T>::TList(char max)
{
  ItemsMax = max;
  Items = new T*[ItemsMax];
  ItemsCount = 0;
}

template<class T>
void TList<T>::AddItem(T *t)
{
  if(ItemsCount < ItemsMax)
    Items[ItemsCount++] = t;
}

//----------------------------------------------------------------------------
//--------------------------- ����� TParamList: ------------------------------
//----------------------------------------------------------------------------

class TParamList : public TList<TParam>
{
private:
public:
  TParamList(char max) : TList(max) {};
  TEeSection *EeSection;
  void LoadDefaults(void);
  void ReadFromEeprom(char n);
  void ReadFromEeprom(void);
  void SaveToEeprom(char n);
  void SaveToEeprom(void);
};

//----------------------------------------------------------------------------
//----------------------------- ����� TData: ---------------------------------
//----------------------------------------------------------------------------

class TData
{
public:
  TData(void);
  TParamList *TopData;
  TParamList *MainData;
  TParamList *SetupData;
  TEeSection *PresetV;
  TEeSection *PresetI;
  TRingSection *Ring;
  bool OutOn;
  void SetVI(void);
  void Apply(char par);
  void ApplyAll(void);
  void TrimParamsLimits(void);
  void ReadV(void);
  void SaveV(void);
  void InitPresets(void);
  void ReadPreset(char n);
  void SavePreset(char n);
};

//----------------------------------------------------------------------------

extern TData *Data;

//----------------------------------------------------------------------------

#endif

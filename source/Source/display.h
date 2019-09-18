//----------------------------------------------------------------------------

//������ ��������� LED-�������, ������������ ����

//----------------------------------------------------------------------------

#ifndef DISPLAY_H
#define DISPLAY_H

//----------------------------------------------------------------------------

#include "systimer.h"
#include "sreg.h"

//----------------------------- ���������: -----------------------------------

#define POINT      0x80 //���� ����������� �����
#define AUTO_SCALE 0x80 //���� �������������������

//����� ���������:

#define ROW1POS1 0x01
#define ROW1POS2 0x02
#define ROW1POS3 0x04
#define ROW1POS4 0x08
#define ROW2POS1 0x10
#define ROW2POS2 0x20
#define ROW2POS3 0x40
#define ROW2POS4 0x80

enum PosName_t //����� ������� �������
{
  POS_1,
  POS_2,
  POS_3,
  POS_4,
  DIGS
};

enum RowName_t //����� ����� �������
{
  ROW_V,
  ROW_I,
  ROWS
};

enum Blink_t //����� ������� �������
{
  BLINK_NO   = 0x00,
  BLINK_V    = ROW1POS1 | ROW1POS2 | ROW1POS3 | ROW1POS4,
  BLINK_I    = ROW2POS1 | ROW2POS2 | ROW2POS3 | ROW2POS4,
  BLINK_VI   = BLINK_V | BLINK_I,
  BLINK_TIM  = ROW1POS3 | ROW1POS4 | BLINK_I,
  BLINK_NC   = ROW2POS4 //��� ������� ����� ���������� �������� �� ������������
};

//----------------------------------------------------------------------------
//---------------------------- ����� TDisplay: -------------------------------
//----------------------------------------------------------------------------

class TDisplay
{
private:
  char SegDataV[DIGS];
  char SegDataI[DIGS];
  char Row;
  char Pos;
  char Conv(char d);
  char SetScan(char phase);
  Blink_t BlinkEn;
  bool BlinkOn;
  bool DispOn;
  TSreg Sreg;
  TSoftTimer *BlinkTimer;
public:
  TDisplay(void);
  void Execute(void); 
  bool LedCV;              //���������� ����������� CV
  bool LedCC;              //���������� ����������� CC
  bool LedOut;             //���������� ����������� OUT
  void Blink(Blink_t blink); //���������/���������� �������
  bool LedFine;            //���������� ����������� FINE
  void Clear(void);        //������� �������
  void Off(void);          //���������� ������� (������ �����)
  void On(void);           //��������� ������� (������ �����)
  void Disable(void);      //���������� ���� ���������
  void Enable(void);       //��������� ���� ���������
  void SetPos(char row, char pos); //��������� �������
  void PutChar(char ch);   //����� �������
  void PutString(char *s); //����� ������ �� RAM
  void PutString(const char *s); //����� ������ �� ROM
  void PutIntF(int32_t v, char n, char d); //��������������� ����� �����
};

//----------------------------------------------------------------------------

extern TDisplay *Display;

//----------------------------------------------------------------------------

#endif

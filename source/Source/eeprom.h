//----------------------------------------------------------------------------

//������ ��������� ������� EEPROM, ������������ ����

//----------------------------------------------------------------------------

#ifndef EEPROM_H
#define EEPROM_H

//----------------------------------------------------------------------------

#include "systimer.h"

//----------------------------- ���������: -----------------------------------

#define EEPROM_SIZE 512 //����� ���������� ������ 24�04, ����

//����� ������ EEPROM:

enum EError_t
{
  ER_NONE  = 0x00, //��� ������
  ER_SIGN  = 0x01, //������ ���������
  ER_CRC   = 0x02, //������ CRC
  ER_ALLOC = 0x04, //������ ��������� ������
  ER_ASK   = 0x08, //��� ������ EEPROM
  ES_PLAIN = 0x00, //������� ������
  ES_CRC   = 0x10, //������ � ������� CRC
  ES_RING  = 0x20  //ring-������
};

//----------------------------------------------------------------------------
//---------------------------- ����� TEeprom: --------------------------------
//----------------------------------------------------------------------------

class TEeprom
{
private:
  static bool SetAddress(uint16_t addr);
  static uint8_t ByteAddress;
  static uint8_t PageAddress;
protected:
  static uint16_t EeTop;
  static uint16_t Read(uint16_t addr);
  static void Write(uint16_t addr, uint16_t data);
  static void Update(uint16_t addr, uint16_t data);
public:
  static void Init(void);
  static uint8_t Error;
};

//----------------------------------------------------------------------------
//--------------------------- ����� TEeSection: ------------------------------
//----------------------------------------------------------------------------

class TEeSection : public TEeprom
{
private:
protected:
  uint16_t Base;
  uint16_t Size;
  uint16_t Sign;
public:
  TEeSection(uint16_t size);
  bool Valid;
  virtual void Validate(void);
  uint16_t Read(uint16_t addr);
  void Write(uint16_t addr, uint16_t data);
  void Update(uint16_t addr, uint16_t data);
};

//----------------------------------------------------------------------------
//--------------------------- ����� TCrcSection: -----------------------------
//----------------------------------------------------------------------------

class TCrcSection : public TEeSection
{
private:
  uint16_t Crc;
  uint16_t GetCRC(void);
protected:
public:
  TCrcSection(uint16_t size);
  virtual void Validate(void);
};

//----------------------------------------------------------------------------
//--------------------------- ����� TRingSection: ----------------------------
//----------------------------------------------------------------------------

class TRingSection : public TEeSection
{
private:
  uint16_t Ptr;
protected:
public:
  TRingSection(uint16_t size);
  //�� ����� ������, ��� ����� ����������� ��������������
  //�������. ���� � ������������ ������ ������� � ����� �������
  //������� ������������, �� ��� ��������������� ����� �������������
  //��� ���������, �.�. ������������ ������� � ������ ����������
  //���� ����� �� �����. ����� ��� � �����, �� ���������� ������ warning.
  uint16_t Read(void);
  void Write(uint16_t data);
  void Update(uint16_t data);
};

//----------------------------------------------------------------------------

#endif

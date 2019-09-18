//----------------------------------------------------------------------------

//������ ��������� ������� EEPROM

//----------------------- ������������ �������: ------------------------------

//������������ ������� ���������� EEPROM ���� 24�04, ������� ����������
//� ����� SCL (PB8), SDA (PB9). ��������� I2C ����������� ����������.
//� ���������, ���������� I2C1 (remap) ������������ ������,
//��� ��� �� ����������� � SPI1 (remap): �� ������ PB5
//������ ������� (��. errata).
//��� ������������ ��������� ���� ������������ ������ TIM16.

//----------------------------------------------------------------------------

#include "main.h"
#include "i2csw.h"
#include "eeprom.h"

//----------------------------- ���������: -----------------------------------

#define I2C_ADDR  0xA0 //����� ���������� EEPROM
#define EEPROM_WRTM 25 //������������ ����� ������, ��

#define EE_SIGNATURE 0xBED3

//----------------------------------------------------------------------------
//----------------------------- ����� TEEPROM: -------------------------------
//----------------------------------------------------------------------------

//----------------------------- �������������: -------------------------------

uint16_t TEeprom::EeTop;
uint8_t TEeprom::Error;
uint8_t TEeprom::ByteAddress;
uint8_t TEeprom::PageAddress;

void TEeprom::Init(void)
{
  TI2Csw::Init();
  Error = ER_NONE;
  EeTop = 0;  //������ ��������� �������
}

//------------- ������ ������ � ��������� ���������� EEPROM: -----------------

//addr - ����� �����
//���������� true ���� ��������� ����� EEPROM

bool TEeprom::SetAddress(uint16_t addr)
{
  bool ask;
  ByteAddress = (addr << 1) & 0xFE;
  PageAddress = (addr >> 6) & 0x0E;
  TSysTimer::TimeoutStart_ms(EEPROM_WRTM);
  do
  {
    TI2Csw::Start();
    ask = TI2Csw::Write(I2C_ADDR | PageAddress);
  }
  while(!ask && !TSysTimer::TimeoutOver_ms());
  if(ask)
  {
    TI2Csw::Write(ByteAddress);
  }
  else
  {
    TI2Csw::Stop();
    Error |= ER_ASK;
  }
  return(ask);
}

//---------------------- ������ ������ �� EEPROM: ----------------------------

//addr - ����� �����
//data - ����� ������ ��� ������ � EEPROM

uint16_t TEeprom::Read(uint16_t addr)
{
  if(!SetAddress(addr)) return(0);
  TI2Csw::Stop();
  TI2Csw::Start();
  TI2Csw::Write(I2C_ADDR | PageAddress | I2C_RD);
  char data_l = TI2Csw::Read(I2C_ACK);
  char data_h = TI2Csw::Read(I2C_NACK);
  return(WORD(data_h, data_l));
}

//----------------------- ������ ������ � EEPROM: ----------------------------

//addr - ����� �����
//data - ����� ������ ��� ������ � EEPROM

void TEeprom::Write(uint16_t addr, uint16_t data)
{
  if(!SetAddress(addr)) return;
  TI2Csw::Write(LO(data));
  TI2Csw::Write(HI(data));
  TI2Csw::Stop();
}

//--------------------- ���������� ������ � EEPROM: --------------------------

//addr - ����� �����
//data - ����� ������ ��� ������ � EEPROM
//������ ������������ ������ � ��� ������, ���� ����� ������ ����������.

void TEeprom::Update(uint16_t addr, uint16_t data)
{
  uint16_t d = Read(addr);
  if(data != d)
    Write(addr, data);
}

//----------------------------------------------------------------------------
//--------------------------- ����� TEeSection: ------------------------------
//----------------------------------------------------------------------------

//������� ������ EEPROM, ���������� ������
//��������������� ��������� ���������.

//----------------------------- �����������: ---------------------------------

TEeSection::TEeSection(uint16_t size)
{
  Base = EeTop;       //������ ������
  Size = size;        //������ ������
  Sign = Base + size; //�������� ���������
  EeTop = Sign + 1;   //����� ������ ���������� ����� EEPROM
  Valid = 1;
  if(EeTop > EEPROM_SIZE)
  {
    TEeprom::Error |= ER_ALLOC;
    Valid = 0;
  }
  else if(TEeprom::Read(Sign) != EE_SIGNATURE)
  {
    TEeprom::Error |= ER_SIGN;
    Valid = 0;
  }
  if(!Valid) TEeprom::Error |= ES_PLAIN;
}

//------------------------- ��������� ����������: ----------------------------

void TEeSection::Validate(void)
{
  TEeprom::Update(Sign, EE_SIGNATURE);
  Valid = 1;
}

//------------------------- ������ ������ ������: ----------------------------

uint16_t TEeSection::Read(uint16_t addr)
{
  return(TEeprom::Read(Base + addr));
}

//------------------------- ������ ������ ������: ----------------------------

void TEeSection::Write(uint16_t addr, uint16_t data)
{
  if(addr < Size)
    TEeprom::Write(Base + addr, data);
}

//----------------------- ���������� ������ ������: --------------------------

void TEeSection::Update(uint16_t addr, uint16_t data)
{
  if(addr < Size)
    TEeprom::Update(Base + addr, data);
}

//----------------------------------------------------------------------------
//--------------------------- ����� TCrcSection: -----------------------------
//----------------------------------------------------------------------------

//������ EEPROM ���������� ����������, ���������� ������
//��������������� ��������� ��������� � CRC.

//----------------------------- �����������: ---------------------------------

TCrcSection::TCrcSection(uint16_t size) : TEeSection(size)
{
  Crc = EeTop;     //�������� CRC
  EeTop = Crc + 1; //����� ������ ���������� ����� EEPROM
  if(EeTop > EEPROM_SIZE)
  {
    TEeprom::Error |= ER_ALLOC;
    Valid = 0;
  }
  else if(TEeprom::Read(Crc) != GetCRC())
  {
    TEeprom::Error |= ER_CRC;
    Valid = 0;
  }
  if(!Valid) TEeprom::Error |= ES_CRC;
}

//------------------------------ ������ CRC: ---------------------------------

uint16_t TCrcSection::GetCRC(void)
{
  RCC->AHBENR |= RCC_AHBENR_CRCEN;
  CRC->CR = CRC_CR_RESET;
  for(uint16_t i = 0; i < Size; i++)
    CRC->DR = (uint32_t)TEeprom::Read(Base + i);
  uint16_t result = CRC->DR;
  RCC->AHBENR &= ~RCC_AHBENR_CRCEN;
  return(result);
}

//------------------------- ��������� ����������: ----------------------------

void TCrcSection::Validate(void)
{
  TEeSection::Validate();
  TEeprom::Update(Crc, GetCRC());
  Valid = 1;
}

//----------------------------------------------------------------------------
//--------------------------- ����� TRingSection: ----------------------------
//----------------------------------------------------------------------------

//������ EEPROM ����������� �������, ���� ����� ������ ��������� ���
//�������� ����� ����������. ������������ ��������� �����, �����
//�������� ������������ �� ���������� ������, ���������� ��������
//��������� (������������ 0xFFFF). ���������� ������ ���������������
//��������� ���������.

//----------------------------- �����������: ---------------------------------

TRingSection::TRingSection(uint16_t size) : TEeSection(size)
{
  Ptr = 0;
  //����� ������:
  if(Valid)
    while((Ptr < Size) && (TEeprom::Read(Base + Ptr) == 0xFFFF))
      Ptr++;
  if(Ptr == Size) Ptr = 0;
  if(!Valid) TEeprom::Error |= ES_RING;
}

//------------------------- ������ ������ ������: ----------------------------

uint16_t TRingSection::Read(void)
{
  return(TEeprom::Read(Base + Ptr));
}

//------------------------- ������ ������ ������: ----------------------------

void TRingSection::Write(uint16_t data)
{
  uint16_t pre = Ptr;
  if(++Ptr == Size) Ptr = 0;
  TEeprom::Write(Base + Ptr, data);
  TEeprom::Write(Base + pre, 0xFFFF);
}

//----------------------- ���������� ������ ������: --------------------------

void TRingSection::Update(uint16_t data)
{
  if(Read() != data)
    Write(data);
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

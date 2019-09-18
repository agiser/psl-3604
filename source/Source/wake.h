//----------------------------------------------------------------------------

//������ ��������� ��������� Wake, ������������ ����

//----------------------------------------------------------------------------

#ifndef WAKE_H
#define WAKE_H

//----------------------------- ���������: -----------------------------------

//���� ������:

#define ERR_NO      0 //no error
#define ERR_TX      1 //Rx/Tx error
#define ERR_BU      2 //device busy error
#define ERR_RE      3 //device not ready error
#define ERR_PA      4 //parameters value error
#define ERR_NR      5 //no replay
#define ERR_NC      6 //no carrier

//���� ����������� ������:

#define CMD_NOP     0 //��� ��������
#define CMD_ERR     1 //������ ������ ������
#define CMD_ECHO    2 //���
#define CMD_INFO    3 //������ ���������� �� ����������
#define CMD_SETADDR 4 //��������� ������
#define CMD_GETADDR 5 //������ ������

//------------------------ ����� ��������� WAKE: -----------------------------

class TWake
{
private:
  enum WPnt_t
  {
    PTR_ADD,      //�������� � ������ ��� ������
    PTR_CMD,      //�������� � ������ ��� ���� �������
    PTR_LNG,      //�������� � ������ ��� ����� ������
    PTR_DAT       //�������� � ������ ��� ������
  };
  enum WStuff_t
  {
    FEND  = 0xC0, //Frame END
    FESC  = 0xDB, //Frame ESCape
    TFEND = 0xDC, //Transposed Frame END
    TFESC = 0xDD  //Transposed Frame ESCape
  };
  enum WState_t
  {
    WST_IDLE,     //��������� ��������
    WST_ADD,      //����� ������
    WST_CMD,      //����� �������
    WST_LNG,      //����� ����� ������
    WST_DATA,     //�����/�������� ������
    WST_CRC,      //�����/�������� CRC
    WST_DONE      //��������� ����������
  };
  
  char Addr;      //����� ����������
  char RxState;   //��������� �������� ������
  bool RxStuff;   //������� ��������� ��� ������
  char *RxPtr;    //��������� ������ ������
  char *RxEnd;    //�������� ��������� ����� ������ ������
  char RxCount;   //���������� �������� ����
  char *RxData;   //����� ������

  char TxState;   //��������� �������� ��������
  bool TxStuff;   //������� ��������� ��� ��������
  char *TxPtr;    //��������� ������ ��������
  char *TxEnd;    //�������� ��������� ����� ������ ��������
  char TxCount;   //���������� ������������ ����
  char *TxData;   //����� ��������
  
  char Frame;
  void Do_Crc8(char b, char *crc); //���������� ����������� �����
protected:
  void Rx(char data);  //����� �����
  bool Tx(char &data); //�������� �����
public:
  TWake(char frame);
  char GetCmd(void);      //���������� ������� ��� �������
  char GetRxCount(void);  //���������� ���������� �������� ����
  void SetRxPtr(char p);  //������������� ��������� ������ ������
  char GetRxPtr(void);    //������ ��������� ������ ������
  char GetByte(void);     //������ ���� �� ������ ������
  int16_t GetWord(void);  //������ ����� �� ������ ������
  int32_t GetDWord(void); //������ ������� ����� �� ������ ������
  void GetData(char *d, char count); //������ ������ �� ������ ������

  void SetTxPtr(char p);  //������������� ��������� ������ ��������
  char GetTxPtr(void);    //������ ��������� ������ ��������
  void AddByte(char b);   //�������� ���� � ����� ��������
  void AddWord(int16_t w);   //�������� ����� � ����� ��������
  void AddDWord(int32_t dw); //�������� ������� ����� � ����� ��������
  void AddData(char *d, char count); //�������� ������ � ����� ��������
  void TxStart(char cmd, char &data); //������ �������� ������
  bool AskTxEnd(void);    //����������� ����� �������� ������
};

//----------------------------------------------------------------------------

#endif

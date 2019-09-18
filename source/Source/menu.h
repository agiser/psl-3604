//----------------------------------------------------------------------------

//������ ���������� ����, ������������ ����

//----------------------------------------------------------------------------

#ifndef MENU_H
#define MENU_H

//----------------------------------------------------------------------------

#include "data.h"
#include "keyboard.h"

//------------------------------- ���������: ---------------------------------

enum Menu_t //������ ����
{
  MNU_SPLASH,
  MNU_ERROR,
  MNU_MAIN,
  MNU_SETUP,
  MNU_PRESET,
  MNU_PROT,
  MNU_TOP,
  MNU_CALIB,
  MENUS
};

//----------------------------------------------------------------------------
//--------------------- ����������� ����� TMenuItem: -------------------------
//----------------------------------------------------------------------------

class TMenuItem
{
private:
protected:
  TParamList *Params;
  TParam *Par;
  bool Edit;
  uint16_t BackupV;
  virtual void LoadParam(TParam *p);
  virtual void EditEnter(void);
  virtual void EditExit(void);
  virtual void EditEscape(void);
public:
  TMenuItem(TParamList *p) : Params(p) {};
  Menu_t MnuIndex;
  char ParIndex;
  uint16_t Timeout;
  virtual void Init(void) = 0;
  virtual void OnKeyboard(KeyMsg_t &msg) = 0;
  virtual void OnEncoder(int8_t &msg) = 0;
  virtual void OnTimer(void);
  virtual void Execute(void) {};
};

//----------------------------------------------------------------------------
//--------------------------- ����� TMenuSplash: -----------------------------
//----------------------------------------------------------------------------

class TMenuSplash : public TMenuItem
{
private:
public:
  TMenuSplash(TParamList *p) : TMenuItem(p) {};
  virtual void Init(void);
  virtual void OnEncoder(int8_t &msg);
  virtual void OnKeyboard(KeyMsg_t &msg);
};

//----------------------------------------------------------------------------
//--------------------------- ����� TMenuError: ------------------------------
//----------------------------------------------------------------------------

class TMenuError : public TMenuItem
{
private:
public:
  TMenuError(TParamList *p) : TMenuItem(p) {};
  virtual void Init(void);
  virtual void OnEncoder(int8_t &msg);
  virtual void OnKeyboard(KeyMsg_t &msg);
};

//----------------------------------------------------------------------------
//--------------------------- ����� TMenuMain: -------------------------------
//----------------------------------------------------------------------------

class TMenuMain : public TMenuItem
{
private:
  bool Edited;
  bool ForceV;
  bool ForceI;
  bool Dnp;
protected:
  virtual void EditExit(void);
public:
  TMenuMain(TParamList *p) : TMenuItem(p) {};
  virtual void Init(void);
  virtual void Execute(void);
  virtual void OnEncoder(int8_t &msg);
  virtual void OnKeyboard(KeyMsg_t &msg);
  virtual void OnTimer(void);
};

//----------------------------------------------------------------------------
//-------------------------- ����� TMenuPreset: ------------------------------
//----------------------------------------------------------------------------

class TMenuPreset : public TMenuItem
{
private:
  uint16_t BackupI;
protected:
  virtual void EditEnter(void);
  virtual void EditExit(void);
  virtual void EditEscape(void);
public:
  TMenuPreset(TParamList *p) : TMenuItem(p) {};
  virtual void Init(void);
  virtual void OnEncoder(int8_t &msg);
  virtual void OnKeyboard(KeyMsg_t &msg);
};

//----------------------------------------------------------------------------
//--------------------------- ����� TMenuSetup: ------------------------------
//----------------------------------------------------------------------------

class TMenuSetup : public TMenuItem
{
private:
  char ActiveIndex;
  bool Force;
public:
  TMenuSetup(TParamList *p) : TMenuItem(p), ActiveIndex(0) {};
  virtual void Init(void);
  virtual void Execute(void);
  virtual void OnEncoder(int8_t &msg);
  virtual void OnKeyboard(KeyMsg_t &msg);
  virtual void OnTimer(void);
};

//----------------------------------------------------------------------------
//---------------------------- ����� TMenuProt: ------------------------------
//----------------------------------------------------------------------------

class TMenuProt : public TMenuItem
{
private:
  bool Prot;
  bool Force;
  TParam *Temp;
public:
  TMenuProt(TParamList *p) : TMenuItem(p) {};
  virtual void Init(void);
  virtual void Execute(void);
  virtual void OnEncoder(int8_t &msg);
  virtual void OnKeyboard(KeyMsg_t &msg);
  virtual void OnTimer(void);
};

//----------------------------------------------------------------------------
//---------------------------- ����� TMenuTop: -------------------------------
//----------------------------------------------------------------------------

class TMenuTop : public TMenuItem
{
private:
public:
  TMenuTop(TParamList *p) : TMenuItem(p) {};
  virtual void Init(void);
  virtual void OnEncoder(int8_t &msg);
  virtual void OnKeyboard(KeyMsg_t &msg);
};

//----------------------------------------------------------------------------
//--------------------------- ����� TMenuCalib: ------------------------------
//----------------------------------------------------------------------------

class TMenuCalib : public TMenuItem
{
private:
  void Apply(char p);
  bool UpdateCV;
  bool UpdateCI;
public:
  TMenuCalib(TParamList *p) : TMenuItem(p) {};
  virtual void Init(void);
  virtual void OnEncoder(int8_t &msg);
  virtual void OnKeyboard(KeyMsg_t &msg);
};

//----------------------------------------------------------------------------
//--------------------------- ����� TMenuItems: ------------------------------
//----------------------------------------------------------------------------

class TMenuItems : public TList<TMenuItem>
{
public:
  TMenuItems(char max);
  void SelectMenu(Menu_t mnu, char par = 0);
  TMenuItem *SelectedMenu;
};

//----------------------------------------------------------------------------

#endif

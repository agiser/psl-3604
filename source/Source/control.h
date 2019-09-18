//----------------------------------------------------------------------------

//������ ������ ����������, ������������ ����

//----------------------------------------------------------------------------

#ifndef CONTROL_H
#define CONTROL_H

//----------------------------------------------------------------------------

#include "systimer.h"
#include "encoder.h"
#include "keyboard.h"
#include "menu.h"

//----------------------------------------------------------------------------
//---------------------------- ����� TControl: -------------------------------
//----------------------------------------------------------------------------

class TControl
{
private:
  TKeyboard *Keyboard;
  TSoftTimer *MenuTimer;
  TMenuItems *Menu;
  Menu_t MnuIndex;
  char ParIndex;
  void ProtectionService(KeyMsg_t &KeyMsg);
  void FineSwitchService(KeyMsg_t &KeyMsg);
  void OutSwitchService(KeyMsg_t &KeyMsg);
public:
  TControl(void);
  void Execute(void);
};

//----------------------------------------------------------------------------

#endif

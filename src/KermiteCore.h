#pragma once

#include <stdint.h>

class KermiteCore {
public:
  KermiteCore();
  void setKeyboardName(const char *keyboardName);
  void setFirmwareId(const char *firmwareId);
  void enableDebugLogging();
  void begin();
  void feedKeyState(int keyIndex, bool hold);
  void processUpdate();
  void setFlashSavingWaitTimeSec(int sec);
  void setProductionMode();
};

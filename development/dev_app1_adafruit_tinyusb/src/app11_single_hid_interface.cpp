#include "BoardLED.hpp"
#include "Button.hpp"
#include <Adafruit_TinyUSB.h>
#include <Arduino.h>

// tested on Tiny2040

enum {
  RID_KEYBOARD = 1,
  // RID_MOUSE,
  // RID_CONSUMER_CONTROL,
  RID_GENERIC_INOUT,
};

#define RAWHID_REPORT_LEN 63

static const uint8_t desc_hid_report[] = {
  TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(RID_KEYBOARD)),
  // TUD_HID_REPORT_DESC_MOUSE(HID_REPORT_ID(RID_MOUSE)),
  // TUD_HID_REPORT_DESC_CONSUMER(HID_REPORT_ID(RID_CONSUMER_CONTROL)),
  TUD_HID_REPORT_DESC_GENERIC_INOUT(RAWHID_REPORT_LEN, HID_REPORT_ID(RID_GENERIC_INOUT)),

};

static Adafruit_USBD_HID usb_hid(desc_hid_report, sizeof(desc_hid_report), HID_ITF_PROTOCOL_NONE, 2, true);

static BoardLED boardLED(20, 19, 18, true);
static Button buttons[] = { Button(7), Button(5), Button(2), Button(26) };

static void sendHidKey(uint hidKeycode, bool pressed) {
  static uint8_t hidKeycodes[6];
  if (usb_hid.ready()) {
    hidKeycodes[0] = pressed ? hidKeycode : 0;
    usb_hid.keyboardReport(RID_KEYBOARD, 0, hidKeycodes);
  }
}

static uint8_t rawHidTxBuf[RAWHID_REPORT_LEN];

static void updateButton() {
  {
    Button &button = buttons[0];
    button.update();
    if (button.pressed) {
      sendHidKey(4, true);
    }
    if (button.released) {
      sendHidKey(4, false);
    }
  }

#if 0
  {
    Button &button = buttons[1];
    button.update();
    if (button.hold) {
      int8_t delta = 5;
      if (usb_hid.ready()) {
        usb_hid.mouseMove(RID_MOUSE, delta, delta); // right + down
      }
    }
  }

  {
    Button &button = buttons[2];
    button.update();
    if (button.pressed) {
      if (usb_hid.ready()) {
        usb_hid.sendReport16(RID_CONSUMER_CONTROL, HID_USAGE_CONSUMER_VOLUME_DECREMENT);
      }
    }
    if (button.released) {
      if (usb_hid.ready()) {
        usb_hid.sendReport16(RID_CONSUMER_CONTROL, 0);
      }
    }
  }
#endif

  {
    Button &button = buttons[3];
    button.update();
    if (button.pressed) {
      if (usb_hid.ready()) {
        rawHidTxBuf[0] = 100;
        rawHidTxBuf[1] = 200;
        usb_hid.sendReport(RID_GENERIC_INOUT, rawHidTxBuf, sizeof(rawHidTxBuf));
      }
    }
  }

  boardLED.write(1, buttons[3].hold);
}

static void debugShowBytes(uint8_t *buf, int len) {
  for (int i = 0; i < len; i++) {
    Serial.printf("%02X ", buf[i]);
  }
  Serial.printf("\n");
}

static void set_report_callback(uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize) {
  if (bufsize == RAWHID_REPORT_LEN + 1) {
    //retrieve report id from first byte
    report_id = buffer[0];
    buffer = buffer + 1;
    bufsize -= 1;
  }
  Serial.printf("received %d %d %d\n", report_id, report_type, bufsize);
  debugShowBytes((uint8_t *)buffer, bufsize);
  if (report_id == RID_GENERIC_INOUT && bufsize == RAWHID_REPORT_LEN) {
    boardLED.toggle(2);
    rawHidTxBuf[0] = 200;
    rawHidTxBuf[1] = 202;
    usb_hid.sendReport(RID_GENERIC_INOUT, rawHidTxBuf, RAWHID_REPORT_LEN);
  }
}

void app11Entry() {
  boardLED.initialize();

  USBDevice.setID(0xF055, 0xA57A);

  usb_hid.setReportCallback(NULL, set_report_callback);
  usb_hid.begin();

  boardLED.write(0, true);
  while (!USBDevice.mounted())
    delay(1);
  boardLED.write(0, false);

  while (1) {
    static int cnt = 0;
    if (cnt % 1000 == 0) {
      boardLED.write(0, true);
    }
    if (cnt % 1000 == 1) {
      boardLED.write(0, false);
    }
    if (cnt % 10 == 0) {
      updateButton();
    }
    cnt++;
    yield();
    delay(1);
  }
}
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/regs/usb.h"
#include "hardware/structs/resets.h"
#include "hardware/structs/usb.h"
#include "hardware/structs/usb_dpram.h"
#include "hardware/irq.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"

#include "device/usbd.h"
#include "bsp/board_api.h"
#include "tusb.h"
#include "pico/unique_id.h"
/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bsp/board_api.h"
#include "tusb.h"

#include "usb_descriptors.h"

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+

/* Blink pattern
 * - 250 ms  : device not mounted
 * - 1000 ms : device mounted
 * - 2500 ms : device is suspended
 */
// Test button: GPIO pulled high internally, active-low when shorted to GND.
// Replace with a real key pin once matrix scanning is wired up.
#define BOARD_BUTTON_PIN 15

enum {
  BLINK_NOT_MOUNTED = 250,
  BLINK_MOUNTED     = 1000,
  BLINK_SUSPENDED   = 2500,
};

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

void led_blinking_task(void);
void hid_task(void);

/*------------- MAIN -------------*/
int main(void) {
  board_init();

  // init device stack on configured roothub port
  tusb_rhport_init_t dev_init = {.role = TUSB_ROLE_DEVICE, .speed = TUSB_SPEED_AUTO};
  tusb_init(BOARD_TUD_RHPORT, &dev_init);

  board_init_after_tusb();

  while (1) {
    tud_task(); // tinyusb device task
    led_blinking_task();
    hid_task();
  }
}

// Each "platform" (like the RP2350) must define its own board_init() function.
// This comes from the datasheet's USB section: 
// https://github.com/raspberrypi/pico-examples/blob/master/usb/device/dev_lowlevel/dev_lowlevel.c#L183-L217
void board_init() {
  // Initialize onboard LED GPIO so board_led_write() works
  gpio_init(PICO_DEFAULT_LED_PIN);
  gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

  // Initialize test button pin (active-low, internal pull-up)
  // Swap BOARD_BUTTON_PIN for a real key GPIO once matrix scanning is implemented
  gpio_init(BOARD_BUTTON_PIN);
  gpio_set_dir(BOARD_BUTTON_PIN, GPIO_IN);
  gpio_pull_up(BOARD_BUTTON_PIN);

  // Reset usb controller
  reset_unreset_block_num_wait_blocking(RESET_USBCTRL);
  
  // Clear any previous state in dpram just in case
  memset(usb_dpram, 0, sizeof(*usb_dpram));
  
  // Enable USB interrupt at processor
  irq_set_enabled(USBCTRL_IRQ, true);
  
  // Mux the controller to the onboard usb phy
  usb_hw->muxing = USB_USB_MUXING_TO_PHY_BITS | USB_USB_MUXING_SOFTCON_BITS;
  
  // Force VBUS detect so the device thinks it is plugged into a host
  usb_hw->pwr = USB_USB_PWR_VBUS_DETECT_BITS | USB_USB_PWR_VBUS_DETECT_OVERRIDE_EN_BITS;
  
  // Enable the USB controller in device mode.
  usb_hw->main_ctrl = USB_MAIN_CTRL_CONTROLLER_EN_BITS;
  
  // Enable an interrupt per EP0 transaction
  usb_hw->sie_ctrl = USB_SIE_CTRL_EP0_INT_1BUF_BITS;
  
  // Enable interrupts for when a buffer is done, when the bus is reset,
  
  // and when a setup packet is received
  usb_hw->inte = USB_INTS_BUFF_STATUS_BITS |
  USB_INTS_BUS_RESET_BITS |
  USB_INTS_SETUP_REQ_BITS;
  
  usb_hw_t *usb_hw_set = (usb_hw_t *)hw_set_alias_untyped(usb_hw);
  // Present full speed device by enabling pull up on DP
  usb_hw_set->sie_ctrl = USB_SIE_CTRL_PULLUP_EN_BITS;
}

void board_init_after_tusb(void) {
  // Nothing needed post-TinyUSB init on RP2040
}

size_t board_get_unique_id(uint8_t id[], size_t max_len) {
  pico_unique_board_id_t pico_id;
  pico_get_unique_board_id(&pico_id);
  size_t len = PICO_UNIQUE_BOARD_ID_SIZE_BYTES;
  if (len > max_len) len = max_len;
  memcpy(id, pico_id.id, len);
  return len;
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void) {
  blink_interval_ms = BLINK_MOUNTED;
  printf("mounted\n");
}

// Invoked when device is unmounted
void tud_umount_cb(void) {
  blink_interval_ms = BLINK_NOT_MOUNTED;
  printf("unmounted\n");
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perfaaaaaaaaaaaorm remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en) {
  (void)remote_wakeup_en;
  blink_interval_ms = BLINK_SUSPENDED;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void) {
  blink_interval_ms = tud_mounted() ? BLINK_MOUNTED : BLINK_NOT_MOUNTED;
}

//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+

static void send_hid_report(uint8_t report_id, uint32_t btn) {
  // skip if hid is not ready yet
  if (!tud_hid_ready()) {
    return;
  }

  switch (report_id) {
    case REPORT_ID_KEYBOARD: {
      // use to avoid send multiple consecutive zero report for keyboard
      static bool has_keyboard_key = false;

      if (btn != 0u) {
        uint8_t keycode[6] = {0};
        keycode[0]         = HID_KEY_A;

        tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, keycode);
        has_keyboard_key = true;
      } else {
        // send empty key report if previously has key pressed
        if (has_keyboard_key) {
          tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);
        }
        has_keyboard_key = false;
      }
      break;
    }

    case REPORT_ID_MOUSE: {
      int8_t const delta = 5;

      // no button, right + down, no scroll, no pan
      tud_hid_mouse_report(REPORT_ID_MOUSE, 0x00, delta, delta, 0, 0);
      break;
    }

    case REPORT_ID_CONSUMER_CONTROL: {
      // use to avoid send multiple consecutive zero report
      static bool has_consumer_key = false;

      if (btn != 0u) {
        // volume down
        uint16_t volume_down = HID_USAGE_CONSUMER_VOLUME_DECREMENT;
        tud_hid_report(REPORT_ID_CONSUMER_CONTROL, &volume_down, 2);
        has_consumer_key = true;
      } else {
        // send empty key report (release key) if previously has key pressed
        uint16_t empty_key = 0;
        if (has_consumer_key) {
          tud_hid_report(REPORT_ID_CONSUMER_CONTROL, &empty_key, 2);
        }
        has_consumer_key = false;
      }
      break;
    }

    case REPORT_ID_GAMEPAD: {
      // use to avoid send multiple consecutive zero report for keyboard
      static bool has_gamepad_key = false;

      hid_gamepad_report_t report = {.x = 0, .y = 0, .z = 0, .rz = 0, .rx = 0, .ry = 0, .hat = 0, .buttons = 0};

      if (btn != 0u) {
        report.hat     = GAMEPAD_HAT_UP;
        report.buttons = GAMEPAD_BUTTON_A;
        tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));

        has_gamepad_key = true;
      } else {
        report.hat     = GAMEPAD_HAT_CENTERED;
        report.buttons = 0;
        if (has_gamepad_key) {
          tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));
        }
        has_gamepad_key = false;
      }
      break;
    }

    default: break; // unknown report id
  }
}

// Every 10ms, we will sent 1 report for each HID profile (keyboard, mouse etc ..)
// tud_hid_report_complete_cb() is used to send the next report after previous one is complete
void hid_task(void) {
  // Poll every 10ms
  const uint32_t  interval_ms = 10;
  static uint32_t start_ms    = 0;

  if (to_ms_since_boot(get_absolute_time()) - start_ms < interval_ms) {
    return; // not enough time
  }
  start_ms += interval_ms;

  uint32_t const btn = !gpio_get(BOARD_BUTTON_PIN);

  // Remote wakeup
  if (tud_suspended() && btn != 0u) {
    // Wake up host if we are in suspend mode
    // and REMOTE_WAKEUP feature is enabled by host
    tud_remote_wakeup();
  } else {
    // Send the 1st of report chain, the rest will be sent by tud_hid_report_complete_cb()
    send_hid_report(REPORT_ID_KEYBOARD, btn);
  }
}

// Invoked when sent REPORT successfully to host
// Application can use this to send the next report
// Note: For composite reports, report[0] is report ID
void tud_hid_report_complete_cb(uint8_t instance, uint8_t const *report, uint16_t len) {
  (void)instance;
  (void)len;

  uint8_t next_report_id = report[0] + 1u;

  if (next_report_id < REPORT_ID_COUNT) {
    send_hid_report(next_report_id, !gpio_get(BOARD_BUTTON_PIN));
  }
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(
    uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen) {
  // TODO not Implemented
  (void)instance;
  (void)report_id;
  (void)report_type;
  (void)buffer;
  (void)reqlen;

  return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(
    uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize) {
  (void)instance;

  if (report_type == HID_REPORT_TYPE_OUTPUT) {
    // Set keyboard LED e.g Capslock, Numlock etc...
    if (report_id == REPORT_ID_KEYBOARD) {
      // bufsize should be (at least) 1
      if (bufsize < 1) {
        return;
      }

      uint8_t const kbd_leds = buffer[0];

      if ((kbd_leds & KEYBOARD_LED_CAPSLOCK) != 0u) {
        // Capslock On: disable blink, turn led on
        blink_interval_ms = 0;
          gpio_put(PICO_DEFAULT_LED_PIN, true);
      } else {
        // Caplocks Off: back to normal blink
         gpio_put(PICO_DEFAULT_LED_PIN, false);
        blink_interval_ms = BLINK_MOUNTED;
      }
    }
  }
}

//--------------------------------------------------------------------+
// BLINKING TASK
//--------------------------------------------------------------------+
void led_blinking_task(void) {
  static uint32_t start_ms  = 0;
  static bool     led_state = false;

  // blink is disabled
  if (0u == blink_interval_ms) {
    return;
  }

  // Blink every interval ms
  if (to_ms_since_boot(get_absolute_time()) - start_ms < blink_interval_ms) {
    return; // not enough time
  }
  start_ms += blink_interval_ms;

  gpio_put(PICO_DEFAULT_LED_PIN, led_state);
  led_state = 1 - led_state; // toggle
}
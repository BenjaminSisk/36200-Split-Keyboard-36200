#include "hardware/structs/usb.h"
#include "hardware/structs/usb_dpram.h"
#include "hardware/regs/usb.h"
#include "tusb.h"
#include "usb_descriptors.h"
#include "device/usbd.h"
#include "bsp/board_api.h"

#define BOARD_BUTTON_PIN 15

enum {
  BLINK_NOT_MOUNTED = 250,
  BLINK_MOUNTED     = 1000,
  BLINK_SUSPENDED   = 2500,
};

/* Blink pattern
* - 250 ms  : device not mounted
* - 1000 ms : device mounted
* - 2500 ms : device is suspended
*/

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

// Joystick state updated by hid_task() each poll, read by send_hid_report().
// Stored here so tud_hid_report_complete_cb (fixed TinyUSB signature) can reach it.
static int8_t s_joy_x = 0;
static int8_t s_joy_y = 0;
static bool   s_joy_pressed = false;

// File-scope pointer set in main() so tud_hid_report_complete_cb (fixed TinyUSB
// signature, no user-data parameter) can forward calls to send_hid_report.
static InputHandler* g_inputHandler = nullptr;

void hid_task(InputHandler& inputHandler);

/**
 * @brief Translates an equipment ID into the corresponding USB HID keycode.
 * @param equipmentId The equipment ID assigned in hardwareMap.
 * @return HID keycode (e.g. HID_KEY_A), or 0 if the ID has no mapping.
 */
uint8_t mapEquipmentToHidCode(uint8_t equipmentId) {
    char c = qwertyMap::getChar(equipmentId);

    if (c >= 'a' && c <= 'z') {
        // e.g., if c is 'c', ('c' - 'a') is 2. HID_KEY_A + 2 = HID_KEY_C.
        return HID_KEY_A + (c - 'a'); 
    }

    switch (c) {
        case '\n': return HID_KEY_ENTER;
        case ' ':  return HID_KEY_SPACE;
        case '[':  return HID_KEY_BRACKET_LEFT;
        case '\\': return HID_KEY_BACKSLASH;
        case ';':  return HID_KEY_SEMICOLON;
        case '\'': return HID_KEY_APOSTROPHE;
        case ',':  return HID_KEY_COMMA;
        case '.':  return HID_KEY_PERIOD;
        case '/':  return HID_KEY_SLASH;
        
        // Return 0 for '\0' (out of bounds) or unmapped slots
        default:   return 0; 
    }

}

/**
 * @brief Initialises RP2350 USB hardware and GPIO for device mode.
 * Must be called before tusb_init(). Configures the USB PHY, pulls up DP,
 * and sets up the test button GPIO. Derived from the RP2350 USB datasheet:
 * https://github.com/raspberrypi/pico-examples/blob/master/usb/device/dev_lowlevel/dev_lowlevel.c
 */
void board_init() {
  // Initialize onboard LED GPIO so board_led_write() works

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

/// @brief Called by TinyUSB after its own initialisation. No-op on RP2350.
void board_init_after_tusb(void) {
  // Nothing needed post-TinyUSB init on RP2040
}

/**
 * @brief Fills a buffer with the board's unique 64-bit identifier.
 * @param id      Buffer to receive the ID bytes.
 * @param max_len Maximum bytes to write into id.
 * @return Number of bytes written.
 */
size_t board_get_unique_id(uint8_t id[], size_t max_len) {
  pico_unique_board_id_t pico_id;
  pico_get_unique_board_id(&pico_id);
  size_t len = PICO_UNIQUE_BOARD_ID_SIZE_BYTES;
  if (len > max_len) len = max_len;
  memcpy(id, pico_id.id, len);
  return len;
}

/// @brief TinyUSB callback: invoked when the host mounts the device.
void tud_mount_cb(void) {
  blink_interval_ms = BLINK_MOUNTED;
  printf("mounted\n");
}

/// @brief TinyUSB callback: invoked when the host unmounts the device.
void tud_umount_cb(void) {
  blink_interval_ms = BLINK_NOT_MOUNTED;
  printf("unmounted\n");
}

/**
 * @brief TinyUSB callback: invoked when the USB bus is suspended.
 * Device must drop average current below 2.5 mA within 7 ms of this call.
 * @param remote_wakeup_en True if the host permits the device to issue remote wakeup.
 */
void tud_suspend_cb(bool remote_wakeup_en) {
  (void)remote_wakeup_en;
  blink_interval_ms = BLINK_SUSPENDED;
}

/// @brief TinyUSB callback: invoked when the USB bus resumes after suspension.
void tud_resume_cb(void) {
  blink_interval_ms = tud_mounted() ? BLINK_MOUNTED : BLINK_NOT_MOUNTED;
}

/**
 * @brief Builds and sends a single HID report for the given report ID.
 * Called from hid_task() to start the chain and from tud_hid_report_complete_cb()
 * to advance it.
 * @param report_id    One of the REPORT_ID_* constants from usb_descriptors.h.
 * @param inputHandler Reference to the InputHandler for reading live key/joystick state.
 */
static void send_hid_report(uint8_t report_id, InputHandler& inputHandler) {
  // skip if hid is not ready yet
  if (!tud_hid_ready()) {
    return;
  }

  switch (report_id) {
    case REPORT_ID_KEYBOARD: {
      // use to avoid send multiple consecutive zero report for keyboard
      static bool has_keyboard_key = false;

      std::vector<uint8_t> activeIds = inputHandler.getActiveEquipmentIds();
      uint8_t keycodes[6] = {0};
      uint8_t count = 0;
      for (uint8_t id : activeIds) {
          if (count >= 6) break;
          uint8_t hid_code = mapEquipmentToHidCode(id);
          if (hid_code != 0) keycodes[count++] = hid_code;
      }

      if (count > 0) {
          tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, keycodes);
          has_keyboard_key = true;
      } else if (has_keyboard_key) {
          tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);
          has_keyboard_key = false;
      }
      break;
    }

    case REPORT_ID_MOUSE: {
      tud_hid_mouse_report(REPORT_ID_MOUSE, 0x00, s_joy_x, s_joy_y, 0, 0);
      break;
    }

    case REPORT_ID_CONSUMER_CONTROL: {
      // Not yet implemented; sends empty report to keep the chain moving.
      static bool has_consumer_key = false;
      uint16_t empty_key = 0;
      if (has_consumer_key) {
        tud_hid_report(REPORT_ID_CONSUMER_CONTROL, &empty_key, 2);
        has_consumer_key = false;
      }
      break;
    }

    case REPORT_ID_GAMEPAD: {
      // use to avoid send multiple consecutive zero report for keyboard
      static bool has_gamepad_key = false;

      hid_gamepad_report_t report = {.x = 0, .y = 0, .z = 0, .rz = 0, .rx = 0, .ry = 0, .hat = 0, .buttons = 0};

      report.x = s_joy_x;
      report.y = s_joy_y;
      if (s_joy_pressed) {
          report.buttons |= GAMEPAD_BUTTON_A;
      }

      if (s_joy_x != 0 || s_joy_y != 0 || s_joy_pressed) {
          tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));
          has_gamepad_key = true;
      } else if (has_gamepad_key) {
          tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));
          has_gamepad_key = false;
      }
      break;
    }

    default: break; // unknown report id
  }
}

/**
 * @brief Polls HID state every 10 ms and kicks off the composite report chain.
 * Updates joystick delta state and sends the first report (keyboard); the rest
 * of the chain (mouse, consumer, gamepad) is driven by tud_hid_report_complete_cb().
 * @param inputHandler Reference to the InputHandler for reading key and joystick state.
 */
void hid_task(InputHandler& inputHandler) {
  // Poll every 10ms
  const uint32_t  interval_ms = 10;
  static uint32_t start_ms    = 0;

  if (to_ms_since_boot(get_absolute_time()) - start_ms < interval_ms) {
    return; // not enough time
  }
  start_ms += interval_ms;

  uint32_t const btn = !gpio_get(BOARD_BUTTON_PIN);

  // Convert 12-bit joystick (0-4095, center=2048) to signed mouse deltas.
  // Dead zone prevents cursor drift from ADC noise at rest.
  const int16_t center      = 2048;
  const int16_t dead_zone   = 200;
  const int16_t sensitivity = 200;
  int16_t rx = (int16_t)inputHandler.getJoystickX() - center;
  int16_t ry = (int16_t)inputHandler.getJoystickY() - center;
  s_joy_x       = (abs(rx) > dead_zone) ? (int8_t)(rx / sensitivity) : 0;
  s_joy_y       = (abs(ry) > dead_zone) ? (int8_t)(ry / sensitivity) : 0;
  s_joy_pressed = inputHandler.getJoystickPressed();

  // Remote wakeup
  if (tud_suspended() && btn != 0u) {
    tud_remote_wakeup();
  } else {
    // Send the 1st of report chain, the rest will be sent by tud_hid_report_complete_cb()
    send_hid_report(REPORT_ID_KEYBOARD, inputHandler);
  }
}

/**
 * @brief TinyUSB callback: invoked after a HID report is successfully sent.
 * Advances the composite report chain by sending the next report ID in sequence.
 * @param instance HID interface instance index.
 * @param report   Pointer to the report that just completed; report[0] is the report ID.
 * @param len      Length in bytes of the completed report.
 */
void tud_hid_report_complete_cb(uint8_t instance, uint8_t const *report, uint16_t len) {
  (void)instance;
  (void)len;

  uint8_t next_report_id = report[0] + 1u;

  if (next_report_id < REPORT_ID_COUNT) {
    if (g_inputHandler) send_hid_report(next_report_id, *g_inputHandler);
  }
}

/**
 * @brief TinyUSB callback: invoked on a GET_REPORT control request from the host.
 * @param instance    HID interface instance index.
 * @param report_id   Requested report ID.
 * @param report_type HID report type (input / output / feature).
 * @param buffer      Buffer to fill with the report data.
 * @param reqlen      Maximum bytes the host is requesting.
 * @return Number of bytes written into buffer, or 0 to STALL the request.
 */
uint16_t tud_hid_get_report_cb(
    uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen) {
  // Not implemented — returns 0 to STALL the request.
  (void)instance;
  (void)report_id;
  (void)report_type;
  (void)buffer;
  (void)reqlen;

  return 0;
}

/**
 * @brief TinyUSB callback: invoked on a SET_REPORT request or OUT endpoint data.
 * Used to receive keyboard LED state (Caps Lock, Num Lock, etc.) from the host.
 * @param instance    HID interface instance index.
 * @param report_id   Report ID of the incoming data.
 * @param report_type HID report type.
 * @param buffer      Incoming report data.
 * @param bufsize     Length of buffer in bytes.
 */
void tud_hid_set_report_cb(
    uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize) {
  (void)instance;

  if (report_type == HID_REPORT_TYPE_OUTPUT) {
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
#pragma once
#include <array>
#include <cstdint>

namespace qwertyMap {

    // USB HID keyboard keycodes (USB HID Usage Tables §10, Keyboard/Keypad page 0x07)
    static constexpr uint8_t K_NONE  = 0x00;
    static constexpr uint8_t K_A     = 0x04;
    static constexpr uint8_t K_B     = 0x05;
    static constexpr uint8_t K_C     = 0x06;
    static constexpr uint8_t K_D     = 0x07;
    static constexpr uint8_t K_E     = 0x08;
    static constexpr uint8_t K_F     = 0x09;
    static constexpr uint8_t K_G     = 0x0A;
    static constexpr uint8_t K_H     = 0x0B;
    static constexpr uint8_t K_I     = 0x0C;
    static constexpr uint8_t K_J     = 0x0D;
    static constexpr uint8_t K_K     = 0x0E;
    static constexpr uint8_t K_L     = 0x0F;
    static constexpr uint8_t K_M     = 0x10;
    static constexpr uint8_t K_N     = 0x11;
    static constexpr uint8_t K_O     = 0x12;
    static constexpr uint8_t K_P     = 0x13;
    static constexpr uint8_t K_Q     = 0x14;
    static constexpr uint8_t K_R     = 0x15;
    static constexpr uint8_t K_S     = 0x16;
    static constexpr uint8_t K_T     = 0x17;
    static constexpr uint8_t K_U     = 0x18;
    static constexpr uint8_t K_V     = 0x19;
    static constexpr uint8_t K_W     = 0x1A;
    static constexpr uint8_t K_X     = 0x1B;
    static constexpr uint8_t K_Y     = 0x1C;
    static constexpr uint8_t K_Z     = 0x1D;
    static constexpr uint8_t K_1     = 0x1E;
    static constexpr uint8_t K_2     = 0x1F;
    static constexpr uint8_t K_3     = 0x20;
    static constexpr uint8_t K_4     = 0x21;
    static constexpr uint8_t K_5     = 0x22;
    static constexpr uint8_t K_6     = 0x23;
    static constexpr uint8_t K_7     = 0x24;
    static constexpr uint8_t K_8     = 0x25;
    static constexpr uint8_t K_9     = 0x26;
    static constexpr uint8_t K_0     = 0x27;
    static constexpr uint8_t K_ENT   = 0x28;
    static constexpr uint8_t K_ESC   = 0x29;
    static constexpr uint8_t K_BSP   = 0x2A;
    static constexpr uint8_t K_TAB   = 0x2B;
    static constexpr uint8_t K_SPC   = 0x2C;
    static constexpr uint8_t K_MINUS = 0x2D;
    static constexpr uint8_t K_EQL   = 0x2E;
    static constexpr uint8_t K_LBRK  = 0x2F;
    static constexpr uint8_t K_RBRK  = 0x30;
    static constexpr uint8_t K_BSLS  = 0x31;
    static constexpr uint8_t K_SEMI  = 0x33;
    static constexpr uint8_t K_QUOT  = 0x34;
    static constexpr uint8_t K_GRV   = 0x35;
    static constexpr uint8_t K_COMM  = 0x36;
    static constexpr uint8_t K_DOT   = 0x37;
    static constexpr uint8_t K_SLSH  = 0x38;
    static constexpr uint8_t K_CAPS  = 0x39;
    static constexpr uint8_t K_F1    = 0x3A;
    static constexpr uint8_t K_F2    = 0x3B;
    static constexpr uint8_t K_F3    = 0x3C;
    static constexpr uint8_t K_F4    = 0x3D;
    static constexpr uint8_t K_F5    = 0x3E;
    static constexpr uint8_t K_F6    = 0x3F;
    static constexpr uint8_t K_F7    = 0x40;
    static constexpr uint8_t K_F8    = 0x41;
    static constexpr uint8_t K_F9    = 0x42;
    static constexpr uint8_t K_F10   = 0x43;
    static constexpr uint8_t K_F11   = 0x44;
    static constexpr uint8_t K_F12   = 0x45;
    static constexpr uint8_t K_DEL   = 0x4C;
    static constexpr uint8_t K_HOME  = 0x4A;
    static constexpr uint8_t K_PGUP  = 0x4B;
    static constexpr uint8_t K_END   = 0x4D;
    static constexpr uint8_t K_PGDN  = 0x4E;
    static constexpr uint8_t K_ARTR  = 0x4F;
    static constexpr uint8_t K_ARTL  = 0x50;
    static constexpr uint8_t K_ARTD  = 0x51;
    static constexpr uint8_t K_ARTU  = 0x52;

    // HID keyboard modifier bits (modifier byte, §10 Table 12)
    static constexpr uint8_t M_NONE  = 0x00;
    static constexpr uint8_t M_LCTL  = 0x01;
    static constexpr uint8_t M_LSFT  = 0x02;
    static constexpr uint8_t M_LALT  = 0x04;
    static constexpr uint8_t M_LGUI  = 0x08;
    static constexpr uint8_t M_RALT  = 0x40; // AltGr / Right Alt

    static constexpr uint8_t NUM_LAYERS = 4;
    static constexpr uint8_t NUM_KEYS   = 48;

    /**
     * Equipment ID → HID keycode per layer.
     *
     * Physical layout (matrixToId after update):
     *   Row 0  IDs  0-11: Tab  Q  W  E  R  T  |  Y  U  I  O  P  Del
     *   Row 1  IDs 12-23: Alt  A  S  D  F  G  |  H  J  K  L  '  AltGr
     *   Row 2  IDs 24-35: Sft  Z  X  C  V  B  |  N  M  ,  .  /  Func
     *   Row 3  IDs 36-41: [36=unoccupied]  Ctrl  Enter  Tab  [LayPrev]  [LayNext]  Bsp  Space  Sft-R
     *          IDs 44-46: Tab(L0)  LayPrev  LayNext
     *
     * Layers:  0=QWERTY  1=Lower(symbols/numbers)  2=Raise(nav)  3=Fn(F-keys)
     * "None" in CSV = K_NONE (dead key, not transparent).
     */
    constexpr std::array<std::array<uint8_t, NUM_KEYS>, NUM_LAYERS> keycodes = {{

        // ── Layer 0: base QWERTY ─────────────────────────────────────────────────
        //  ID:  0        1        2        3        4        5        6        7        8        9       10       11
        {{  K_TAB,  K_Q,    K_W,    K_E,    K_R,    K_T,    K_Y,    K_U,    K_I,    K_O,    K_P,    K_DEL,
        //  12       13       14       15       16       17       18       19       20       21       22       23
            K_NONE, K_A,    K_S,    K_D,    K_F,    K_G,    K_H,    K_J,    K_K,    K_L,    K_QUOT, K_NONE,
        //  24       25       26       27       28       29       30       31       32       33       34       35
            K_NONE, K_Z,    K_X,    K_C,    K_V,    K_B,    K_N,    K_M,    K_COMM, K_DOT,  K_SLSH, K_NONE,
        //  36       37       38       39       40       41       42       43       44       45       46       47
            K_NONE, K_NONE, K_ENT,  K_BSP,  K_SPC,  K_NONE, K_NONE, K_NONE, K_TAB,  K_NONE, K_NONE, K_NONE }},
        //          ^Ctrl(mod) ^Enter  ^Bsp    ^Space  ^Sft(mod)       ^Joy SW         ^Tab    ^LayPrev ^LayNext

        // ── Layer 1: Lower — symbols (top row) + numbers (home row) ─────────────
        //   Q=!  W=@  E=#  R=$  T=%  Y=^  U=&  I=*  O=(  P=)   (Shift applied via modifiers[])
        //   A=1  S=2  D=3  F=4  G=5  H=6  J=7  K=8  L=9  '=0
        //   X=~  C='  V=[  B={  N=}  M=]
        //   Right-thumb Sft position → : (colon, Shift+; via modifiers[])
        {{  K_TAB,  K_1,    K_2,    K_3,    K_4,    K_5,    K_6,    K_7,    K_8,    K_9,    K_0,    K_DEL,
            K_NONE, K_1,    K_2,    K_3,    K_4,    K_5,    K_6,    K_7,    K_8,    K_9,    K_0,    K_NONE,
            K_NONE, K_NONE, K_GRV,  K_QUOT, K_LBRK, K_LBRK, K_RBRK, K_RBRK, K_NONE, K_NONE, K_NONE, K_NONE,
            K_NONE, K_NONE, K_NONE, K_NONE, K_NONE, K_SEMI, K_NONE, K_NONE, K_NONE, K_NONE, K_NONE, K_NONE }},
        //                                                   ^: (colon)

        // ── Layer 2: Raise — navigation & symbols ───────────────────────────────
        //   Q=Del  E=_  R=+  T=PgUp  O=\  P=|
        //   A=Home  S=End  D=-  F==  G=PgDn  H=←  J=↓  K=↑  L=→
        //   Z=<  X=>  C=Copy(Ctrl+C)  V=Paste(Ctrl+V)  B=;
        //   N-,=media stubs   Ctrl-thumb=Esc  Enter-thumb=Win(GUI)
        {{  K_TAB,  K_DEL,  K_NONE, K_MINUS,K_EQL,  K_PGUP, K_NONE, K_NONE, K_NONE, K_BSLS, K_BSLS, K_DEL,
            K_NONE, K_HOME, K_END,  K_MINUS,K_EQL,  K_PGDN, K_ARTL, K_ARTD, K_ARTU, K_ARTR, K_NONE, K_NONE,
            K_NONE, K_COMM, K_DOT,  K_C,    K_V,    K_SEMI, K_NONE, K_NONE, K_NONE, K_NONE, K_NONE, K_NONE,
            K_NONE, K_ESC,  K_NONE, K_NONE, K_NONE, K_NONE, K_NONE, K_NONE, K_NONE, K_NONE, K_NONE, K_NONE }},
        //          ^Esc    ^Win(mod)

        // ── Layer 3: Fn — function keys ─────────────────────────────────────────
        //   Q=F1  W=F2  E=F3  R=F4  T=F5  Y=F6  U=F7  I=F8  O=F9  P=F10
        //   A=F11  S=F12
        //   Z=CapsLock   /=Reset(stub)
        {{  K_TAB,  K_F1,   K_F2,   K_F3,   K_F4,   K_F5,   K_F6,   K_F7,   K_F8,   K_F9,   K_F10,  K_DEL,
            K_NONE, K_F11,  K_F12,  K_NONE, K_NONE, K_NONE, K_NONE, K_NONE, K_NONE, K_NONE, K_NONE, K_NONE,
            K_NONE, K_CAPS, K_NONE, K_NONE, K_NONE, K_NONE, K_NONE, K_NONE, K_NONE, K_NONE, K_NONE, K_NONE,
            K_NONE, K_NONE, K_NONE, K_NONE, K_NONE, K_NONE, K_NONE, K_NONE, K_NONE, K_NONE, K_NONE, K_NONE }}
    }};

    /**
     * HID modifier byte per layer × equipment ID.
     * Accumulated with all active keys each HID report cycle.
     */
    constexpr std::array<std::array<uint8_t, NUM_KEYS>, NUM_LAYERS> modifiers = {{

        // Layer 0: physical modifier keys
        // Alt=12  AltGr=23  Shift-L=24  Ctrl=37  Shift-R=41
        {{ M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE,
           M_LALT, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_RALT,
           M_LSFT, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE,
           M_NONE, M_LCTL, M_NONE, M_NONE, M_NONE, M_LSFT, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE }},
        //         ^Ctrl                                    ^Shift-R

        // Layer 1: top-row symbols need Shift; {,} need Shift; colon needs Shift
        // IDs 1-10 → Shift (! @ # $ % ^ & * ( ))
        // ID 26 → Shift (~ = Shift+`)
        // IDs 29,30 → Shift ({ = Shift+[, } = Shift+])
        // ID 41 → Shift (: = Shift+;)
        {{ M_NONE, M_LSFT, M_LSFT, M_LSFT, M_LSFT, M_LSFT, M_LSFT, M_LSFT, M_LSFT, M_LSFT, M_LSFT, M_NONE,
           M_LALT, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_LCTL,
           M_LSFT, M_NONE, M_LSFT, M_NONE, M_NONE, M_LSFT, M_LSFT, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE,
           M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_LSFT, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE }},

        // Layer 2: _ + | < > need Shift; Copy/Paste need Ctrl; Win key uses LGUI modifier
        // ID 3 → Shift (_)   ID 4 → Shift (+)   ID 10 → Shift (|)
        // ID 25 → Shift (<)  ID 26 → Shift (>)
        // ID 27 → Ctrl (Copy) ID 28 → Ctrl (Paste)
        // ID 38 → LGUI (Windows key)
        {{ M_NONE, M_NONE, M_NONE, M_LSFT, M_LSFT, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_LSFT, M_NONE,
           M_LALT, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_LCTL,
           M_LSFT, M_LSFT, M_LSFT, M_LCTL, M_LCTL, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE,
           M_NONE, M_NONE, M_LGUI, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE }},

        // Layer 3: Alt/Shift/Ctrl column keys persist; F-keys have no modifiers
        {{ M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE,
           M_LALT, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_LCTL,
           M_LSFT, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE,
           M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE, M_NONE }}
    }};

    // Active layer, updated by hid_task() on each layer-cycle key press.
    // 0=QWERTY  1=Lower  2=Raise  3=Fn
    inline uint8_t currentLayer = 0;

    inline uint8_t getCode(uint8_t equipmentId, uint8_t layer = currentLayer) {
        if (layer >= NUM_LAYERS || equipmentId >= NUM_KEYS) return K_NONE;
        return keycodes[layer][equipmentId];
    }

    inline uint8_t getModifier(uint8_t equipmentId, uint8_t layer = currentLayer) {
        if (layer >= NUM_LAYERS || equipmentId >= NUM_KEYS) return M_NONE;
        return modifiers[layer][equipmentId];
    }
}

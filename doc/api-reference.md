---
layout: default
title: API Reference
---

# API Reference — every class, struct, function and ISR

> Complete reference of the public interface of each module, with purpose, side effects, call relations and source excerpts. Line numbers refer to the repository as of commit `master` (May 2016). Diagrams and object relations: [class-map.md](class-map.md). Known defects are marked ⚠ and cross-linked to [state-of-code.md](state-of-code.md).

**Legend**: 🟢 active in this release · 🔶 compiled but unreachable from `main()` · ⛔ dead code

---

## 1. Conventions used by this codebase

- **`///` triple-slash comments** are in Italian and are the author's main documentation; they are quoted here (translated) where they explain intent.
- **Global orchestrators live in `main()`, hardware wrappers are globals** — see [class-map.md §4](class-map.md#4-object-inventory--the-concrete-instances).
- **`volatile` is used consistently** on everything shared with ISRs.
- **`PRINTF(...)`** (`uartp/uartstdio.c:1297`) is a custom `vsprintf`-based formatter writing to whichever UART `g_ui32Base` points to — selected by `setupUART(0|1)`.
- **`DI()` / `EI()`** (`gen_def.h:19-20`) wrap the Cortex-M `cpsid i` / `cpsie i` instructions.

```c
/// gen_def.h — interrupt gate macros
#define DI()  __asm("    cpsid   i\n")   // disable IRQ
#define EI()  __asm("    cpsie   i\n")   // enable IRQ
```

---

## 2. Main application — `main.cpp`

### `int main(void)` 🟢
**Lines 70–351.** Boot sequence, then the infinite task loop described in [architecture.md §3](architecture.md#3-main-loop-timeline-one-iteration).

Responsibilities, in order:

1. Assign hardware base addresses to the encoder objects:
   ```c
   ENC0.setAddr(QEI0_BASE);
   ENC1.setAddr(QEI1_BASE);
   ```
2. Instantiate orchestrators on the stack: `syn_stat synSTATO`, `xBee XB`, `comando CMD1` (and the unused `syntaxStatus synStat` ⚠ [duplicate](state-of-code.md#3-duplicate-and-parallel-implementations)).
3. Boot hardware: `DI()` → `setupMCU()` → `setupUART(0)` → `PRINT_WELCOME()` → `initTimer0(INT_STEP_10_MS)` → `ENC0/ENC1.qeiInit()` → `EI()` → `XB.test()`.
4. `while(1)`: demultiplex the UART1 ring buffer into `PARSE()`, answer valid frames with encoder bytes, and execute the 10 ms / 100 ms / 1 s cadenced blocks driven by `procCom` / `tick10` / `tick100`.

⚠ **Blocking console read at line 181**: `uint8_t valore_carattere = UARTCharGet(UART0_BASE);` stalls the whole loop until a character is typed on the debug console. The variable is never used.

⚠ **Undeclared identifiers under `#ifndef _DEBUG_`** (lines 294–313): `MISURE.d_mm[]` and `Rot.yaw/yawF/yawF0` are not declared in any translation unit — the release **only compiles with `_DEBUG_` defined**.

⚠ The little-endian byte-dump reply (lines 188–211) sends `PRINTF("%d", byte)` — decimal text of each byte, not raw binary, with the `posV` indexing `i` and `i+8` picking the first four slots of each half of the 64-slot ring.

### `uint8_t printFloat(double number, uint8_t digits)` 🟢
**Lines 355–394.** Minimal float-to-ASCII printer used because `PRINTF` has no `%f`. Handles sign, rounding, integer part and the requested number of decimals; returns 0. Also declared in `Giroscopio.h:76` (a header/implementation mismatch — it belongs to neither).

---

## 3. Boot and platform support — `gyro_init.h / gyro_init.c`

### `void setupMCU(void)` 🟢
**gyro_init.c:157–231.** Platform bring-up:

| Step | Detail |
|------|--------|
| Clock | `ROM_SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ)` → **80 MHz** |
| LEDs | GPIO F pins 1/2/3 as outputs; a red→blue→green blink parade as boot indicator |
| Start gate | PF0 unlocked, configured as input with pull-up; **blocks until PF0 reads 0** (`while(...) == GPIO_PIN_0`) — the robot waits for its start button |
| Debug pin | PB5 configured as output (also used later as XBee reset — ⚠ shared with `xBee::xBee()`) |

```c
/// gyro_init.c:199-204 — the start-button gate
/// legge il pin (ricordare lo shift di 2 posizione verso sinistra della maschera di bit
///  p. 654 U.G.)
valore = 10;
while(--valore);
while(HWREG(GPIO_PORTF_BASE + (GPIO_O_DATA + (GPIO_PIN_0 << 2))) == GPIO_PIN_0);
```

> Student note: the `<< 2` on the pin mask is a TM4C quirk — GPIO data registers alias across a 256-byte window; see [student-notes.md §8](student-notes.md#8-tiva-specific-idioms).

### `void setupUART(int num)` 🟢
**gyro_init.c:251–262.** Calls `ConfigureUART(115200, UART1)` **and** `ConfigureUART(115200, UART0)`, then points `PRINTF`'s target `g_ui32Base` at `g_ui32UARTBase[num]`. Parameter clamped to 0 above 1.

### `void PRINT_WELCOME(void)` 🟢
**gyro_init.c:266–274.** Banner + measured clock frequency on the console.

### `void attesa(int delay)` 🔶
**gyro_init.c:236–244.** Busy-wait for `delay` multiples of the 10 ms tick by polling `procCom`. Not called in this release.

### `void minQuad(int16_t *x, int16_t *y, int numS, float *b, float *a)` 🟢 (called by gyro calibration)
**gyro_init.c:279–304.** Ordinary least-squares fit `y = m·x + q` over `numS` samples; returns slope in `*b`, intercept in `*a`. Used by `Giroscopio::azzeraAssi()` to fit the Z-axis bias drift line (stored as `Giroscopio::m`, `Giroscopio::q`).

### `void blinkBlueLed() / blinkRedLed()` 🟢
**gyro_init.c:25–48.** Two long (~0.3 s each) LED blinks — used as "gyro found" / "accelerometer found" signals from `initGyro`.

---

## 4. Encoder subsystem

### 4.1 `class encQuad` — `encQuad.h / encQuad.cpp` 🟢

The C++ wrapper around the TM4C hardware QEI modules. Two instances exist: `ENC0` (QEI0) and `ENC1` (QEI1), globals in `main.cpp:66`.

| Member | Signature | Purpose |
|--------|-----------|---------|
| `setAddr` | `void setAddr(uint32_t add)` 🟢 inline | stores `QEI0_BASE`/`QEI1_BASE` in `address` |
| `qeiInit` | `void qeiInit()` 🟢 | full peripheral setup (below) |
| `readPos` | `void readPos()` 🟢 | `pos = QEIPositionGet(address); dist_mm = pos * 1.0` (⚠ conversion factor `k` is 1.0 — mm == counts) |
| `readVel` | `void readVel()` 🔶 | **empty body** — velocity is configured but never read |
| `readDir` | `void readDir()` 🟢 | `dir = QEIDirectionGet(address)` |
| `update` | `void update()` 🔶 | pos + vel + dir in one call (not used) |
| `intIDXEnable` | `void intIDXEnable()` 🔶 | `QEIIntEnable(address, QEI_INTINDEX)` — hardware index interrupt (alternative to the GPIO one actually used) |

`qeiInit()` flow (encQuad.cpp:60–206) — the most instructive peripheral setup in the project:

```c
/// encQuad.cpp (QEI0 branch, abridged)
SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
UnlockPD7_01();                              // PD7 is NMI-locked at reset!
GPIODirModeSet(GPIO_PORTD_BASE, GPIO_PIN_7 | GPIO_PIN_6, GPIO_DIR_MODE_IN);
SysCtlPeripheralEnable(SYSCTL_PERIPH_QEI0);
QEIDisable(address);
GPIOPinConfigure(GPIO_PD6_PHA0);
GPIOPinConfigure(GPIO_PD7_PHB0);
GPIOPinTypeQEI(GPIO_PORTD_BASE, GPIO_PIN_7 | GPIO_PIN_6);
ROM_GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_4);      // index on PF4
ROM_GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_RISING_EDGE);
ROM_IntPrioritySet(INT_GPIOF, 0x00);         // highest priority group
GPIOIntEnable(GPIO_PORTF_BASE, GPIO_INT_PIN_4);
ROM_IntEnable(INT_GPIOF);
QEIConfigure(address, (QEI_CONFIG_CAPTURE_A | QEI_CONFIG_RESET_IDX |
              QEI_CONFIG_QUADRATURE | QEI_CONFIG_NO_SWAP), fscala);
QEIPositionSet(address, zero_pos);
QEIVelocityDisable(address);
QEIVelocityConfigure(address, QEI_VELDIV_2, vel_period);    // 80MHz/10 = 8M ticks window
QEIVelocityEnable(address);
QEIEnable(address);
```

Default tuning (constructor, encQuad.cpp:43–54): `fscala = 40000`, `vel_period = ROM_SysCtlClockGet()/10` (= 10 ms window), `zero_pos = 0`. A `///TODO: tarare i due valori` comment marks both as uncalibrated.

### 4.2 ISRs of the encoder subsystem 🟢

#### `void IntGPIOf(void)` — encQuad.cpp:257–270 🟢
Runs on every **rising edge of PF4** (encoder index pulse). This is the heart of "rigaMagn":

```c
void IntGPIOf(void){
    GPIOIntClear(GPIO_PORTF_BASE, GPIO_INT_PIN_4);
    ENC0.pos = QEIPositionGet(QEI0_BASE);
    if (ENC0.pos > ENC0.fscala / 2){
            /// posizione negativa
            ENC0.pos -= ENC0.fscala / 2;
    }
    ENC0.posV[ENC0.indice++] = ENC0.pos;
    ENC0.indice &= 63;
    if (QEIDirectionGet(QEI0_BASE) == 1)
            ENC0.contIDX++;
    else
            ENC0.contIDX--;
}
```

What it does: snapshots the QEI0 position, folds the upper half of the counter range down (⚠ half-rectangle wrap — see [state-of-code.md §6](state-of-code.md#6-bugs-and-fragile-points-summary-table) — instead of producing negative values), pushes it into the 64-slot ring `posV[]`, and increments/decrements `contIDX` as a net-turns counter using the QEI direction bit.

#### `void IntEnc0(void)` — encQuad.cpp:249–253 ⛔
Clears QEI interrupts, commented "occorrerebbe contare il numero di IDX" ("one should count the IDX pulses"). **Not present in the vector table** — dead.

### 4.3 `void UnlockPD7_01()` — encQuad.cpp:32–41 🟢
Unlocks PD7 (default function: NMI). Writes `GPIO_LOCK_KEY` to `GPIO_O_LOCK`, opens the commit register for bit 7, forces alternate function via `GPIO_O_PCTL |= 0x60000000`, re-locks. Duplicated as `UnlockPD7()` in `qei.cpp:94-103` ⚠.

### 4.4 Legacy C API — `qei.h / qei.cpp` ⛔ (whole module)

| Symbol | Line | Status |
|--------|------|--------|
| `typedef struct _qei {...} qei` | qei.h:16–28 | struct with pos/vel/dir for both encoders |
| `void QEI0IntHandler()` | qei.cpp:31–55 | ⛔ not in vector table; ⚠ reads global `qei *p` which is **never initialized** (line 30) |
| `void QEI1IntHandler()` | qei.cpp:57–80 | ⛔ same, plus ⚠ copy-paste bug: `if ((status1 & QEI_INTDIR) == QEI_INTERROR)` compares the wrong mask |
| `void UnlockPD7()` | qei.cpp:94–103 | ⛔ duplicate of `UnlockPD7_01` |
| `void qei_init(qei *p)` | qei.cpp:249–372 | ⛔ C-style init of both QEI modules (superseded by `encQuad::qeiInit`) |
| `void qei_test(qei *p)` | qei.cpp:375–400 | 🔶 only reachable via commented `main` line 159 |
| `void qei_data(qei *p)` | qei.cpp:402–412 | ⛔ |
| `void qei_reset(qei *p)` | qei.cpp:414–418 | ⛔ |

About 170 additional lines of commented-out class code (`class QEI{...}`) remain inside the files — kept as a design diary.

---

## 5. Gyroscope subsystem

### 5.1 `class Giroscopio` — `Giroscopio.h / Giroscopio.cpp` 🔶 (class fully implemented; instantiated by commented code only)

Driver for the **ST L3GD20** MEMS gyroscope (I2C address `0x6B`, WHO_AM_I `0xD4`), integrating angular rate into an angle. In this release no `Giroscopio` object is constructed by `main()` — but the telemetry prints reference `Rot.*`, and `parse.h`'s `glb` struct carries a `Giroscopio *gyro` pointer: the class is the centre of gravity of the *next* release.

| Member | Signature | Purpose |
|--------|-----------|---------|
| `initGyro` | `void initGyro(char assi)` 🔶 | WHO_AM_I check → `blinkBlueLed()` → `setupAssi(assi)`; sets `IsPresent` |
| `setupAssi` | `void setupAssi(char stato)` 🔶 | bit0=x, bit1=y, bit2=z (`ALL_AXIS=7`, `Z_AXIS=4`); writes CTRL_REG1 = `0x08|stato`; sets FS 250 °/s (`FS = 250.0/32768.0`); empirical scale `kz = 1.140` |
| `misuraAngoli` | `void misuraAngoli()` 🔶 | **core**: reads STATUS_REG, then OUT_Z (± 6-byte burst for all axes), subtracts bias `z0`, rectangular integration `yawF += z·FS·kz·tick`; every 1000 samples (10 s) at rest performs bias drift compensation (`yawF = yawP`) or full recalibration; honours `IsRotating` and `offsetRequest` flags set by the PID |
| `azzeraAssi` | `void azzeraAssi()` 🔶 | bias calibration: 32 samples per axis (ALL_AXIS) or 128 Z samples (`numSampleBias`), stores mean in `x0/y0/z0` and fits the drift line via `minQuad()` |
| `primoAzzeramento` | `void primoAzzeramento()` 🔶 | waits 70 ticks then calls `azzeraAssi()` and prints `media`, `m`, `q` |
| `getTemp` | `int getTemp()` 🔶 | OUT_TEMP → `temp = 45 - ris` (linear map around 25 °C) |
| `attachI2C` | `void attachI2C(I2C *p, uint8_t sa)` 🔶 | dependency injection of the I2C channel; when NULL the class falls back to the raw global `I2CReceive/I2CSend` API — a deliberate dual-path design |
| `printAsseZ` | `void printAsseZ(int numS)` 🔶 | dumps the calibration buffer |

⚠ **`azzeraAssi` ALL_AXIS branch bug** (Giroscopio.cpp:61–64 and 86–88): samples are read into `buffer` but the accumulation uses `buffer3A[...]`, a different array that is never filled — the computed bias is garbage from the stack. The Z_AXIS branch (the one used by the rover) is correct.

```c
/// Giroscopio.cpp:61-64 — the accumulation reads buffer3A, never filled
I2CReceiveN(GYRO_ADDR, OUT_X_L | MUL_READ, 6, buffer);
x += (int16_t)((buffer3A[1]<< 8) + buffer3A[0]);
y += (int)((buffer3A[3]<< 8) + buffer3A[2]);
z += (int)((buffer3A[5]<< 8) + buffer3A[4]);
```

Integration maths (student summary): raw 16-bit reading → subtract static bias → multiply by `FS` (°/s per LSB) and per-axis correction `kz` → multiply by the integration step `tick` (seconds) → accumulate into `yawF`. This is **rectangular (Euler) integration**; the drift-compensation block every 10 s is the practical defence against the gyroscope's zero-rate bias. See [student-notes.md §2](student-notes.md#2-from-angular-rate-to-angle-numeric-integration).

### 5.2 Register map — `gyro_init.h`

Defines for the L3GD20: `GYRO_ADDR 0x6B`, `WHO_AM_I 0x0F`, `CTRL_REG1..5 0x20..0x24`, `OUT_TEMP 0x26`, `STATUS_REG 0x27`, `OUT_X_L..OUT_Z_H 0x28..0x2D`, FIFO registers, status bits (`ZYXDA`, `ZYXOR`...), full-scale selects (`FS_250/500/2000`), `MUL_READ 0x80` (auto-increment bit for multi-byte reads), axis masks `ALL_AXIS 0x7 / Z_AXIS 0x4`.

### 5.3 Legacy C API — `gyro_f.h / gyro_f.c` ⛔

`typedef struct _gyro {...}` plus `void misuraAngoli(gyro *G)` — the entire C implementation is **commented out** (gyro_f.c:22–103); only the struct and the `DPS_500/DPS_250` conversion macros remain live. `gyro_init.h` still prototypes the old `initGyro(gyro*, char) / setupAssi(gyro*, char) / azzeraAssi(gyro*)` trio ⚠ (prototypes point to code that no longer exists — harmless because never called, but a trap for revivers).

### 5.4 `Giroscopio.h` extras

`#define maxDimBuff 256`, `#define numSampleBias 128` (calibration sample counts) and a stray `uint8_t printFloat(double, uint8_t);` declaration.

---

## 6. Command & control subsystem

### 6.1 `class ControlloPID` — `pid.h:48–68 / pid.cpp` 🔶

The digital PID. Three conceptual instances (straight / rotate-right / rotate-left) were planned; `comando::numPid` indexes them.

| Member | Purpose |
|--------|---------|
| `setupPID(int type)` 🔶 | loops `setKpid(1.1, 2.1, 0.5)` three times, stores `tipo` |
| `setKpid(float, float, float)` 🔶 | ⚠ **self-assignment bug**: body is `kp = kp; kd = kd; ki = ki;` — the gains never reach the members (shadowed parameters). Only `I1` and `d` are actually reset |
| `integra(float tick)` 🔶 | rectangular PID step: `D = kd·(e[1]−e[0])/tick`, `P = kp·e[1]`, `I = 0.5·(I1 + ki·tick·(e[1]+e[0]))`, output saturated to ±100, then `e[0] = e[1]` |
| `Run(Giroscopio*, PWM_MOTORI*×2, distMis*)` 🔶 | per-type control law: `AVANZA` sets fixed duty 65 and calls `MotorGo()`; `RUOTA_DESTRA` integrates `valFin − G->yaw` error and drives the gyro; `RUOTA_SINISTRA` also flips motor directions and raises `rispondi = TRUE` within ±1° |

```c
/// pid.cpp:33-40 — the classic shadowed-parameter trap
void ControlloPID::setKpid(float kp, float kd, float ki){
        kp = kp;   /// assigns parameter to itself!
        kd = kd;
        ki = ki;
        I1 = 0.0;
        d = 0.0;
}
```

### 6.2 `class comando` — `pid.h:29–46 / pid.cpp` 🔶

Represents *the current command* and owns its execution loop `RUN()`:

```c
int comando::RUN(ControlloPID *p, syn_stat *s, PWM_MOTORI *PWM1,
                 PWM_MOTORI *PWM2, encQuad *ENC1, encQuad *ENC2, Giroscopio *G)
```

Behaviour (pid.cpp:219–365):

1. **Timeout** — `tick > TIMEOUT_CMD` (150 ticks = 1.5 s): deactivates the active PID, `MotorStop()` on both motors, `isRun = false`, marks `s->valid = NON_VALIDO`.
2. **Direction pre-set** — token `INDIETRO` → `direction = -1`; token `AVANTI` → `+1` on both motors.
3. **Per-`numPid` action**:
   - `AVANZA` (0): duty 65 on both motors, `MotorGo()` — "MODO SEMPLICE", the PID terms are commented out;
   - `RUOTA_DESTRA` (1): while `|G->yaw − valFin| > 2°` spins with duty 50 in opposite directions and sets `G->IsRotating = true`; on arrival stops, clears the flag and requests a gyro offset (`G->offsetRequest = 1`);
   - `RUOTA_SINISTRA` (2): mirror image;
   - `ARRESTA` (-1) / default: stop.
4. Members `azione`, `isRun`, `finished`, `token`, `avvia`, `valFin` mirror the parsed command; `setUptrasducers()` injects sensor pointers (unused in this release).

### 6.3 Parser — `parse.h / parse.cpp`

| Function | Signature | Status | Purpose |
|----------|-----------|--------|---------|
| `PARSE` | `void PARSE(syn_stat *STATO)` | 🟢 | the state machine actually called by `main`; recognises the 4-byte frame, raises `valid = VALIDO`. Note: it never calls `convertToToken`, so `STATO->token` stays stale |
| `parse` | `void parse(syn_stat*, comando*, syntaxStatus*)` | 🔶 | twin of `PARSE` that *does* call `convertToToken(STATO, cmdPtr)` — the "full" version, unreachable in this release |
| `convertToToken` | `void convertToToken(syn_stat*, comando*)` | 🔶 | maps the command letter to tokens and sets `numPid`, `valFin` (±90° for R/L), `avvia` for `P`; ⚠ **`case 'P':` falls through to `default:`** so the token immediately becomes `ERRORE` |
| `resetAutoma` | `void resetAutoma(syn_stat*)` | 🟢 | clears state, `valid = NON_VALIDO` |
| `rispondiComando` | `void rispondiComando(syn_stat*, glb*)` | 🔶 | builds the reply for `LETTURA_SENSORE` queries: checksum over 3 bytes XOR `CHECK_SUM`, terminator `'*'` |
| `sendReply` | `void sendReply(syn_stat*, uint8_t numChar)` | 🔶 | pushes `buff_reply[0..numChar-1]` plus `'*'` on UART1 |
| `inviaSensore` | `void inviaSensore(syn_stat*, glb*)` | 🔶 | sensor-ID dispatch for data queries (table in [architecture.md §4](architecture.md#4-the-parser-state-machine)); ⚠ case 7 answers with ID byte 8, cases 7/8 are duplicated ⚠ |

The `PARSE` automaton with exact per-state transitions is drawn in [architecture.md §4](architecture.md#4-the-parser-state-machine).

### 6.4 `syntaxStatus` class + `glb` struct — `parse.h`

`syntaxStatus` (parse.h:41–56) is field-for-field identical to `syn_stat` plus a `valore` field — the C++ rewrite that never replaced the struct (⚠ see [state-of-code.md](state-of-code.md#3-duplicate-and-parallel-implementations)). `glb` (parse.h:23–39) is the pointer bag aggregating all sensor/orchestrator objects for the query path; nothing fills it in this release.

### 6.5 Protocol constants — `init.h`

`DIM_READ_BUFF 64` (ring size), `CHECK_SUM 0xA9`, token values (`ERRORE 255 … MISURA_GRADI 10`), `TIMEOUT_CMD 150` (1.5 s at 10 ms ticks), `TRUE '1' / FALSE '0'` (⚠ char-typed booleans), `INT_STEP_10_MS 10`, and the legacy `dati` pointer struct.

---

## 7. Communication

### 7.1 UART plumbing — `uartp/uart.h / uart.c` 🟢

- `void ConfigureUART(uint32_t baud, int numUart)` — UART0: PA0/PA1, PIOSC clock, `UARTStdioConfig(0, baud, 16000000)`, RX interrupt enabled; UART1: PB0/PB1, RX+TX interrupts enabled. Both 115200 8N1.
- `void UARTSend(const uint8_t*, uint32_t)` — non-blocking string push on UART0.
- `uartstdio.c` (TivaWare, adapted): `PRINTF(...)` at line 1297 + the `g_ui32Base` / `g_ui32UARTBase[3]` output-selector globals. `ustdlib.c` provides the integer formatting kernels.

### 7.2 UART1 receive ISR — `interrupt.c:28–58` 🟢

```c
volatile uint8_t uart1buffer[], RX_PTR1 = 0, READ_PTR1 = 0;

void UART1IntHandler(void)
{
    uint32_t ui32Status = ROM_UARTIntStatus(UART1_BASE, true);
    ROM_UARTIntClear(UART1_BASE, ui32Status);
    while(ROM_UARTCharsAvail(UART1_BASE)){
        uart1buffer[RX_PTR1] = ROM_UARTCharGetNonBlocking(UART1_BASE);
        RX_PTR1++;
        RX_PTR1 &= DIM_READ_BUFF - 1;      // ring wrap, size 64
    }
}
```

Note: `uart1buffer` is declared **without a size** here and with `[16]` in the `extern` declarations of `main.cpp:63`, `tiva_timer.c:28`, `xbee.cpp:23` — but `DIM_READ_BUFF` is 64, so indices run to 63. In practice the linker allocates the array from this file's (sizeless) definition and the ring works, but the inconsistent `extern[16]` declarations are undefined behaviour waiting to bite ⚠ ([details](state-of-code.md#6-bugs-and-fragile-points-summary-table)).

### 7.3 `class xBee` — `xbee/xbee.h / xbee.cpp` 🟢 (probe) / 🔶 (rest)

Radio module on UART1; PB5 is its reset line (constructor drives it low, `test()` releases it).

| Member | Status | Purpose |
|--------|--------|---------|
| `test()` | 🟢 | releases reset, sends `"+++"`, waits up to 2 s scanning the ring buffer for `O`,`K`,`\r`; on success sends `ATCN\r` (exit command mode) and sets `status = TXRX`, `present = XBEE_PRESENT` |
| `sendChar(char)` | 🟢 | `UARTCharPut(UART1_BASE, c)` |
| `sendString(char*, int)` | 🔶 | ⚠ loop increments `s` but never `i` — the `i < n` bound never engages; termination relies solely on the NUL byte |
| `receive()` | ⛔ | declared in the header, **never defined** — linker error if ever called |
| `readMyAdd()` / `readReg()` | ⛔ | ATSH/ATSL address readback; blocked on `endMess0` which is never set by any ISR — would hang forever |
| `setBaud(uint32_t)` | 🔶 | stores the value only |
| free `uint32_t testXbee()` | ⛔ | xbee.cpp:29–61, earlier non-class duplicate of `test()` |

---

## 8. I2C — `I2C/tiva_i2c.h / tiva_i2c.cpp / i2cTiva.h`

Two coexisting layers over I2C0:

**Raw C API** (tiva_i2c.h, `extern "C"`): 🟢
- `void InitI2C0(void)` — GPIOB pins + I2C0 master config at 100 kbps (`I2CMasterInitExpClk(I2C0_BASE, SysCtlClockGet(), false)`).
- `uint32_t I2CReceive(uint32_t slave_addr, uint8_t reg)` — single register read: write reg address, repeated start, read one byte.
- `void I2CReceiveN(uint8_t slave_addr, uint8_t reg, uint8_t numElem, uint8_t buff[])` — burst read with `MUL_READ` auto-increment (used for the gyro 2/6-byte bursts).
- `void I2CSend(uint8_t slave_addr, uint8_t num_of_args, ...)` — variadic register write (reg, val1, val2...).
- `void I2CSend_String(uint32_t, char[])` — string write.

**`class I2C`** (i2cTiva.h, implemented in tiva_i2c.cpp:348+): 🔶 the OO rewrite — `InitI2C(uint32_t base)`, `I2CSetSlave_Add(uint8_t)`, `I2CGetN`, `I2CGet`, `I2CPut`, `I2CPut_String`; state flag `IMPOSTATA/NON_IMPOSTATA`. Injected into `Giroscopio`/`TEMPER` via their `attachI2C()` methods.

All transactions busy-wait on the master-busy/status registers — the standard blocking-master pattern; no I2C interrupts are used.

---

## 9. Motor drive — `pwm/pwm.h / pwm.cpp / motpwm.h`

### 9.1 `class PWM_MOTORI` 🔶

DC motors through the L298-style driver: PWM on **PE5 (M0PWM5, generator 2)** and **PB4 (M0PWM2, generator 1)** of module PWM0; direction via digital pins **PE4 / PA5**; PWM clock = system/32, period `NCont = SysCtlClockGet()/256000` ≈ 312 → **8 kHz PWM** with ~0.32% granularity.

| Member | Purpose |
|--------|---------|
| `Init()` | per-motor branch on `numMot` (static counter identifies motor 1 vs 2): enable GPIO/PWM0, configure pin mux + generator + period, outputs on, `MotorStop()` |
| `MotorGo()` | sets the `PWM_O_INVERT` register bit according to `direction` (+1/-1), then `PWMPulseWidthSet(PWM0_BASE, numPin, NCont·delta·k/100)` with per-motor trim `k` = 1.076 (motor 1) / 1.0 (motor 2) — the hardware way of doing what `PWMOutputInvert()` would redo |
| `MotorStop()` | `delta = 0`, direction forward, `MotorGo()` |
| `pwmPower()` | untrimmed pulse-width update |

### 9.2 `class PWM_SERVI` 🔶

Servos on **PD0 (M1PWM0)** and **PD1 (M1PWM1)** of module PWM1, 50 Hz (period from `SysCtlClockGet()/1600` with divider 32 → 20 ms).

- `MotorGo(int16_t gradi)` → `convertDeg2Pwm()`: `rot = gradi/36.0 + 7.50` (ms of pulse), pulse width set.
- `scarico()` 🔶 — the rescue-pack release move: −30°, busy wait, +80°.
- `MotorStop()` — pulse width 0.

### 9.3 Legacy `pwm` struct + C functions ⛔

`pwm.h` still defines `typedef struct _pwm {NCont, delta_1, delta_2, dir_1, dir_2}` and prototypes `pwmMotInit / pwm_power / pwm_dir / pwmServoInit / pwmPowerA6 / pwmPowerA7` — none of these functions is implemented in `pwm.cpp` anymore (⚠ dangling prototypes).

---

## 10. Analog & optical sensors — `sens_col_ir/`

### 10.1 `class TEMPER` 🔶
IR thermopile (Melexis-style) on I2C. `readTemp()` reads 3 bytes from `TEMP_REG|0x20` → `Temp = 0.02·raw − 273.15`, `Tint` integer copy; `taraturaTemp()` also reads the case-temperature register (0x06); `attachI2C()` injects the bus. Guarded against a NULL `i2cPtr` with a console warning.

### 10.2 `class COLORE` + `class TILE` 🔶
Light sensor = photodiode whose pulses are counted:

```c
/// senscol.cpp:48-65 — readCol(): one measurement window
HWREG(...PA2...) = GPIO_PIN_2;          // power the LED
TimerEnable(TIMER4_BASE, TIMER_A);      // open the 10 ms window
GPIOIntEnable(GPIO_PORTA_BASE, GPIO_INT_PIN_3);
while(procCom4 == 0);                   // Timer4ISR closes the window
procCom4 = 0;
tmp = contLightPwm;                     // pulse count = luminance
contLightPwm = 0;
return tmp;
```

`COLORE::Run()` classifies `luminanza < bianco/4` → dark tile (`TILE.isDark`). `initLightSens1()` sets PA2 as 8 mA output, PA3 rising-edge interrupt, priority 0. `initTimer4(ms)` configures TIMER4A periodic. `normalizzaColori()` (RGB norm-2) is the leftover of an older RGB sensor.

### 10.3 Legacy structs & macros 🔶
`sens.h`: `colore`, `survivor`, `temperatura` structs; `COLOR_ADDR 0x29` (TCS34725-era), thresholds `SOGLIAFERITO 600`, message constants `IS_WALL/IS_SURVIOR/GET_...` — Rescue-field vocabulary from earlier generations of the codebase.

### 10.4 `class distMis` 🔶
Holds ADC results `dI[8]` for 5–6 Sharp-style IR distance sensors and converts raw counts to mm:

- `rawTomm()` — per-sensor piecewise model: linear segments of `1/d = m·dI + q` for sensors 3/6, quadratics `f = a·dI² + b·dI + c` for 2/5 and a 3-piece parabola for 4; distance = `10/f` mm. Clamps dI into [400, 3745] for the linear ones (≈ 3.5–40 cm).
- `rawTomm1()` — compact power-law fit `d = k·dI^m` (regression done 22 April 2016, credited "Bricca e compagni" in the comment), clamps dI to [650, 3600].

⚠ Never called from `main()`; ⚠ `dI[6..7]` never initialised.

---

## 11. Power — `power.h / power.cpp` ⛔

`class power { int battLevel; int safeLevel; }` — constructors set −1. No measurement code, no instances. Intended to watch the battery through the 6th distance-sensor ADC channel (see comment in `distMis.h:24-25`).

---

## 12. Timers, interrupts and startup

### 12.1 `tiva_timer.c`

| Symbol | Status | Purpose |
|--------|--------|---------|
| `void Timer0ISR(void)` | 🟢 vector: Timer 0A | `procCom = 1; tick10++; tick100++;` — the entire scheduler |
| `void Timer1ISR(void)` | ⛔ vector: default | increments a local counter; `initTimer1` never called in this release |
| `void initTimer0(int interval)` | 🟢 | FPU lazy stacking; `tick = SysCtlClockGet()/1000·interval`; TIMER0A periodic; interrupt enable; timer enable |
| `void initTimer1(int interval)` | ⛔ | same for TIMER1A (not used) |

```c
/// tiva_timer.c:32-44 — the 3-line scheduler
void Timer0ISR(void){
    ROM_TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    procCom = 1;
    tick10++;
    tick100++;
}
```

### 12.2 `interrupt.c` — other ISRs

| Symbol | Vector slot | Status | Behaviour |
|--------|-------------|--------|-----------|
| `UART1IntHandler` | UART1 | 🟢 | ring-buffer fill (see §7.2) |
| `IntGpioA` | GPIO Port A | 🔶 | clears PA3 int, `contLightPwm++` (light sensor counting) |
| `IntGPIOd` | GPIO Port D | ⛔ | clears PD1 int, `contLightPwm++` — older pin assignment; PD1 is repurposed by `PWM_SERVI` now |
| `Timer4ISR` | Timer 4A | 🔶 | closes the light-sampling window: clears int, disables PA3 interrupts + TIMER4, `procCom4 = 1`, PA2 off |

### 12.3 `tm4c123gh6pm_startup_ccs.c` 🟢
CCS startup file: 154-entry vector table (initial stack pointer + 153 handlers). Wired to project handlers: `IntGpioA` (GPIOA), `IntGPIOd` (GPIOD), `IntGPIOf` (GPIOF), `UART1IntHandler` (UART1), `Timer0ISR` (Timer 0A), `Timer4ISR` (Timer 4A). Everything else → `IntDefaultHandler` (infinite loop). Also contains the minimal `ResetISR` copying data/bss sections.

### 12.4 `tm4c123gh6pm.cmd` 🟢
Linker script: 256 KB FLASH at 0x0000_0000, 32 KB SRAM at 0x2000_0000, stack 512 bytes.

---

## 13. `gen_def.h` — shared definitions

`OFF 4 / ON 2 / OK 0 / NOT_PRESENT 3`, `DI()/EI()` macros, `initTimer0/1` prototypes (C linkage), `UART0/UART1` numeric aliases. Includes `gyro_f.h`, which is why nearly every file transitively sees the legacy `gyro` struct ⚠.

---

## 14. Alphabetical index of all functions

| Function | File | Status |
|----------|------|--------|
| `attesa` | gyro_init.c:236 | 🔶 |
| `azzeraAssi` (gyro*) | gyro_init.c (commented) | ⛔ |
| `blinkBlueLed` / `blinkRedLed` | gyro_init.c:25/38 | 🟢 / 🔶 |
| `comando::RUN` | pid.cpp:219 | 🔶 |
| `comando::setUptrasducers` | pid.cpp:211 | 🔶 |
| `ConfigureUART` | uartp/uart.c:35 | 🟢 |
| `ControlloPID::integra` | pid.cpp:44 | 🔶 |
| `ControlloPID::Run` | pid.cpp:71 | 🔶 |
| `ControlloPID::setKpid` | pid.cpp:33 | 🔶 ⚠ |
| `ControlloPID::setupPID` | pid.cpp:21 | 🔶 |
| `convertToToken` | parse.cpp:157 | 🔶 ⚠ |
| `COLORE::COLORE/Run/...` | senscol.cpp:260/264 | 🔶 |
| `distMis::rawTomm/rawTomm1` | distMis.cpp:36/135 | 🔶 |
| `encQuad::encQuad/~encQuad` | encQuad.cpp:43/56 | 🟢 |
| `encQuad::intIDXEnable` | encQuad.cpp:242 | 🔶 |
| `encQuad::qeiInit` | encQuad.cpp:60 | 🟢 |
| `encQuad::readDir/readPos/readVel/update` | encQuad.cpp:223/209/217/231 | 🟢/🔶 |
| `I2C::InitI2C/I2CGet/I2CGetN/I2CPut/I2CPut_String` | I2C/tiva_i2c.cpp:348+ | 🔶 |
| `I2CSend / I2CSend_String` | I2C/tiva_i2c.cpp:203/31 | 🟢 (gyro path) |
| `I2CReceive / I2CReceiveN` | I2C/tiva_i2c.cpp:107/286 | 🟢 |
| `InitI2C0` | I2C/tiva_i2c.cpp:164 | 🔶 (not called in main) |
| `initGyro / setupAssi` (C, gyro*) | gyro_init.c (commented) | ⛔ |
| `initLightSens1` | senscol.cpp:116 | 🔶 |
| `initTimer0` / `initTimer1` | tiva_timer.c:76/129 | 🟢 / ⛔ |
| `initTimer4` | senscol.cpp:172 | 🔶 |
| `IntEnc0` | encQuad.cpp:249 | ⛔ |
| `IntGPIOd` | interrupt.c:65 | ⛔ |
| `IntGpioA` | interrupt.c:84 | 🔶 |
| `IntGPIOf` | encQuad.cpp:257 | 🟢 |
| `inviaSensore` | parse.cpp:418 | 🔶 |
| `main` | main.cpp:70 | 🟢 |
| `minQuad` | gyro_init.c:279 | 🟢 (via gyro calib) |
| `normalizzaColori` | senscol.cpp:25 | 🔶 |
| `PARSE` | parse.cpp:36 | 🟢 |
| `parse` | parse.cpp:99 | 🔶 |
| `PARSE/parse` helpers `resetAutoma` | parse.cpp:25 | 🟢 |
| `printFloat` | main.cpp:355 | 🟢 |
| `PRINTF` | uartp/uartstdio.c:1297 | 🟢 |
| `PRINT_WELCOME` | gyro_init.c:266 | 🟢 |
| `PWM_MOTORI::Init/MotorGo/MotorStop/pwmPower` | pwm/pwm.cpp:39/171/153/163 | 🔶 |
| `PWM_SERVI::Init/MotorGo/MotorStop/scarico/convertDeg2Pwm` | pwm/pwm.cpp:215/305/324/350/339 | 🔶 |
| `qei_data/qei_init/qei_reset/qei_test` | qei.cpp:402/249/414/375 | ⛔ |
| `QEI0IntHandler / QEI1IntHandler` | qei.cpp:31/57 | ⛔ ⚠ |
| `readCol` | senscol.cpp:48 | 🔶 |
| `readColourSens/readTemp/readTempSens/taraturaTemp` (C) | sens.h prototypes | ⛔ (unimplemented) |
| `resetAutoma` | parse.cpp:25 | 🟢 |
| `rispondiComando` | parse.cpp:359 | 🔶 |
| `rispostaRotazione` (proto in pid.cpp:18) | — | ⛔ declared, never defined |
| `sendReply` | parse.cpp:406 | 🔶 |
| `setupAssi` (Giroscopio) | Giroscopio.cpp:205 | 🔶 |
| `setupMCU` | gyro_init.c:157 | 🟢 |
| `setupUART` | gyro_init.c:251 | 🟢 |
| `TEMPER::readTemp/taraturaTemp/attachI2C` | senscol.cpp:202/222/251 | 🔶 |
| `testXbee` (free) | xbee/xbee.cpp:29 | ⛔ |
| `Timer0ISR` / `Timer1ISR` | tiva_timer.c:32/49 | 🟢 / ⛔ |
| `Timer4ISR` | interrupt.c:96 | 🔶 |
| `UART1IntHandler` | interrupt.c:30 | 🟢 |
| `UARTSend` | uartp/uart.c:108 | 🔶 |
| `UnlockPD7` | qei.cpp:94 | ⛔ |
| `UnlockPD7_01` | encQuad.cpp:32 | 🟢 |
| `xBee::test/sendChar/sendString/...` | xbee/xbee.cpp:83/144/149 | 🟢/🔶/⛔ |

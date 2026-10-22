---
layout: default
title: State of the Code
---

# State of the Code — review after 10 years

> A candid audit of the repository as it stands at commit `master` (May 2016). Nothing here is criticism for its own sake: the code was written fast, by a student team, under competition pressure, and it *worked on the bench*. This page is the map of what to fix first if anyone revives it. Cross-references: [api-reference.md](api-reference.md) documents each symbol; [class-map.md](class-map.md) shows the object relations.

Summary counts: **9 real bugs or fragile points**, **7 dead modules/files**, **5 duplicate/parallel implementations**, **6 open TODOs left in the source**.

---

## 1. Real bugs

### 1.1 `case 'P':` falls through to `default:` — parse.cpp:202-211  ⚠ HIGH

```c
case 'P':
        //rilascio rescue pack
        STATO->token = RILASCIO_PACK;
        cmdPtr->azione = true;
        cmdPtr->avvia = true;
default:                                  // <-- missing break!
        /// se nessun comando e' giusto produce un errore.
        STATO->token = ERRORE;
        cmdPtr->numPid = -1;
break;
```

Every received `P` (rescue-pack release) is immediately overwritten with `token = ERRORE`. The comment in `main.cpp:265-269` ("QUESTO ACCADE IN convertToToken") shows the author expected `RILASCIO_PACK` to survive. **Fix: add `break;` before `default:`.**

### 1.2 `ControlloPID::setKpid` never assigns the gains — pid.cpp:33-40  ⚠ HIGH

```c
void ControlloPID::setKpid(float kp, float kd, float ki){
        kp = kp;   // parameter shadows member, assigned to itself
        kd = kd;
        ki = ki;
```

All three gains stay 0.0 forever, so even if the PID path were re-connected, `integra()` would compute a zero output. Classic C++ shadowing trap. **Fix: `this->kp = kp;` etc. (or rename parameters).**

### 1.3 Blocking console read at the top of the main loop — main.cpp:181  ⚠ HIGH

```c
while(1){
    uint8_t valore_carattere = UARTCharGet(UART0_BASE);   // blocks!
```

`UARTCharGet` waits for a character on the *debug* console. One loop pass == one typed character, so the 10/100/1000 ms cadences and the command parsing only advance while somebody types. Fine as a bench test, fatal in a match. **Fix: use `UARTCharGetNonBlocking` and ignore `valore_carattere` (it is already unused).**

### 1.4 Undeclared globals under `#ifndef _DEBUG_` — main.cpp:294-313  ⚠ HIGH

`MISURE.d_mm[]` (a `distMis` object) and `Rot` (a `Giroscopio` object) are printed here but declared nowhere in the file or its includes. Consequences:

- Building **with** `_DEBUG_` defined: compiles (blocks removed) — evidently the configuration used in 2016.
- Building **without**: hard compile errors.

This also means the telemetry of distances/yaw in the release is the *debug* branch, i.e. mostly empty prints. **Fix: instantiate `distMis MISURE; Giroscopio Rot;` at file scope (or delete the block).**

### 1.5 `azzeraAssi` accumulates the wrong buffer — Giroscopio.cpp:61-64, 86-88  ⚠ MEDIUM

The ALL_AXIS bias calibration reads 6 bytes into `buffer` but sums `buffer3A[0..5]`, an unrelated, never-written array. Result: garbage X/Y/Z biases whenever the gyro is configured for all axes (drone mode). The Z_AXIS branch (rover mode) is correct. **Fix: use `buffer`.**

### 1.6 Wrong mask compared in QEI1 handler — qei.cpp:48, 73  ⚠ MEDIUM (dead code)

```c
if (  (status1 & QEI_INTDIR) == QEI_INTERROR)   // INTDIR tested twice,
        QEIIntClear(QEI0_BASE, QEI_INTERROR);    // INTERROR never tested
```

Copy-paste slip in both handlers. Harmless today (module dead) but a booby trap for anyone re-enabling the C API.

### 1.7 Uninitialised global pointer `qei *p` — qei.cpp:30  ⚠ MEDIUM (dead code)

Both QEI handlers dereference this NULL-pointer-to-be. If the module is ever re-wired into the vector table, first interrupt = hard fault.

### 1.8 Inconsistent `extern` size for `uart1buffer`  ⚠ MEDIUM

- Definition: `volatile uint8_t uart1buffer[];` (interrupt.c:28 — sizeless)
- Externs: `extern volatile uint8_t uart1buffer[16], ...` in main.cpp:63, tiva_timer.c:28, xbee.cpp:23
- Actual usage: ring of `DIM_READ_BUFF` = **64** bytes.

Type-safe C would refuse this mix; by luck the linker allocates enough and all readers use `DIM_READ_BUFF`. But any new file that trusts the `[16]` extern could allocate a 16-byte local copy and get silent corruption. **Fix: declare `volatile uint8_t uart1buffer[DIM_READ_BUFF];` in interrupt.c and extern it identically everywhere.**

### 1.9 `xBee::sendString` never increments its counter — xbee.cpp:149-156  ⚠ LOW

```c
while((*s) != 0 && i < n){
        sendChar(*s);
        s++;            // i is never incremented
}
```

The length cap is ineffective; termination depends on the NUL byte only. Benign today (all callers pass NUL-terminated literals) but the signature promises more than it delivers.

---

## 2. Dead code inventory

Everything below is compiled (or present) but unreachable / unused in this release:

| Item | Where | Why it is dead | Verdict |
|------|-------|----------------|---------|
| `qei.cpp` entire module (C API + 2 handlers + `UnlockPD7`) | qei.cpp | superseded by `encQuad` class; handlers not in vector table | delete or archive |
| `gyro_f.c` / `gyro_f.h` | gyro_f.c | implementation 100% commented out; only the `gyro` struct + DPS macros still used by headers | keep struct if `init.h`/`gen_def.h` chains need it, else delete |
| `power.h/.cpp` | root | never instantiated | delete (or implement battery monitoring as planned) |
| `xBee::receive()` | xbee.h:31 | declared, never defined | remove declaration |
| `testXbee()` free function | xbee.cpp:29 | superseded by `xBee::test()` | remove |
| `IntEnc0()` | encQuad.cpp:249 | not in vector table | remove |
| `IntGPIOd()` (PD1 version) | interrupt.c:65 | PD1 repurposed for servo PWM; `IntGpioA` is the live twin | remove |
| `Timer1ISR` + `initTimer1` | tiva_timer.c | timer 1 unused in this release | keep only if scheduler will grow |
| `rispostaRotazione` prototype | pid.cpp:18 | declared, never defined, never called | remove |
| `dati` struct + `dati_a_struttura`/`datiRaccolti` comments | init.h:67-75, main.cpp:97-101 | old pointer-bag replaced by `glb` | remove after `glb` is filled |
| C prototypes in `gyro_init.h` (`initGyro(gyro*,...)` etc.) | gyro_init.h:112-121 | point to commented-out code | remove |
| `pwm.h` C prototypes (`pwmMotInit`, `pwm_power`, `pwmPowerA6/A7`...) | pwm.h:40-47 | functions no longer implemented | remove |
| `readColourSens/readTemp/readTempSens/taraturaTemp` prototypes (C) | sens.h:68-76 | unimplemented | remove |
| `qei_test` call site | main.cpp:159 (commented) | — | clean-up |

The `Release/` folder also references three directories that **no longer exist** in the repository (`allineamento/`, `adc/`, `accel/` — see `Release/makefile`, objects listed but absent). Those modules were removed in a later cleanup that never updated the makefile — the project as published cannot be rebuilt as-is without CCS project surgery.

---

## 3. Duplicate and parallel implementations

The strongest "10-years-later" smell: every subsystem exists twice (C-era and C++-era).

| Concept | C version | C++ version | Winner in `main()` |
|---------|-----------|-------------|--------------------|
| Encoder reading | `qei` struct + `qei_*()` functions | `encQuad` class | C++ |
| Gyro reading | `gyro` struct + `misuraAngoli(gyro*)` | `Giroscopio` class | *neither* (both unused; struct still transitively included) |
| Parser state | `syn_stat` struct + `PARSE()` | `syntaxStatus` class + `parse()` | struct + `PARSE()` |
| PWM motors | `pwm` struct + `pwm_power()` (prototypes only) | `PWM_MOTORI` / `PWM_SERVI` classes | C++ (but dormant) |
| I2C access | `I2CSend/I2CReceive*` raw functions | `I2C` class | both! (`Giroscopio` supports either path via `attachI2C`) |
| XBee test | `testXbee()` free function | `xBee::test()` method | method |
| Temperature | `temperatura` struct + C prototypes | `TEMPER` class | class (dormant) |
| Color | `colore` struct + `readColourSens` | `COLORE` + `TILE` classes | class (dormant) |

**Recommendation**: pick one (C++ classes), delete the other, and — most valuable — make the parser a member function of a `Parser` class holding `syn_stat` privately.

---

## 4. The disconnected command path

The most important architectural finding: **the wiring between "valid command received" and "motors move" was cut in this release**.

The intended chain (all pieces exist, all compiled):

```
UART1IntHandler → uart1buffer → PARSE() → convertToToken() → comando CMD1
      → CMD1.RUN(CTRL, &synSTATO, &PWM1, &PWM2, &ENC0, &ENC1, &G)
            → ControlloPID.Run() / PWM_MOTORI::MotorGo() → motors
```

What actually runs:

```
UART1IntHandler → uart1buffer → PARSE() → synSTATO.valid = VALIDO
      → main() dumps ENC0.pos / posV[] to the console    ← full stop
```

Specifically:

1. `PARSE()` never calls `convertToToken` (only its twin `parse()` does — never invoked).
2. `CMD1.RUN(...)` is never called; `CMD1.avvia/isRun` are touched only inside `#ifdef _DEBUG_`.
3. No `PWM_MOTORI` objects are constructed in `main()` (the class even has a nice static auto-numbering scheme waiting to be used).
4. No `Giroscopio` object exists despite `Rot.yaw` being printed (see §1.4).

This is consistent with the repo name and README: the release is the **encoder test bench**, ancestor of a fuller rover release that presumably lived on another branch/machine. The `main.cpp` header comment ("VERSIONE COMPLETA DI TEST") says exactly that.

---

## 5. TODOs left in the source

| Location | TODO text (translated) |
|----------|------------------------|
| encQuad.cpp:62 | "calibrate the two values: period and full-scale" |
| encQuad.cpp:194 | "if you want QEI interrupts, uncomment the two lines below" |
| main.cpp:245 | "is the PID computed every 10 ms or every 20 ms? Which is better?" |
| Giroscopio.cpp:293 | "if not rotating it could perform an axes reset" |
| parse.cpp:420 | "added pointer to color struct, should temperature's be added too?" |
| parse.cpp:467 | "widen the d_mm vector by 1 (6th sensor)" |
| senscol.cpp:148 | "check whether a pull-up is needed" |
| pid.cpp:130 | "now the command must be sent to the PWM" |

---

## 6. Bugs and fragile points (summary table)

| # | Where | Issue | Severity | One-line fix |
|---|-------|-------|----------|--------------|
| 1 | parse.cpp:207 | missing `break` after `case 'P'` | high | add `break;` |
| 2 | pid.cpp:34-36 | `setKpid` self-assignment | high | `this->kp = kp;` |
| 3 | main.cpp:181 | blocking `UARTCharGet(UART0)` in main loop | high | `UARTCharGetNonBlocking` |
| 4 | main.cpp:294-313 | `MISURE`, `Rot` undeclared (non-debug build) | high | declare objects or delete block |
| 5 | Giroscopio.cpp:61-64 | wrong buffer in ALL_AXIS bias | medium | use `buffer` |
| 6 | qei.cpp:48,73 | wrong interrupt-mask comparison | medium (dead) | `status1 & QEI_INTERROR` |
| 7 | qei.cpp:30 | global `qei *p` never initialised | medium (dead) | delete with module |
| 8 | interrupt.c:28 vs externs | `uart1buffer` sizeless def vs `[16]` externs | medium | consistent `[DIM_READ_BUFF]` |
| 9 | xbee.cpp:152 | `sendString` counter never incremented | low | `i++` |
| 10 | encQuad.cpp:259-263 | index wrap maps upper half to small positive (never negative) | low | subtract `fscala` when `pos > fscala/2` |
| 11 | distMis.cpp:19-25 | `run`/`kf` re-set inside loop; `dI[6..7]` uninitialised | low | move out of loop |
| 12 | init.h:42-43 | `TRUE`/`FALSE` are chars `'1'/'0'` compared against `bool` | low | use stdbool |
| 13 | Giroscopio.h:76 | `printFloat` declared in wrong header | cosmetic | move to main/uart header |
| 14 | Release/makefile | references removed dirs (`adc/`, `accel/`, `allineamento/`) | build | regenerate CCS project |

Also worth noting: `IntGPIOf` computes "negative" positions as `pos −= fscala/2` (encQuad.cpp:260-263), which maps the upper counter half into `0..fscala/2` rather than `−fscala/2..0` — the net effect is a sawtooth discontinuity instead of signed position. For "count index pulses" purposes it is harmless, but it surprised the author too (comment says *posizione negativa*).

---

## 7. Suggested re-activation plan

If the goal is to bring the rover back to life on top of this code, the order that minimises pain:

1. **Make it compile warning-free** with `_DEBUG_` (fix §6 items 4, 8, 11, 14 first — pure plumbing).
2. **Fix the three functional bugs**: `break` after `case 'P'`, `setKpid`, blocking console read.
3. **Delete the C-era twins** (qei.cpp, gyro_f.c, free `testXbee`, dead ISRs) — the class-map becomes half the size.
4. **Re-connect the command path**: instantiate `PWM_MOTORI PWM1, PWM2;` and `Giroscopio G;`, switch the main loop from `PARSE` to `parse` (the full twin), then call `CMD1.RUN(&CTRL[0], &synSTATO, &PWM1, &PWM2, &ENC0, &ENC1, &G)` inside the 10 ms block — the author left that skeleton in the commented `if (contatore == 1){...}` block of `main.cpp:322-344`.
5. **Re-enable the gyro telemetry**: `Rot` is the missing object; call `G.initGyro(Z_AXIS)`, `G.primoAzzeramento()` at boot and `G.misuraAngoli()` every 10 ms — exactly what the commented-out PID block assumed.
6. **Replace the open-loop duty 65 in `AVANZA`** with the real PID using `distMis` velocity (the commented code in `ControlloPID::Run` shows the intended error: `valFin − DISTANZA->vel`).
7. Only then re-enable the light sensor / thermometer / servo pack release.

A realistic modernisation wish-list (optional): move the parser into a class, replace `delay loops` with the scheduler ticks, add a watchdog, and swap the blocking `readCol()` window for a state machine — but none of these are needed to reproduce 2016 behaviour.

---

*For the concepts behind the code (QEI, PID, gyro integration, ring buffers) see [student-notes.md](student-notes.md).*

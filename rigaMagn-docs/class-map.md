# Class Map — classes, structs and their relations

> This page is the **object map** of rigaMagn. Two views are given: (a) the C++ class hierarchy as UML diagrams, (b) the legacy C structs that coexist with them, and (c) the concrete objects (global variables) and their wiring. Function-by-function detail lives in [api-reference.md](api-reference.md).

---

## 1. The C++ classes — motion & control subsystem

```mermaid
classDiagram
    direction TB

    class encQuad {
        +int vel
        +int dir
        +int pos
        +int vel_period
        +int fscala
        +int zero_pos
        +uint32_t address
        +int dist_mm
        +int posV[64]
        +int indice
        +int contIDX
        +qeiInit() void
        +readPos() void
        +readVel() void
        +readDir() void
        +setAddr(add) void
        +update() void
        +intIDXEnable() void
    }

    class Giroscopio {
        +char IsPresent
        +char IsOn
        +int16_t x0 y0 z0
        +char asseOn
        +int16_t yaw roll pitch
        +float yawF yawF0
        +float rollF pitchF
        +float yawP rollP pitchP
        +int16_t gradiSec
        +float FS
        +float tick
        +float kz ky kx
        +int temp
        +char IsRotating
        +char offsetRequest
        +float media
        +float m
        +float q
        +initGyro(assi) void
        +setupAssi(stato) void
        +misuraAngoli() void
        +azzeraAssi() void
        +primoAzzeramento() void
        +getTemp() int
        +attachI2C(p sa) void
    }

    class ControlloPID {
        +float kp
        +float kd
        +float ki
        +float e[2]
        +float I1
        +float d
        +float uscita
        +uint32_t tipo
        +int valFin
        +bool attivo
        +uint8_t rispondi
        +setupPID(type) void
        +setKpid(kp kd ki) void
        +integra(tick) void
        +Run(G P1 P2 D) int
    }

    class comando {
        +bool azione
        +bool isRun
        +bool finished
        +int numPid
        +int token
        +int avvia
        +uint32_t tick
        +int valFin
        +RUN(p s P1 P2 E1 E2 G) int
        +setUptrasducers(G p d) void
    }

    class PWM_MOTORI {
        +static int numPwm
        +int NCont
        +int delta
        +int direction
        +uint32_t numPin
        +uint32_t numMot
        +Init() void
        +MotorGo() void
        +MotorStop() void
        +pwmPower() void
        +setDir(dir) void$ 
    }

    class PWM_SERVI {
        +static int numServi
        +int NCont
        +int delta
        +uint32_t numS
        +uint32_t numPin
        +Init() void
        +MotorGo(gradi) void
        +MotorStop() void
        +scarico() void
        +convertDeg2Pwm(gradi) uint32_t
    }

    class distMis {
        +float misSens[6]
        +float kf
        +int d_mm[6]
        +uint32_t dI[8]
        +bool run
        +int vel
        +rawTomm() void
        +rawTomm1() void
    }

    class I2C {
        +uint32_t BASE_ADDR
        +uint8_t SLAVE_ADD
        +uint16_t stato
        +InitI2C(base) void
        +I2CGetN(reg num buff) uint32_t
        +I2CGet(reg) uint32_t
        +I2CPut(num ...) void
        +I2CPut_String(array) void
        +I2CSetSlave_Add(sa) void
    }

    comando --> ControlloPID : "RUN() selects by numPid"
    comando --> PWM_MOTORI : "drives PWM1 / PWM2"
    comando --> encQuad : "receives ENC1 ENC2"
    comando --> Giroscopio : "gPtr / reads yaw"
    ControlloPID --> Giroscopio : "Run() reads yaw"
    ControlloPID --> PWM_MOTORI : "Run() writes delta"
    ControlloPID --> distMis : "Run() reads vel"
    Giroscopio --> I2C : "i2cPtr (optional)"
    distMis ..> PWM_MOTORI : "not wired (legacy)"
```

**How to read this**: `comando` is the *behaviour* object (what to do), `ControlloPID` is the *regulator* (how to do it), `encQuad`/`Giroscopio`/`distMis` are *sensors*, `PWM_MOTORI` is the *actuator*. The dashed link marks a dependency that exists in the original design but is not wired in this release.

---

## 2. The C++ classes — communication & I/O subsystem

```mermaid
classDiagram
    direction TB

    class xBee {
        +uint16_t present
        +uint16_t NumUart
        +uint32_t m_Baud
        -uint16_t status
        -uint8_t m_ADD_H[4]
        -uint8_t m_ADD_L[4]
        -uint8_t m_DATA[32]
        +test() void
        +sendChar(c) void
        +sendString(s n) void
        +receive() char
        +setBaud(b) void
        -readMyAdd() void
        -readReg(reg) void
        -upRptr(p) uchar
    }

    class TEMPER {
        +float Temp
        +int tempRaw
        +float T_tar
        +int Tcase
        +int Tint
        +int ok
        +uint16_t isSurvivor
        +I2C* i2cPtr
        +readTemp() float
        +getTemp() float
        +taraturaTemp() void
        +attachI2C(p sa) void
    }

    class COLORE {
        +int rosso
        +int verde
        +int blu
        +int luminanza
        +int bianco
        +TILE piastra
        +Init() void
        +read() int
        +get() int
        +getWhite() int
        +set(v) void
        +WhiteBalance() void
        +Run() void
    }

    class TILE {
        +uint32_t isDark
    }

    class syntaxStatus {
        +int valore
        +int ST
        +uint8_t cmd[4]
        +uint8_t l_cmd
        +uint8_t check
        +uint8_t token
        +uint8_t tick
        +uint16_t valid
        +uint8_t buff_reply[8]
        +uint8_t dato_valido
        +uint8_t suspend_reply
    }

    class power {
        +int battLevel
        +int safeLevel
    }

    xBee ..> UART1_HW : "UARTCharPut / Get"
    TEMPER --> I2C : "i2cPtr"
    COLORE ..> TIMER4_HW : "initTimer4(10) window"
    COLORE ..> GPIOA_HW : "PA2 power / PA3 pulses"
    COLORE *-- TILE : "piastra"

    class UART1_HW {
        <<peripheral>>
    }
    class TIMER4_HW {
        <<peripheral>>
    }
    class GPIOA_HW {
        <<peripheral>>
    }
    class I2C {
        +InitI2C(base) void
        +I2CGetN(reg num buff) uint32_t
        +I2CGet(reg) uint32_t
        +I2CPut(num ...) void
    }
```

`power` and `syntaxStatus` are **orphans** in this release: `power` is never instantiated, and `syntaxStatus` duplicates the C struct `syn_stat` (which is the one actually used by `main`). See [state-of-code.md](state-of-code.md#3-duplicate-and-parallel-implementations).

---

## 3. The legacy C structs

The project evolved from a pure-C codebase; the old structs still share the header files with the classes that superseded them.

```mermaid
classDiagram
    direction LR

    class syn_stat {
        <<struct - init.h>>
        +int ST
        +uint8_t cmd[4]
        +uint8_t l_cmd
        +uint8_t check
        +uint8_t token
        +uint8_t tick
        +uint16_t valid
        +uint8_t buff_reply[8]
        +uint8_t dato_valido
        +uint8_t suspend_reply
    }
    class glb {
        <<struct - parse.h>>
        TEMPER* temperat
        temperatura* tempPtr
        colore* colPtr
        COLORE* colorClass
        qei* qeiPtr
        survivor* surPtr
        distMis* DSTptr
        Giroscopio* gyro
        encQuad* encoder
    }
    class dati {
        <<struct - init.h>>
        gyro* gPtr
        temperatura* tempPtr
        colore* colPtr
        survivor* surPtr
    }
    class gyro {
        <<struct - gyro_f.h>>
        char IsPresent
        char IsOn
        int16_t x0 y0 z0
        char asseOn
        int16_t yaw roll pitch
        float yawF rollF pitchF
        float yawP rollP pitchP
        int16_t gradiSec
        float FS
        float tick
        float kz ky kx
    }
    class qei {
        <<struct - qei.h>>
        uint32_t pos_1 pos_2
        uint32_t vel_1 vel_2
        uint32_t dir_1 dir_2
        uint32_t fscala
        uint32_t vel_period
        uint32_t zero_pos
    }
    class pwm {
        <<struct - pwm.h>>
        uint32_t NCont
        uint32_t delta_1 delta_2
        uint8_t dir_1 dir_2
    }
    class colore {
        <<struct - sens.h>>
        int rosso verde blu
        int luminanza bianco
    }
    class temperatura {
        <<struct - sens.h>>
        float Temp
        int tempRaw
        int T_tar Tcase ok
    }
    class survivor {
        <<struct - sens.h>>
        uint8_t isSurvivor
    }

    syn_stat ..> glb : "rispondiComando(syn_stat*, glb*)"
    dati ..> gyro : pointer
    dati ..> temperatura : pointer
    dati ..> colore : pointer
    dati ..> survivor : pointer
    glb ..> qei : pointer
    glb ..> distMis : "DSTptr"
    glb ..> Giroscopio : "gyro ptr"
    glb ..> encQuad : "encoder ptr"
```

**Struct ↔ class duplication map** (old → new):

| Legacy C struct | Superseded by C++ class | Status in this release |
|---|---|---|
| `gyro` (gyro_f.h) | `Giroscopio` | struct still included everywhere; class is the active one |
| `qei` (qei.h) | `encQuad` | whole `qei.cpp` compiles but is never called from `main` |
| `pwm` (pwm.h) | `PWM_MOTORI` | struct referenced by legacy prototypes only |
| `colore` / `temperatura` / `survivor` (sens.h) | `COLORE` / `TEMPER` | structs survive inside `dati`/`glb` pointer bags |
| `syn_stat` (init.h) | `syntaxStatus` (parse.h) | **struct wins**: `main` uses `syn_stat`; the class is instantiated once and ignored |

---

## 4. Object inventory — the concrete instances

These are the actual objects created at runtime (all file-scope globals or locals in `main()`):

| Object | Type | Scope | Created in | Purpose |
|--------|------|-------|-----------|---------|
| `ENC0` | `encQuad` | global (`main.cpp:66`) | static | left/right encoder on **QEI0**, also written by ISR `IntGPIOf` |
| `ENC1` | `encQuad` | global (`main.cpp:66`) | static | second encoder on **QEI1** (never read in main loop) |
| `synSTATO` | `syn_stat` | local in `main()` | stack | parser state machine state |
| `CMD1` | `comando` | local in `main()` | stack | current command descriptor (token, numPid, tick...) |
| `XB` | `xBee` | local in `main()` | stack | radio module; `test()` probes presence at boot |
| `synStat` | `syntaxStatus` | local in `main()` | stack | **dead** — declared, never used |
| `uart1buffer[64]`, `RX_PTR1`, `READ_PTR1` | `volatile uint8_t` | global (`interrupt.c:28`) | static | UART1 receive ring buffer shared ISR ↔ main |
| `procCom`, `tick10`, `tick100`, `millis10` | `volatile int` | global (`main.cpp:59`) | static | scheduler flags set by `Timer0ISR` |
| `procCom4` | `volatile int` | global (`main.cpp:60`) | static | TIMER4 window-done flag (light sensor) |
| `contLightPwm` | `volatile int` | global (`interrupt.c:63`) | static | light pulse counter (`IntGpioA`) |
| `p` (in qei.cpp) | `qei*` | global (`qei.cpp:30`) | static | **never initialized** — NULL pointer risk |

Note the symmetric pattern: **encoders and sensors are globals; orchestrators (`synSTATO`, `CMD1`, `XB`) are stack locals of `main()`**. ISR code can only safely touch the globals — which is exactly what `IntGPIOf` does with `ENC0`.

---

## 5. Who-calls-whom: the relation matrix

Relations between the main actors (✅ = actually executed in this release; 🔶 = compiled but unreachable from the current `main()`; ⛔ = would crash if executed):

| Caller ↓ / Callee → | `PARSE` | `convertToToken` | `encQuad` methods | `Giroscopio` methods | `comando::RUN` | `PWM_MOTORI` | `xBee::test` | `sendReply` |
|---|---|---|---|---|---|---|---|---|
| `main()` loop | ✅ | 🔶 (only via `parse()`) | ✅ `readPos/readDir` | 🔶 | 🔶 | 🔶 | ✅ | 🔶 |
| `PARSE()` | — | 🔶 | — | — | — | — | — | — |
| `parse()` (unused twin) | — | ✅ | — | — | — | — | — | — |
| `comando::RUN` | — | — | 🔶 (params only) | 🔶 | — | 🔶 | — | — |
| ISR `IntGPIOf` | — | — | ✅ writes `ENC0` fields | — | — | — | — | — |
| ISR `UART1IntHandler` | — | — | — | — | — | — | — | — |
| `rispondiComando` | — | — | — | — | — | — | — | ✅ |
| `inviaSensore` | — | — | — | 🔶 reads `Giroscopio::yaw` | — | — | — | via caller |

Key takeaway: **the live data path is much narrower than the compiled code** — parser → (no token) → encoder reply, and ISR → `ENC0`. Everything else is dormant scaffolding, ready to be re-connected (the plan for that re-connection is documented in [state-of-code.md](state-of-code.md#7-suggested-re-activation-plan)).

---

*Next: [architecture.md](architecture.md) explains when these calls happen in time; [api-reference.md](api-reference.md) documents each one in detail.*

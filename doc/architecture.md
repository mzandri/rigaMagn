---
layout: default
title: Architecture
---

# Architecture — data flow, interrupts and timing

> This page answers the questions *how does a byte travel from the Raspberry Pi to a motor*, and *what happens every 10 ms*. The static object relations are in [class-map.md](class-map.md).

---

## 1. System architecture diagram

```mermaid
flowchart TB
    subgraph EXTERN["External world"]
        RPI["Raspberry Pi<br>mission controller"]
        XBW["XBee radio<br>telemetry"]
    end

    subgraph TIVA["TM4C123GH6PM @ 80 MHz"]
        subgraph COMM["Communication layer"]
            U0["UART0<br>PA0/PA1 - debug console"]
            U1["UART1 RX ISR<br>PB0/PB1 - ring buffer"]
        end

        subgraph SCHED["Scheduler"]
            T0["TIMER0 ISR<br>every 10 ms"]
            FLAGS["procCom / tick10 / tick100<br>volatile flags"]
        end

        subgraph PARSE["Command path"]
            SM["PARSE state machine<br>syn_stat"]
            CT["convertToToken<br>(dormant)"]
            CMDR["comando + ControlloPID<br>(dormant)"]
        end

        subgraph SENSE["Sensors"]
            QEI0H["QEI0 hardware<br>PD6 PD7 + PF4 idx"]
            QEI1H["QEI1 hardware<br>PC5 PC6"]
            GY["Giroscopio class<br>L3GD20 on I2C0"]
            DIST["distMis class<br>5 IR sensors on ADC<br>(dormant)"]
            COL["COLORE / TEMPER<br>(dormant)"]
        end

        subgraph ACT["Actuators"]
            PWM["PWM_MOTORI x2<br>PE5 / PB4 + dir pins<br>(dormant)"]
            SRV["PWM_SERVI x2<br>PD0 PD1<br>(dormant)"]
        end

        ENC0["ENC0 : encQuad"]
        ENC1["ENC1 : encQuad"]
        LED["LEDs PF1-PF3"]
    end

    RPI -->|"4-byte frames 115200 8N1"| U1
    U1 -->|"uart1buffer[]"| SM
    SM -->|"valid frame"| RPI_REPLY["reply: encoder bytes<br>via UART0 PRINTF"]
    RPI_REPLY --> U0
    U0 --> PC["PC terminal"]

    XBW <-->|"AT commands / '+' probe"| U1
    XBW -.->|"reset line"| PB5["PB5 GPIO"]

    T0 --> FLAGS
    FLAGS --> T0
    QEI0H -->|"position/direction"| ENC0
    QEI1H -->|"position/direction"| ENC1
    PF4["PF4 index pulse"] -->|"IntGPIOf ISR"| ENC0
    GY -->|"yaw integration"| ROT["Rot / telemetry values<br>(debug prints)"]
    ENC0 -->|"100 ms print"| U0

    SM -.->|"token (if wired)"| CT
    CT -.-> CMDR
    CMDR -.-> PWM
    CMDR -.-> SRV
    CMDR -.-> GY

    style RPI fill:#FFF7ED,stroke:#F59E0B
    style XBW fill:#FFF7ED,stroke:#F59E0B
    style U0 fill:#EFF6FF,stroke:#3B82F6
    style U1 fill:#EFF6FF,stroke:#3B82F6
    style T0 fill:#F0FDF4,stroke:#10B981
    style FLAGS fill:#F0FDF4,stroke:#10B981
    style SM fill:#EFF6FF,stroke:#3B82F6
    style ENC0 fill:#EFF6FF,stroke:#3B82F6
    style ENC1 fill:#EFF6FF,stroke:#3B82F6
    style GY fill:#EFF6FF,stroke:#3B82F6
    style QEI0H fill:#F1F5F9,stroke:#64748B
    style QEI1H fill:#F1F5F9,stroke:#64748B
    style DIST fill:#FEE2E2,stroke:#EF4444,stroke-dasharray: 5 5
    style COL fill:#FEE2E2,stroke:#EF4444,stroke-dasharray: 5 5
    style PWM fill:#FEE2E2,stroke:#EF4444,stroke-dasharray: 5 5
    style SRV fill:#FEE2E2,stroke:#EF4444,stroke-dasharray: 5 5
    style CT fill:#FEE2E2,stroke:#EF4444,stroke-dasharray: 5 5
    style CMDR fill:#FEE2E2,stroke:#EF4444,stroke-dasharray: 5 5
```

Solid blue = live path · dashed red = compiled but unreachable from `main()` in this release.

---

## 2. Interrupt wiring

The interrupt vector table (`tm4c123gh6pm_startup_ccs.c`) binds these handlers:

```mermaid
flowchart LR
    subgraph SOURCES["Hardware event sources"]
        EV1["UART1 RX FIFO<br>byte from Raspberry Pi"]
        EV2["TIMER0A timeout<br>10 ms"]
        EV3["TIMER4A timeout<br>10 ms sampling window"]
        EV4["PF4 rising edge<br>QEI0 index pulse"]
        EV5["PA3 rising edge<br>light sensor pulses"]
    end

    subgraph HANDLERS["ISRs (interrupt.c / encQuad.cpp / tiva_timer.c)"]
        H1["UART1IntHandler"]
        H2["Timer0ISR"]
        H3["Timer4ISR"]
        H4["IntGPIOf"]
        H5["IntGpioA"]
    end

    subgraph SHARED["Shared state (volatile globals)"]
        S1["uart1buffer[64]<br>RX_PTR1 / READ_PTR1"]
        S2["procCom / tick10 / tick100"]
        S3["procCom4"]
        S4["ENC0.pos / posV[] / contIDX"]
        S5["contLightPwm"]
    end

    EV1 --> H1 --> S1
    EV2 --> H2 --> S2
    EV3 --> H3 --> S3
    EV4 --> H4 --> S4
    EV5 --> H5 --> S5

    S1 -->|"main loop consumes"| MAIN["main() while(1)"]
    S2 -->|"cadence 10ms/100ms/1s"| MAIN
    S3 -->|"window done"| READCOL["readCol() callers"]
    S4 -->|"100 ms read"| MAIN
```

Points worth noting:

- **Single-producer / single-consumer ring buffers** are used in two places (`uart1buffer` and `ENC0.posV[]`), the classic embedded idiom — see [student-notes.md](student-notes.md#4-ring-buffers-and-the-volatile-keyword).
- `IntGPIOf` (defined in `encQuad.cpp`) is registered **statically** in the vector table, not via `GPIOIntRegister` — the comment lines in `qeiInit` show both approaches were tried.
- `Timer1ISR` and the `QEI0/1IntHandler` pair in `qei.cpp` are **not referenced by the vector table** → dead handlers (see [state-of-code.md](state-of-code.md#2-dead-code-inventory)).
- The UART1 interrupt handler keeps running while XBee `test()` polls the same port — the `'+'` probe and the `OK` answer flow through the same ring buffer, which is why `test()` resets `READ_PTR1/RX_PTR1` first.

---

## 3. Main-loop timeline (one iteration)

```mermaid
sequenceDiagram
    participant T0 as TIMER0 ISR (every 10 ms)
    participant M as main() while(1)
    participant P as PARSE()
    participant E as ENC0 (encQuad)
    participant U as UART0 console

    Note over M: every pass: UARTCharGet(UART0) blocks!<br>waits for a keyboard char on the console
    T0-->>M: procCom = 1, tick10++, tick100++
    M->>M: read byte from uart1buffer ring?
    alt new byte(s) available
        M->>P: PARSE(&synSTATO)
        P->>M: synSTATO.valid ?
        alt VALIDO
            M->>U: print ENC0.pos + posV[0..3] + posV[8..11] bytes
            M->>M: synSTATO.valid = NON_VALIDO
        end
        M->>M: READ_PTR1++ (mod 64)
    end
    alt procCom == 1 (10 ms tick)
        M->>M: contatore++, millis10++
        M->>M: (PID would run here - dormant)
    end
    alt tick10 >= 10 (100 ms)
        M->>E: readPos() / readDir()
        M->>U: "POS: pos dir"
    end
    alt tick100 >= 100 (1 s)
        M->>U: toggle green LED, print distances / yaw
    end
```

⚠️ The first statement inside `while(1)` is `UARTCharGet(UART0_BASE)` — a **blocking** console read. One loop pass happens only per typed character. This is fine on a test bench (you press a key, you see telemetry) but it would starve the rover during a real match. Fix suggestions: [state-of-code.md §6](state-of-code.md#6-bugs-and-fragile-points-summary-table).

---

## 4. The parser state machine

`PARSE()` processes **one byte per call** and advances `STATO->ST` through a 4-state automaton. A frame is `CMD DATA CHECK TERM` (4 bytes):

```mermaid
stateDiagram-v2
    [*] --> ST0 : resetAutoma()
    ST0 : ST = 0 — wait for header
    ST1 : ST = 1 — data byte
    ST2 : ST = 2 — checksum test
    ST3 : ST = 3 — terminator seen
    ST0 --> ST0 : byte not in A-Z<br>(stay, resync)
    ST0 --> ST1 : byte is A-Z<br>check = byte
    ST1 --> ST2 : check ^= data
    ST2 --> ST2 : check ^= 0xA9 ≠ cmd[2]<br>back to resync (ST=0 written)
    ST2 --> ST3 : check ^= 0xA9 == cmd[2]<br>valid = VALIDO
    ST3 --> ST0 : reset, valid = NON_VALIDO
```

Verification: `check = header XOR data XOR 0xA9` must equal byte 3. `CHECK_SUM` constant is `0xA9` (`init.h:21`).

Command letters recognised by `convertToToken()` (note: in this release only reachable via the unused twin `parse()`):

| Letter | Token constant | `numPid` | Effect (when wired) |
|--------|---------------|----------|---------------------|
| `F` | `AVANTI` (1) | 0 | both motors forward |
| `B` | `INDIETRO` (2) | 0 | both motors backward |
| `S` | `STOP` (0) | -1 | stop motors |
| `R` | `DESTRA` (3) | 1 | turn right 90° (`valFin -= 90`) |
| `L` | `SINISTRA` (4) | 2 | turn left 90° (`valFin += 90`) |
| `I` | `GIRA_INDIETRO` (5) | unset | 180° spin |
| `G` | `MISURA_GRADI` (10) | — | query angle (not implemented) |
| `D` | `LETTURA_SENSORE` (7) | — | read sensor # (ID in byte 2) |
| `P` | `RILASCIO_PACK` (8) | — | release rescue pack — **falls through to `default`** ⚠ |
| *(other)* | `ERRORE` (255) | -1 | error |

Sensor IDs served by `inviaSensore()` for `D` commands:

| ID | Reading | Source object |
|----|---------|---------------|
| 1–5 | distance mm | `glb.DSTptr->d_mm[0..4]` |
| 6 | yaw | `glb.gyro->yaw` |
| 7 | luminance | `glb.colorClass->luminanza` |
| 8 | temperature | `glb.temperat->Tint` |
| 9 | speed cm/s | hardcoded `1` (placeholder) |
| 10 | travelled distance | `glb.encoder->dist_mm` |
| 11 | accelerometer angle | commented out |

Reply frame: 4 payload bytes + `'*'` terminator (`sendReply()`).

---

## 5. Boot sequence

```mermaid
flowchart TB
    A["Reset vector<br>tm4c123gh6pm_startup_ccs.c"] --> B["main()"]
    B --> C["ENC0/ENC1 setAddr(QEI0/QEI1_BASE)"]
    C --> D["DI() — interrupts off"]
    D --> E["setupMCU()<br>80 MHz PLL, LEDs, PB5, PF0 button wait"]
    E --> F["setupUART(0)<br>UART0 + UART1 @ 115200, PRINTF on UART0"]
    F --> G["PRINT_WELCOME()"]
    G --> H["initTimer0(10)<br>TIMER0A, 10 ms tick"]
    H --> I["synSTATO.tick = 0"]
    I --> J["ENC0.qeiInit() + ENC1.qeiInit()<br>PD7 unlock, QEI config, PF4 int enable"]
    J --> K["EI() — interrupts on"]
    K --> L["XB.test()<br>'+++' probe, ATCN, status = TXRX"]
    L --> M["while(1) main loop"]

    style A fill:#F1F5F9,stroke:#64748B
    style M fill:#EFF6FF,stroke:#3B82F6,stroke-width:2px
    style E fill:#F0FDF4,stroke:#10B981
    style J fill:#F0FDF4,stroke:#10B981
    style L fill:#FFF7ED,stroke:#F59E0B
```

Details of every step (and the exact registers touched) are in [api-reference.md](api-reference.md).

---

*Next: [class-map.md](class-map.md) for the object view, or [api-reference.md](api-reference.md) for the function-level reference.*

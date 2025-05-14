## Telemetry Configuration Language

```
!! optionName  optionValue              # Global option line
>  BoardName [free-text …]              # Board line
>> MessageName messageID messageSize    # Message line
>>> SignalName dataType startBit length factor offset [signedness] [endianness]
>>>> EnumName rawValue                  # Enum entry (only if dataType = enum)
```

---

### 1 Purpose

A compact, human-readable file format for describing

- **Which CAN messages** the telemetry subsystem listens to, and
- **How each message’s payload is decoded and logged.**

The format is hierarchical, line-oriented, and version-controlled-friendly (diffable).

---

### 2 Prefixes & Hierarchy

| Prefix | Scope             | Required Parent | Example                        |
| ------ | ----------------- | --------------- | ------------------------------ |
| `!!`   | **Global option** | —               | `!! logPeriodMs 50`            |
| `>`    | **Board / ECU**   | —               | `> POWERTRAIN`                 |
| `>>`   | **Message**       | Board           | `>> DRIVESTATUS 0x200 8`       |
| `>>>`  | **Signal**        | Message         | `>>> MotorRPM uint16 0 16 1 0` |
| `>>>>` | **Enum entry**    | Signal (enum)   | `>>>> CHARGE 1`                |

_Every `>>>` line **must** be nested under a `>>`, and every `>>` under a `>` (strict hierarchy)._

---

### 3 Lexical Rules (apply to all lines)

- Whitespace (spaces / tabs) separates fields.
- `#` starts a comment; the rest of the line is ignored.
- Blank lines may appear anywhere.
- Identifiers (`BoardName`, `MessageName`, `SignalName`, enum names) follow the regex
  `[A-Za-z_][A-Za-z0-9_]*` and are case-sensitive.
- Integers may be decimal (`123`) or hexadecimal (`0x7A`); **message IDs must be hex**.
- Floating literals use C syntax (`3.14`, `-1.2e-3`).

---

### 4 Global Option Lines (`!!`)

```
!! optionName optionValue
```

| Option        | Value type  | Effect                                                                              |
| ------------- | ----------- | ----------------------------------------------------------------------------------- |
| `logPeriodMs` | **int > 0** | Default logging period (ms) applied to _all_ messages unless firmware overrides it. |

Later duplicate `!! logPeriodMs …` lines override earlier ones.
Unknown option names: parser **warns** but continues (forward-compatibility).

---

### 5 Board Lines (`>`)

```
> BoardName [description …]
```

- `BoardName` must be unique in the file.
- Optional free-text description (everything after the name) is ignored by the parser but kept for UIs.

---

### 6 Message Lines (`>>`)

```
>> MessageName messageID messageSize
```

| Field         | Type       | Constraints                                       |
| ------------- | ---------- | ------------------------------------------------- |
| `MessageName` | identifier | Unique within its Board.                          |
| `messageID`   | **hex**    | `0x000 – 0x7FF` (11-bit only).                    |
| `messageSize` | int        | 0 – 8 bytes (classic CAN) or up to 64 for CAN FD. |

_No extended-ID flag and no period field; IDs are always standard 11-bit._

---

### 7 Signal Lines (`>>>`)

```
>>> SignalName dataType startBit length factor offset [signedness] [endianness]
```

| Field        | Type       | Details & Defaults                                                                                         |                                                 |
| ------------ | ---------- | ---------------------------------------------------------------------------------------------------------- | ----------------------------------------------- |
| `SignalName` | identifier | Unique within the Message.                                                                                 |                                                 |
| `dataType`   | keyword    | • `int8/16/32/64` (signed by default) • `uint8/16/32/64` (unsigned) • `float` • `double` • `bool` • `enum` |                                                 |
| `startBit`   | **int**    | 0 ≤ startBit < `messageSize × 8`.                                                                          |                                                 |
| `length`     | **int**    | 1 – 64 bits ≤ payload size.                                                                                |                                                 |
| `factor`     | float      | Scaling multiplier.                                                                                        |                                                 |
| `offset`     | float      | Scaling offset.                                                                                            |                                                 |
| `signedness` | _(opt.)_   | `signed`                                                                                                   | `unsigned`. Overrides the type-implied default. |
| `endianness` | _(opt.)_   | `little` (default)                                                                                         | `big`.                                          |

> **Scaling formula** > `physical = (isSigned ? signExtend(rawBits) : rawBits) × factor + offset`

---

### 8 Enumerations (only if `dataType = enum`)

Immediately after the signal line, list one or more enum entries:

```
>>>> EnumName rawValue
```

- `rawValue` is an int that fits in `length` bits.
- Parsing of enum entries stops at the first non-`>>>>` line.

---

### 9 Validation Checklist

1. **Hierarchy** – A `>>>` without a current `>>`, or a `>>` without a current `>`, is an error.
2. **Bit-fit** – `startBit + length` > `messageSize × 8` → error.
3. **Bit overlap** – Signals within a message must not share bits.
4. **Message ID range** – ID outside `0x000 – 0x7FF` → error.
5. **Name clashes** – Duplicate Board, Message, or Signal names at the same scope.
6. **Enum sanity** – Duplicate enum raw values → error / missing raw values → warning.
7. **Field legality** –

   - `signedness`, if present, is neither `signed` nor `unsigned`.
   - `endianness`, if present, is neither `little` nor `big`.

---

### 10 Informal EBNF

```
file        ::= { blank | comment | option | board }
option      ::= "!!" ws name ws number nl
board       ::= ">"  ws name [ ws text ] nl
                { blank | comment | message }+
message     ::= ">>" ws name ws hex ws number nl
                { blank | comment | signal }+
signal      ::= ">>>" ws name ws type ws number ws number ws float ws float
                [ ws signedness ] [ ws endianness ] nl
                { enumEntry }
enumEntry   ::= ">>>>" ws name ws number nl
comment     ::= "#" { any } nl
blank       ::= nl
nl          ::= "\n" | "\r\n"
ws          ::= 1*(" " | "\t")
hex         ::= "0x" 1*hexDigit
```

---

### 11 Complete Worked Example

```ini
!! logPeriodMs 50     # global default: 20 Hz logging

# ───── POWERTRAIN ECU ───────────────────────────────────────────────────────
> POWERTRAIN            # traction inverter node

>> DRIVESTATUS 0x200 8  # status every 50 ms by default
>>> MotorRPM       uint16  0 16 1      0           # little-endian (default)
>>> InverterTemp   int16  16 16 0.1    0
>>> FaultFlags     uint16 32 16 1      0 big       # override endianness

>> DRIVE_CMD 0x201 8
>>> TorqueCmd      int16   0 16 0.01   0
>>> Enable         bool   16  1 1      0

# ───── BMS ECU ──────────────────────────────────────────────────────────────
> BMS

>> BMS_PACK 0x300 8
>>> PackVoltage    float  0 32 0.001   0
>>> PackCurrent    float 32 32 0.001   0

>> BMS_MODE 0x301 1
>>> Mode enum 0 3 1 0
>>>> OFF    0
>>>> CHARGE 1
>>>> DRIVE  2
```

---

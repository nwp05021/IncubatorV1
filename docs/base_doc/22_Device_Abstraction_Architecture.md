# 22_Device_Abstraction_Architecture

> Version: 1.0
> Status: Authoritative

---

# 목적

본 문서는 Device Layer 추상화 전략을 정의한다.

---

# 1. 핵심 철학

```text
Device는 하드웨어 접근만 수행한다.
```

---

# 2. Device Layer 역할

| 역할 | 설명 |
|---|---|
| GPIO 접근 | O |
| I2C 접근 | O |
| SPI 접근 | O |
| 상태 판단 | X |
| 정책 판단 | X |
| Alarm 발생 | X |

---

# 3. 권장 구조

```text
IDevice
    ↓
Aht20Device
RelayDevice
FanPwmDevice
DisplayDevice
```

---

# 4. 인터페이스 예시

```cpp
class ITemperatureHumiditySensor
{
public:
    virtual bool read(float& tempC,
                      float& humidityPct) = 0;
};
```

---

# 5. 장점

- 센서 교체 용이
- Mock Test 가능
- 유지보수 향상

---

# 6. 금지

```text
❌ Device 내부 비즈니스 로직
❌ Device 내부 Recovery
❌ Device 내부 상태 저장
```

---

# 7. 최종 원칙

```text
Device는 드라이버일 뿐이다.
```

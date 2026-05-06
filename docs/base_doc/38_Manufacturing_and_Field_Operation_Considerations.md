# 38_Manufacturing_and_Field_Operation_Considerations

> Version: 1.0
> Status: Authoritative

---

# 목적

본 문서는 실제 현장 운영과 양산 관점을 정의한다.

---

# 1. 핵심 철학

```text
현장에서 살아남는 제품이어야 한다.
```

---

# 2. 고려 요소

| 항목 | 설명 |
|---|---|
| 전원 불안정 | 복구 |
| 네트워크 불안정 | Offline |
| 사용자 오조작 | 보호 |
| 장시간 동작 | 안정성 |

---

# 3. 현장 운영 전략

| 전략 | 설명 |
|---|---|
| SafeMode | 출력 보호 |
| Alarm | 즉시 인지 |
| Restore | 자동 복구 |
| Diagnostics | 원인 추적 |

---

# 4. 양산 고려

권장:

```text
Firmware Version 표시
Device ID 표시
Provisioning 단순화
```

---

# 5. 최종 원칙

```text
개발 환경이 아니라
현장을 기준으로 설계한다.
```

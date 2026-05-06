# 41_Command_Definitions_and_Validation_Rules

> Version: 1.0
> Status: Authoritative

---

# 목적

본 문서는 Command 정의 및 Validation 규칙을 정의한다.

---

# 1. 핵심 철학

```text
모든 외부 입력은 Command가 된다.
```

---

# 2. 권장 Command 목록

| Command | 설명 |
|---|---|
| StartBatch | 부화 시작 |
| StopBatch | 부화 중단 |
| PatchPlanRow | Day 수정 |
| UpdateSettings | 설정 변경 |
| ToggleManualHeater | 수동 히터 |
| ClearSafeMode | SafeMode 해제 |

---

# 3. Validation 예시

| 항목 | 검증 |
|---|---|
| Temp | 허용 범위 |
| Day | Batch 범위 |
| SafeMode | 위험 출력 차단 |

---

# 4. Validation 흐름

```text
Command
    ↓
Validator
    ↓
Reject / Apply
```

---

# 5. Reject 전략

Reject 시:

- Event 발행
- UI 표시
- Cloud Error 응답 가능

---

# 6. 최종 원칙

```text
잘못된 상태는 절대 허용하지 않는다.
```

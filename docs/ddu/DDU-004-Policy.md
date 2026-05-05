# DDU-004 — 부화 정책 (Preset + PlanGenerator + DayResolver)
> **Document ID**: DDU-004  
> **Version**: 1.0  
> **상위 문서**: INC-IMPL-001 §7  
> **의존 DDU**: DDU-001  
> **Codex 작업 시간 예상**: 10~15분

---

## 작업 목표

이 DDU 완료 후:
- 조류 4종(Chicken/Duck/Quail/Goose) Preset이 IncubationPlanTable을 정확히 생성한다
- `PlanGenerator::generate(species, batch, table)` 하나로 전체 테이블 생성된다
- `DayResolver::resolve(startEpoch, nowEpoch, totalDays)` 로 현재 Day 계산된다

---

## 1. 생성할 파일 목록

```
include/policy/IPresetPolicy.h
include/policy/ChickenPreset.h
include/policy/DuckPreset.h
include/policy/QuailPreset.h
include/policy/GoosePreset.h
include/policy/PlanGenerator.h
include/policy/DayResolver.h

src/policy/ChickenPreset.cpp
src/policy/DuckPreset.cpp
src/policy/QuailPreset.cpp
src/policy/GoosePreset.cpp
src/policy/PlanGenerator.cpp
src/policy/DayResolver.cpp
```

---

## 2. IPresetPolicy

```cpp
// include/policy/IPresetPolicy.h
#pragma once
#include "domain/IncubationPlanTable.h"
#include "domain/IncubationBatch.h"

namespace incubator::policy
{
    class IPresetPolicy
    {
    public:
        virtual ~IPresetPolicy() = default;

        // batch의 totalDays / lockdownStartDay 를 채우고
        // table 전체 rows 를 생성한다.
        virtual void fill(domain::IncubationPlanTable& table,
                          domain::IncubationBatch&     batch) const = 0;
    };
}
```

---

## 3. 조류별 Preset 기준값 (필수 준수)

| 조류 | totalDays | lockdownStart | 일반 구간 온도 | 일반 구간 습도 | 전란 간격 | Lockdown 온도 | Lockdown 습도 |
|---|---|---|---|---|---|---|---|
| Chicken | 21 | 19 | 37.8°C | 55% | 120분 | 37.2°C | 65% |
| Duck | 28 | 25 | 37.5°C | 55% | 180분 | 37.0°C | 65% |
| Quail | 17 | 15 | 37.8°C | 45% | 120분 | 37.2°C | 60% |
| Goose | 30 | 27 | 37.4°C | 55% | 180분 | 37.0°C | 65% |

**공통 규칙**:
- Day 1 ~ (lockdownStartDay - 1): 일반 구간 (전란 ON)
- Day lockdownStartDay ~ totalDays: Lockdown 구간 (전란 OFF)
- ventFanEnabled = true (전체 일수)
- userOverridden = false (초기 생성 시)

---

## 4. ChickenPreset 구현 패턴 (다른 조류도 동일 패턴)

```cpp
// include/policy/ChickenPreset.h
#pragma once
#include "IPresetPolicy.h"

namespace incubator::policy
{
    class ChickenPreset final : public IPresetPolicy
    {
    public:
        void fill(domain::IncubationPlanTable& table,
                  domain::IncubationBatch&     batch) const override;
    };
}

// src/policy/ChickenPreset.cpp
#include "policy/ChickenPreset.h"
#include "domain/IncubationPlanRow.h"
#include <ctime>

using namespace incubator::domain;

namespace incubator::policy
{

void ChickenPreset::fill(IncubationPlanTable& table, IncubationBatch& batch) const
{
    batch.totalDays        = 21;
    batch.lockdownStartDay = 19;
    table.clear();

    // Day 1~18: 일반 구간
    for (uint16_t d = 1; d <= 18; ++d) {
        table.rows[table.rowCount++] =
            IncubationPlanRow::make(d, 37.8f, 55.0f, true, 120U, true);
    }

    // Day 19~21: Lockdown
    for (uint16_t d = 19; d <= 21; ++d) {
        table.rows[table.rowCount++] =
            IncubationPlanRow::make(d, 37.2f, 65.0f, false, 0U, true);
    }

    table.tableVersion = 1;
    table.lastUpdatedAt = static_cast<uint32_t>(time(nullptr));
}

} // namespace incubator::policy
```

---

## 5. PlanGenerator

```cpp
// include/policy/PlanGenerator.h
#pragma once
#include "domain/IncubationPlanTable.h"
#include "domain/IncubationBatch.h"
#include "domain/IncubationSpecies.h"

namespace incubator::policy
{
    class PlanGenerator
    {
    public:
        // ★ 유일한 외부 진입점.
        //   species에 맞는 Preset을 선택해 table과 batch를 채운다.
        //   Custom species의 경우 Chicken Preset을 기본으로 사용.
        //   반환: 성공 여부
        static bool generate(domain::Species              species,
                             domain::IncubationBatch&     batch,
                             domain::IncubationPlanTable& table);
    };
}

// src/policy/PlanGenerator.cpp 구현:
// switch(species):
//   Chicken → ChickenPreset
//   Duck    → DuckPreset
//   Quail   → QuailPreset
//   Goose   → GoosePreset
//   Custom  → ChickenPreset (기본값)
// preset.fill(table, batch)
// return table.isValid()
```

---

## 6. DayResolver

```cpp
// include/policy/DayResolver.h
#pragma once
#include <cstdint>

namespace incubator::policy
{
    class DayResolver
    {
    public:
        // startEpoch : 부화 시작 Unix timestamp (초)
        // nowEpoch   : 현재 Unix timestamp (초)
        // totalDays  : 총 부화 일수
        // 반환       : 1-based 현재 day (최소 1, 최대 totalDays 클램핑)
        static uint16_t resolve(uint32_t startEpoch,
                                uint32_t nowEpoch,
                                uint16_t totalDays);

        // 부화 완료 예정 epoch 계산
        static uint32_t completionEpoch(uint32_t startEpoch,
                                        uint16_t totalDays)
        {
            return startEpoch + (uint32_t)totalDays * 86400U;
        }

        // 진행률 계산 (0~100%)
        static uint8_t progressPct(uint16_t currentDay, uint16_t totalDays)
        {
            if (totalDays == 0) return 0;
            uint32_t pct = (uint32_t)currentDay * 100U / totalDays;
            return (pct > 100) ? 100 : static_cast<uint8_t>(pct);
        }
    };
}

// src/policy/DayResolver.cpp 구현:
// resolve():
//   if (nowEpoch <= startEpoch) return 1
//   uint32_t elapsed = nowEpoch - startEpoch
//   uint16_t day = (uint16_t)(elapsed / 86400U) + 1
//   return (day > totalDays) ? totalDays : day
```

---

## 완료 기준 (Acceptance Criteria)

| # | 항목 | 기준 |
|---|---|---|
| AC-1 | Chicken 생성 | rowCount == 21, Day 1 targetTempC == 37.8 |
| AC-2 | Lockdown | Day 19 turningEnabled == false, targetTempC == 37.2 |
| AC-3 | Duck 생성 | rowCount == 28, turningIntervalMin == 180 |
| AC-4 | Quail 생성 | rowCount == 17, targetHumidityPct Day 1 == 45.0 |
| AC-5 | Goose 생성 | rowCount == 30, totalDays == 30 |
| AC-6 | DayResolver | startEpoch=0, nowEpoch=86400 → day==2 |
| AC-7 | DayResolver 클램핑 | nowEpoch 매우 크면 → day == totalDays |
| AC-8 | progressPct | day=7, total=21 → 33% |
| AC-9 | userOverridden | 초기 생성 rows 전체 userOverridden == false |

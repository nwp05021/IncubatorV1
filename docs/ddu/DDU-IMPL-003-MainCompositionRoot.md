
# DDU-IMPL-003 — main.cpp Composition Root

> Version: 1.0
> Status: Draft

---

# 1. 목적

main.cpp를 Composition Root 구조로 고정한다.

---

# 2. 핵심 철학

```text
객체 생성은 main.cpp
정책은 외부
```

---

# 3. main.cpp 예시

```cpp
#include <Arduino.h>

#include "product/domain/RuntimeState.h"
#include "product/domain/AppSettings.h"

#include "product/app/CommandQueue.h"
#include "product/app/AppController.h"

using namespace incubator;

domain::RuntimeState runtime;
domain::AppSettings settings;

app::CommandQueue commandQueue;

app::AppController appController(
    runtime,
    settings,
    commandQueue);

void setup()
{
    Serial.begin(115200);
}

void loop()
{
    appController.tick();
}
```

---

# 4. 절대 규칙

```text
main.cpp 내부 상태 변경 금지
```

금지:

```text
❌ runtime.targetTemp = 37.5f
```

반드시:

```text
Command
    ↓
Queue
    ↓
AppController
```

---

# 5. 목표

```text
main.cpp를 읽으면
전체 wiring이 보이도록 유지
```

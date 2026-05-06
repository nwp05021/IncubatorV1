# 65_AppController_Implementation_Template

> Version: 1.0
> Status: Implementation Template

---

# 목적

본 문서는 AppController 구현 템플릿을 제공한다.

---

# 기본 구조

```cpp
class AppController
{
public:
    void enqueue(const Command& cmd);

    void tick();

private:
    void processQueue();

    bool validate(const Command& cmd);

    void apply(const Command& cmd);

private:
    RuntimeState& m_runtime;

    AppSettings& m_settings;

    CommandQueue m_queue;
};
```

---

# Tick 구조

```cpp
void AppController::tick()
{
    processQueue();
}
```

---

# 핵심 원칙

```text
모든 상태 변경은 중앙화한다.
```

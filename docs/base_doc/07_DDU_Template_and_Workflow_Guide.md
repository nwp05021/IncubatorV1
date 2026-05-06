# 07_DDU_Template_and_Workflow_Guide

> Version: 1.0
> Status: Authoritative

---

# 목적

본 문서는 Incubator Firmware 프로젝트에서 사용하는
표준 DDU(Document Driven Unit) 작성 규칙을 정의한다.

목표:

- 사람이 이해 가능한 구현 단위 유지
- Codex/Cursor 의존 최소화
- 구조 붕괴 방지
- 장기 유지보수 가능 구조 확보

---

# 1. DDU 기본 철학

DDU는:

```text
작고
독립적이며
검증 가능해야 한다.
```

좋은 DDU의 핵심:

- 하나의 책임
- 명확한 입력
- 명확한 출력
- 명확한 완료 조건

---

# 2. 권장 DDU 크기

권장 작업 시간:

```text
10분 ~ 40분
```

이상적인 DDU:

- Renderer 하나
- StateMachine 하나
- Serializer 하나
- Overlay 하나
- 작은 Control Logic 하나

---

# 3. 금지되는 DDU

```text
❌ 전체 UI 구현
❌ 전체 Cloud 구현
❌ 전체 제품 생성
❌ 대규모 자동 생성
```

---

# 4. 표준 DDU 템플릿

## Header

```md
# DDU-XXX — 제목

> Version: 1.0
> Status: Draft
> Dependency:
> Estimated Time:
```

---

## 목적

```md
이 DDU 완료 후:
- 무엇이 가능한가
- 무엇이 동작하는가
```

---

## 생성 파일

```md
include/...
src/...
```

---

## 핵심 구조

```md
상태 흐름
입력
출력
```

---

## 금지 사항

```md
❌ RuntimeState 직접 수정
❌ delay()
❌ UI direct GPIO
```

---

## Acceptance Criteria

```md
AC-1
AC-2
AC-3
```

---

# 5. 상태 흐름 우선

항상:

```text
상태 흐름
    ↓
구조
    ↓
코드
```

순서로 진행한다.

---

# 6. Codex 작업 규칙

Codex에는 반드시:

- 작은 범위
- 명확한 목표
- 정확한 파일
- 정확한 입력/출력

만 제공한다.

---

# 7. 최종 원칙

```text
문서가 코드보다 우선한다.
```

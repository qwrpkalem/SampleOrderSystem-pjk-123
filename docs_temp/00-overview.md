# 개발 Phase 개요

`docs/features/`의 요구사항을 바탕으로, 의존관계가 적은 것부터 개발할 수 있도록 Phase를 나눈다.
각 Phase는 [MyTDD skill](../.claude/skills/MyTDD/SKILL.md)의 Red-Green-Review-Refactor 사이클을
따라 진행한다.

## Phase 목록

| Phase | 이름 | 문서 | 선행 Phase |
| --- | --- | --- | --- |
| 1 | 기반 구조 (도메인 모델 + JSON 영속성) | [phase1-foundation.md](./phase1-foundation.md) | - |
| 2 | 시료 관리 (등록 / 조회 / 검색) | [phase2-sample-management.md](./phase2-sample-management.md) | 1 |
| 3 | 시료 주문 접수 | [phase3-order.md](./phase3-order.md) | 1, 2 |
| 4 | 주문 승인/거절 | [phase4-order-approval.md](./phase4-order-approval.md) | 1, 2, 3 |
| 5 | 생산 라인 | [phase5-production-line.md](./phase5-production-line.md) | 1, 2, 4 |
| 6 | 모니터링 | [phase6-monitoring.md](./phase6-monitoring.md) | 1, 2, 3, 5 |
| 7 | 출고 처리 | [phase7-release.md](./phase7-release.md) | 1, 3, 4 |
| 8 | 메인 메뉴 (콘솔 UI 통합) | [phase8-mainmenu.md](./phase8-mainmenu.md) | 2~7 전체 |

## 선정 이유

- **Phase 1 (기반 구조)**: Sample/Order 도메인 모델과 JSON 저장/불러오기가 없으면 다른 어떤 기능도
  테스트를 작성할 수 없으므로 가장 먼저 진행한다.
- **Phase 2 (시료 관리)**: 시료가 등록되어 있어야 주문이 가능하므로 다음 순서.
- **Phase 3 (시료 주문 접수)**: `RESERVED` 주문 생성 로직. 승인/거절보다 먼저 필요.
- **Phase 4 (주문 승인/거절)**: 재고 판정(승인 시점 기준) 및 상태 전이(`CONFIRMED`/`PRODUCING`/
  `REJECTED`)를 구현. 생산 라인 등록의 트리거가 되므로 Phase 5보다 먼저.
- **Phase 5 (생산 라인)**: `PRODUCING` 주문을 소비해 재고를 늘리고 `CONFIRMED`로 전환하는 핵심 로직.
  프로그램 재시작 시 생산 완료 판정도 이 Phase에서 처리.
- **Phase 6 (모니터링)**: 앞선 Phase들의 데이터(주문 상태별 집계, 재고 여유/부족/고갈)를 조회만
  하므로 해당 데이터가 존재해야 의미 있는 테스트 작성 가능.
- **Phase 7 (출고 처리)**: `CONFIRMED` → `RELEASE` 전이만 담당하는 단순 기능이라 후반부에 배치.
- **Phase 8 (메인 메뉴)**: 모든 하위 기능이 준비된 후 콘솔 UI로 통합.

## 완료 기준 (공통)

각 Phase는 아래 기준을 만족해야 완료로 간주한다.

- 해당 Phase 범위의 모든 요구사항에 대해 실패하는 테스트를 먼저 작성하고 통과시켰다.
- 관련 `docs/features/*.md` 문서의 요구사항/결정 사항과 구현이 일치한다.
- 문서에 새로운 결정이 필요했다면 `docs/PRD.md`와 `docs/features/*.md`를 함께 Update했다
  (참고: `CLAUDE.md`의 "문서 동기화 규칙").

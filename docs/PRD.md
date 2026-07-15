# 반도체 시료 생산주문관리 시스템 - PRD

> 출처: `Semi.pdf` (Chapter 01 개인과제 개요, Chapter 02 기능 명세) p.4-23

## 1. 배경 및 목적

가상의 반도체 회사 "S-Semi"는 다양한 반도체 시료(Sample)를 생산하여 연구소, 팹리스(Fabless) 업체,
대학 연구실 등에 납품한다. 시료는 주문이 들어오면 웨이퍼 공정 설비를 통해 제작되고, 검수를 거쳐
고객에게 출고된다.

기존에는 엑셀/메모장으로 주문을 관리하여 실수가 잦고 재고·공정 현황 파악이 어려웠다. 이를 해결하기
위해 콘솔 기반의 "반도체 시료 생산주문관리 시스템"을 개발한다.

- 시스템은 **콘솔 기반**으로 동작하며, 담당자가 직접 명령을 입력해 시료 등록과 주문 처리를 수행한다.
- **생산 라인**은 공장에서 시료 하나를 생산하는 설비 흐름으로, 하나의 생산 라인은 시료를 하나씩
  생산하며 주문이 들어온 시료에 대해서만 생산한다.

## 2. 핵심 도메인 모델

### 2.1 시료 (Sample)

시스템의 가장 기본이 되는 단위. 등록된 시료만 주문 가능. 상세: [sample-management.md](./features/sample-management.md)

| 속성 | 설명 |
| --- | --- |
| 시료 ID | 고유 식별자 |
| 이름 | 시료 이름 |
| 평균 생산시간 | 1개 생산에 소요되는 평균 시간 |
| 수율 | 정상적인 시료 / 총 생산 시료 (예: 100개 중 정상 90개 → 0.9) |
| 재고 수량 | 현재 보유 중인 정상 시료 수량 |

### 2.2 주문 (Order)

상세: [order.md](./features/order.md)

| 속성 | 설명 |
| --- | --- |
| 시료 ID | 주문 대상 시료 |
| 고객명 | 주문 고객 |
| 주문 수량 | 요청 수량 |
| 상태 | 아래 상태 흐름 참조 |

### 2.3 주문 상태 흐름

상세: [order-approval.md](./features/order-approval.md)

| 상태 | 의미 |
| --- | --- |
| RESERVED | 주문 접수 |
| REJECTED | 주문 거절 (정상 흐름 외 상태, 모니터링에서 제외) |
| PRODUCING | 주문 승인 완료 및 재고 부족으로 생산 중 |
| CONFIRMED | 주문 승인 완료 및 출고 대기 중 |
| RELEASE | 출고 완료 |

전이 규칙:
- `RESERVED` → (승인, 재고 충분) → `CONFIRMED`
- `RESERVED` → (승인, 재고 부족) → `PRODUCING` (생산 라인에 자동 등록)
- `RESERVED` → (거절) → `REJECTED`
- `PRODUCING` → (생산 완료) → `CONFIRMED`
- `CONFIRMED` → (출고 실행) → `RELEASE`

## 3. 기능별 요구사항

기능별 상세 요구사항은 `docs/features/` 하위 문서로 분리되어 있다.

| 기능 | 문서 |
| --- | --- |
| 메인 메뉴 | [features/mainmenu.md](./features/mainmenu.md) |
| 시료 관리 (등록 / 조회 / 검색) | [features/sample-management.md](./features/sample-management.md) |
| 시료 주문 (접수) | [features/order.md](./features/order.md) |
| 주문 승인/거절 | [features/order-approval.md](./features/order-approval.md) |
| 모니터링 | [features/monitoring.md](./features/monitoring.md) |
| 생산 라인 | [features/production-line.md](./features/production-line.md) |
| 출고 처리 | [features/release.md](./features/release.md) |
| 데이터 저장/불러오기 (영속성) | [features/persistence.md](./features/persistence.md) |

## 4. 비즈니스 규칙 요약

| 규칙 | 계산식 / 조건 | 상세 |
| --- | --- | --- |
| 수율 | 정상 시료 수 / 총 생산 시료 수 | [sample-management.md](./features/sample-management.md) |
| 실 생산량 | `ceil(부족분 / 수율)` | [production-line.md](./features/production-line.md) |
| 총 생산 시간 | `평균 생산시간 * 실 생산량` | [production-line.md](./features/production-line.md) |
| 생산 큐 스케줄링 | FIFO | [production-line.md](./features/production-line.md) |
| 재고 상태 판정 | 여유 / 부족 / 고갈 (0), 1개 단위로 판정 | [monitoring.md](./features/monitoring.md) |
| 재고 차감 시점 | **출고(RELEASE) 시점에만** 재고를 차감. 승인/생산 완료 시점에는 재고를 변경하지 않음 | [order-approval.md](./features/order-approval.md), [production-line.md](./features/production-line.md), [release.md](./features/release.md) |
| 재고 판단(충분/부족) 시점 | 주문 승인 시점에, 같은 시료의 선점 수량(다른 Producing/Confirmed 주문)을 제외한 가용 재고(`max(0, 재고-선점수량)`) 기준으로 판정 | [order-approval.md](./features/order-approval.md) |
| 재고 Update(생산 완료) 시점 | 생산 완료 시점에 실 생산량 전체를 재고에 한 번에 반영 (진행 중 실시간 반영 아님) | [production-line.md](./features/production-line.md) |
| 생산 실패 없음 | 수율은 실 생산량 계산에만 사용되며, 생산 시도한 수량은 모두 재고에 포함 | [production-line.md](./features/production-line.md) |
| 프로그램 재시작 시 생산 완료 처리 | 재시작 시점에 생산 완료 예정 시각이 이미 지났다면 그 시점에 재고 Update 반영 | [production-line.md](./features/production-line.md), [persistence.md](./features/persistence.md) |
| 데이터 저장 형식 | JSON 파일로 저장하고 시작 시 불러옴 | [persistence.md](./features/persistence.md) |

## 5. UI 관련 참고 사항

- 원본 문서의 UI 화면은 이해를 돕기 위한 예시이며, 실제 화면 구성은 자유롭게 결정 가능
  (콘솔 기반 UI 전제).

## 6. 개발 시 고려 필요 항목 (미결 사항)

각 기능 문서의 "미결 사항"/"결정 사항" 절에도 개별적으로 정리되어 있다.

- 시료 등록 시 초기 재고 수량 처리 방식 (0으로 시작하는지, 입력 가능한지 등)
  → 구현 시 정의 필요 (참고: [sample-management.md](./features/sample-management.md))

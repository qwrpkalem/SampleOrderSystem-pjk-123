# 주문 승인/거절

> 출처: `Semi.pdf` Chapter 02 - 주문 승인/거절

## 개요

접수된 주문(`RESERVED`) 목록을 확인하고, 특정 주문에 대하여 승인 혹은 거절을 처리한다.

## 주문 상태 흐름

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
- `PRODUCING` → (생산 완료) → `CONFIRMED` ([production-line.md](./production-line.md) 참조)
- `CONFIRMED` → (출고 실행) → `RELEASE` ([release.md](./release.md) 참조)

## 기능 요구사항

### 접수된 주문 목록

- `RESERVED` 상태의 주문 목록을 Display.

### 주문 승인

- 접수된 특정 주문에 대해 승인한다.
- 승인 시 재고 상황에 따라 2가지 방식으로 자동 처리된다.
  - 재고가 충분한 경우 → 주문을 즉시 `CONFIRMED` 상태로 전환
  - 재고가 부족한 경우 → 생산 라인에 자동으로 등록, 주문 상태를 `PRODUCING`으로 전환

### 주문 거절

- 접수된 특정 주문에 대해 거절한다.
- 즉시 `REJECTED` 상태로 전환.

## 미결 사항

- 재고 "충분/부족" 판정 기준(임계값)이 문서에 구체적으로 명시되지 않음 → 구현 시 정책 정의 필요
  (참고: [monitoring.md](./monitoring.md)의 재고 상태 판정과 일관되게 정의).

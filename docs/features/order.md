# 시료 주문 (접수)

> 출처: `Semi.pdf` Chapter 02 - 시료 주문

## 개요

고객이 시료를 요청하면 주문 담당자가 주문을 생성한다.

## 도메인 모델 - 주문 (Order)

| 속성 | 설명 |
| --- | --- |
| 시료 ID | 주문 대상 시료 |
| 고객명 | 주문 고객 |
| 주문 수량 | 요청 수량 |
| 상태 | [order-approval.md](./order-approval.md)의 주문 상태 흐름 참조 |

## 기능 요구사항

### 시료 예약

- 고객이 원하는 시료와 수량을 주문한다.
- 이 시점에서 주문 상태는 `RESERVED`.

### 예약 시 입력 값

- 시료 ID
- 고객명
- 주문 수량

## 결정 사항

- 등록되지 않은 시료 ID로 주문을 시도하면 거부한다 (`std::invalid_argument`).
  `sample-management.md`의 "시스템에 등록된 시료만 주문 가능" 요구사항을 그대로 강제한다.

## 관련 문서

- 승인/거절 처리: [order-approval.md](./order-approval.md)
- 승인 후 재고 부족 시 생산 처리: [production-line.md](./production-line.md)
- 승인 후 출고 처리: [release.md](./release.md)

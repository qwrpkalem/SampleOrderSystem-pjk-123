# 출고 처리

> 출처: `Semi.pdf` Chapter 02 - 출고 처리

## 개요

재고가 충분해진 `CONFIRMED` 주문에 대하여 출고를 처리한다.

## 기능 요구사항

- 특정 주문에 대해 출고를 실행한다.
- 주문 상태가 `RELEASE`로 전환된다.
- 출고 처리할 수 있는(`CONFIRMED` 상태) 주문이 하나도 없으면, 그 사실을 화면에 안내한다.

## 결정 사항

- `CONFIRMED`가 아닌 주문(예: `RESERVED`, `PRODUCING`, `REJECTED`, 이미 `RELEASE`된 주문)에
  대해 출고를 시도하면 거부한다 (`std::invalid_argument`).
- 출고가 정상 처리되면 해당 주문 수량만큼 시료 재고를 차감한다. 시스템 전체에서 재고가
  감소하는 시점은 **이 출고 시점뿐**이다 (승인 시점, 생산 완료 시점에는 재고가 변경되지
  않는다). 참고: [order-approval.md](./order-approval.md),
  [production-line.md](./production-line.md).

## 관련 문서

- 주문 상태 흐름 전체: [order-approval.md](./order-approval.md)

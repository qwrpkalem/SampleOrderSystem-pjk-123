# 출고 처리

> 출처: `Semi.pdf` Chapter 02 - 출고 처리

## 개요

재고가 충분해진 `CONFIRMED` 주문에 대하여 출고를 처리한다.

## 기능 요구사항

- 특정 주문에 대해 출고를 실행한다.
- 주문 상태가 `RELEASE`로 전환된다.

## 결정 사항

- `CONFIRMED`가 아닌 주문(예: `RESERVED`, `PRODUCING`, `REJECTED`, 이미 `RELEASE`된 주문)에
  대해 출고를 시도하면 거부한다 (`std::invalid_argument`).

## 관련 문서

- 주문 상태 흐름 전체: [order-approval.md](./order-approval.md)

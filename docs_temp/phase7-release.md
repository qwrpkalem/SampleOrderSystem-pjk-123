# Phase 7: 출고 처리

## 목표

`CONFIRMED` 상태 주문에 대해 출고를 실행하고 `RELEASE`로 전환한다.

## 참고 문서

- [release.md](../docs/features/release.md)

## 구현 범위

1. 특정 `CONFIRMED` 주문에 대해 출고 실행
2. 실행 시 주문 상태를 `RELEASE`로 전환

## 완료 기준 (체크리스트)

- [x] `CONFIRMED` 주문 출고 시 `RELEASE` 전환 테스트 작성 및 통과
- [x] `CONFIRMED`가 아닌 주문에 대한 출고 시도는 거부(예외)하도록 결정하고 테스트 작성 및 통과
  (참고: `release.md` 결정 사항)

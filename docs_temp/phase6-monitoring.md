# Phase 6: 모니터링

## 목표

주문량과 재고량을 상태별로 한눈에 파악할 수 있는 조회 기능을 제공한다.

## 참고 문서

- [monitoring.md](../docs/features/monitoring.md)

## 구현 범위

1. **주문량 확인**: 상태별(`RESERVED`/`CONFIRMED`/`PRODUCING`/`RELEASE`) 목록/건수 확인.
   `REJECTED`는 집계에서 제외
2. **재고량 확인**: 시료별 재고 수량과 여유/부족/고갈 상태 표기
   - 여유: 주문 대비 재고가 1개라도 많음(같으면 여유)
   - 부족: 주문 대비 재고가 1개라도 부족
   - 고갈: 수량이 0 (수요와 무관하게 우선)
   - "주문"은 해당 시료의 `RESERVED` 주문 수량 합으로 결정 (참고: `monitoring.md` 결정 사항)

## 완료 기준 (체크리스트)

- [x] 상태별 주문 집계에서 `REJECTED`가 제외되는지 테스트 작성 및 통과
- [x] 재고 여유/부족/고갈 판정(1개 단위 경계값 포함) 테스트 작성 및 통과
- [x] 시료별 "주문 대비" 수요(`RESERVED` 주문 수량 합) 계산 테스트 작성 및 통과

## 고려 사항

- 실제 콘솔 화면에 상태별 주문 목록/재고 현황을 표시하는 부분은 Phase 8(메인 메뉴 통합)에서
  `OrderStatusSummary`/`evaluateStockLevel`/`totalDemandForSample`을 사용해 구현한다.

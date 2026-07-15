# Phase 4: 주문 승인/거절

## 목표

접수된(`RESERVED`) 주문을 승인 또는 거절하고, 승인 시 재고 상황에 따라 자동으로 상태를 전환한다.

## 참고 문서

- [order-approval.md](../docs/features/order-approval.md)

## 구현 범위

1. **접수된 주문 목록**: `RESERVED` 상태 주문 목록 표시. `OrderBook::list()`가 전체 주문을
   반환하므로, 상태별 필터링(Display)은 호출 측(Phase 8 메인 메뉴)에서 수행한다 — 콘솔 앱은
   단일 스레드로 동작하므로 별도의 "RESERVED만 조회" API 없이도 충분하다고 판단했다.
2. **주문 승인**
   - 승인 시점의 재고 수량을 기준으로 판정 ([order-approval.md](../docs/features/order-approval.md)
     결정 사항 참고)
   - 재고 충분 → `CONFIRMED` + 즉시 재고 차감 (결정 완료, 참고: `order-approval.md`)
   - 재고 부족 → `PRODUCING` (재고는 차감하지 않음). 생산 큐 등록 자체는 Phase 5에서 큐
     도메인 모델과 함께 구현한다.
3. **주문 거절**: `REJECTED`로 즉시 전환

## 완료 기준 (체크리스트)

- [x] 재고 충분 시 승인 → `CONFIRMED` 전환 + 재고 차감 테스트 작성 및 통과
- [x] 재고 부족 시 승인 → `PRODUCING` 전환(재고 유지) 테스트 작성 및 통과. 생산 큐 등록은
  Phase 5로 이연 (`docs_temp/phase5-production-line.md` 참고)
- [x] 거절 시 `REJECTED` 전환 테스트 작성 및 통과
- [x] 재고 판정이 "승인 시점" 기준으로 이루어지는지 검증 — 단일 스레드 콘솔 앱이라 승인 처리
  도중 재고가 바뀌는 동시성 시나리오는 해당하지 않으며, `approve()` 호출 시점의 재고를
  동기적으로 읽어 판정하는 것으로 충분하다고 판단

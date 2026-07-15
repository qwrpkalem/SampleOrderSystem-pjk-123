# Phase 5: 생산 라인

## 목표

`PRODUCING` 주문의 부족분을 생산하고, 생산 완료 시 재고를 반영하며 주문을 `CONFIRMED`로 전환한다.
프로그램이 중간에 종료되어도 재시작 시 생산 완료 여부를 올바르게 판단한다.

## 참고 문서

- [production-line.md](../docs/features/production-line.md)
- [persistence.md](../docs/features/persistence.md)

## 구현 범위

1. **생산 계산** (완료, 참고: `production-line.md`)
   - 실 생산량 = `ceil(부족분 / 수율)`
   - 총 생산 시간 = `평균 생산시간 * 실 생산량` (평균 생산시간의 단위는 분(minute)으로 결정)
2. **생산 큐**: FIFO 스케줄링으로 대기 중인 생산 작업 관리 (완료). `OrderBook::approve()`가
   재고 부족 시 부족분/생산량/완료 예정 시각을 계산해 `ProductionQueue`에 등록하도록 연동
   완료 (참고: `docs_temp/phase4-order-approval.md`).
3. **생산 완료 처리** (완료). `ProductionLine::processCompletedJobs()`가 큐 맨 앞부터 완료
   시각이 지난 작업을 순서대로 꺼내 재고를 증가시키고(`SampleCatalog::increaseStock`) 해당
   주문을 `Confirmed`로 전환(`OrderBook::completeProduction`)한다. 아직 완료되지 않은
   작업을 만나면 그 뒤는 검사하지 않고 중단한다 (FIFO/순차 처리이므로 앞이 안 끝났으면 뒤도
   당연히 미완료).
   - 재고 Update는 생산이 진행되는 동안이 아니라 완료된 시점에 한 번에 반영
   - 수율은 실 생산량 계산에만 사용되고, 생산 시도한 수량은 모두 재고에 포함 (생산 실패로 인한
     폐기 없음)
   - 완료 시 해당 주문을 `PRODUCING` → `CONFIRMED`로 전환
4. **재시작 시 복구** (완료). `OrderBook::restoreOrders()`/`ProductionQueue::restore()`로
   저장된 주문/생산 큐를 가공 없이 그대로 복원한 뒤 `ProductionLine::processCompletedJobs()`를
   호출하면, 프로그램이 꺼져 있던 동안 이미 완료 시각이 지난 생산 건이 즉시 재고 Update와
   상태 전환에 반영된다. `SampleCatalog`는 별도 복원 API 없이 `registerSample()` 반복
   호출로 충분하다고 판단했다 (저장된 시료 ID는 항상 유일).
   - 실제 콘솔 앱 시작 시 `Repository::load()` → 각 컴포넌트 복원 →
     `processCompletedJobs()` 호출을 연결하는 지점은 Phase 8(메인 메뉴 통합)에서 `main()`에
     연결한다.
5. **생산 현황/대기 목록 표기**: 현재 생산 중인 시료 정보와 대기 큐 목록 출력. 데이터는 이미
   `ProductionQueue::list()`로 조회 가능하므로, 콘솔 화면에 실제로 표시하는 부분은
   Phase 8(메인 메뉴/콘솔 UI)에서 함께 다룬다.
6. **생산 큐 영속성** (완료). `SystemState`에 `productionJobs` 필드를 추가하고
   `ProductionJobJson`으로 직렬화(완료 시각은 epoch 초 단위)해 `Repository::save/load`가
   생산 큐까지 저장/복원하도록 확장했다. Phase 1에서는 생산 큐 도메인 타입이 아직 없어
   이연되었던 부분이다 (참고: `docs_temp/phase1-foundation.md`).

## 완료 기준 (체크리스트)

- [x] 실 생산량/총 생산 시간 계산식 테스트 작성 및 통과
- [x] FIFO 큐 동작(투입 순서대로 처리) 테스트 작성 및 통과, `OrderBook.approve()` 연동 완료
- [x] 생산 완료 시 재고가 정확히 실 생산량만큼 한 번에 증가하는지(중간에 반영되지 않는지) 테스트
  작성 및 통과
- [x] 프로그램 재시작 시나리오: 생산 완료 예정 시각이 이미 지난 상태로 재시작했을 때 재고/주문
  상태가 즉시 갱신되는지 테스트 작성 및 통과
- [ ] 생산 현황/대기 큐 조회 기능 콘솔 출력 — Phase 8(메인 메뉴 통합)에서 진행

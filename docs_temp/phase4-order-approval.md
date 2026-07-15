# Phase 4: 주문 승인/거절

## 목표

접수된(`RESERVED`) 주문을 승인 또는 거절하고, 승인 시 재고 상황에 따라 자동으로 상태를 전환한다.

## 참고 문서

- [order-approval.md](../docs/features/order-approval.md)

## 구현 범위

1. **접수된 주문 목록**: `RESERVED` 상태 주문 목록 표시
2. **주문 승인**
   - 승인 시점의 재고 수량을 기준으로 판정 ([order-approval.md](../docs/features/order-approval.md)
     결정 사항 참고)
   - 재고 충분 → `CONFIRMED`
   - 재고 부족 → `PRODUCING` + 생산 큐에 등록 (Phase 5 생산 라인과 연동)
3. **주문 거절**: `REJECTED`로 즉시 전환

## 완료 기준 (체크리스트)

- [ ] 재고 충분 시 승인 → `CONFIRMED` 전환 테스트 작성 및 통과
- [ ] 재고 부족 시 승인 → `PRODUCING` 전환 + 생산 큐 등록 테스트 작성 및 통과
- [ ] 거절 시 `REJECTED` 전환 테스트 작성 및 통과
- [ ] 재고 판정이 "승인 시점" 기준으로 이루어지는지(승인 처리 도중 재고가 바뀌어도 승인 시작
  시점 값을 사용하는지 등) 검증하는 테스트 작성 및 통과

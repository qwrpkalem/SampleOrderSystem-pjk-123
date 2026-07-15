# Phase 1: 기반 구조 (도메인 모델 + JSON 영속성)

## 목표

이후 모든 기능이 공통으로 사용할 도메인 모델과 저장/불러오기 계층을 구현한다.

## 참고 문서

- [sample-management.md](../docs/features/sample-management.md) (도메인 모델)
- [order.md](../docs/features/order.md), [order-approval.md](../docs/features/order-approval.md)
  (도메인 모델, 주문 상태)
- [persistence.md](../docs/features/persistence.md) (JSON 저장/불러오기)

## 구현 범위

1. **도메인 모델**
   - `Sample`: 시료 ID, 이름, 평균 생산시간, 수율, 재고 수량
   - `Order`: 시료 ID, 고객명, 주문 수량, 상태(`RESERVED`/`REJECTED`/`PRODUCING`/`CONFIRMED`/
     `RELEASE`)
   - 생산 큐 항목: 주문 참조, 생산 완료 예정 시각 등 (Phase 5에서 상세화되지만 저장 스키마에는
     Phase 1에서 자리를 마련해둔다)
2. **JSON 직렬화/역직렬화**
   - Sample 목록, Order 목록, 생산 큐(진행 중인 생산 정보 포함)를 JSON으로 저장/불러오기
3. **저장소(Repository) 계층**
   - 파일 경로 지정, 저장, 불러오기 기본 동작
   - 파일이 없을 때(최초 실행) 빈 상태로 시작

## 완료 기준 (체크리스트)

- [ ] Sample, Order 모델 단위 테스트 작성 및 통과
- [ ] JSON으로 저장한 데이터를 다시 불러왔을 때 원본과 동일한지 검증하는 테스트(round-trip) 작성
  및 통과
- [ ] 저장 파일이 없는 최초 실행 상황에 대한 테스트 작성 및 통과
- [ ] gtest 빌드/실행 확인 (개발 환경 SetUp 커밋에서 이미 검증됨)

# 메인 메뉴

> 출처: `Semi.pdf` Chapter 02 - 메인 메뉴

## 개요

기능별 선택 화면을 표시하고, 전체 시료에 대한 요약 정보를 확인할 수 있는 시작 화면.

## 기능 요구사항

- 기능별 선택 화면을 Display
- 전체 시료에 대한 요약 정보를 확인
- 하위 메뉴로 진입 가능해야 한다.

| 하위 메뉴 | 상세 문서 |
| --- | --- |
| 시료 관리 | [sample-management.md](./sample-management.md) |
| 주문 (접수 / 승인 / 거절) | [order.md](./order.md), [order-approval.md](./order-approval.md) |
| 모니터링 | [monitoring.md](./monitoring.md) |
| 출고 처리 | [release.md](./release.md) |
| 생산 라인 | [production-line.md](./production-line.md) |

## UI 관련 참고 사항

- 원본 문서의 UI 화면은 이해를 돕기 위한 예시이며, 실제 화면 구성은 자유롭게 결정 가능
  (콘솔 기반 UI 전제).

## 미결 사항

- 전체 시료 "요약 정보"에 어떤 항목(재고 총량, 주문 건수 등)을 포함할지 구체 명시 없음 →
  구현 시 정의 필요.

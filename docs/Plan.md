# Json PoC 전체 계획 (Data Persistence)

이 문서는 `DataPersistence-shp-260715` (`Json/`) PoC를 여러 phase로 나눈 전체
설계다. 각 phase는 `../CLAUDE.md`(Json PoC 목표/설계 방향/TDD 진행 방식)를
근거로 하며, TDD(Red → Green → Refactor)로 진행한다.

이 PoC에서 확정하는 **JSON 스키마와 Repository 인터페이스**는 `Monitor/`,
`Dummy/`, `Main/`이 공유하는 데이터 포맷의 기준이 되므로, phase가 끝날 때마다
스키마에 변경이 생기면 이 문서와 각 phase 문서를 함께 갱신한다.

## Phase 목록 및 순서(의존관계)

```
Phase 1 (엔티티 + 직렬화 round-trip)
   └─▶ Phase 2 (빈 상태 / 파일 없음 처리)
          └─▶ Phase 3 (Create/Read, 같은 인스턴스)
                 └─▶ Phase 4 (영속성 핵심: 재시작 시뮬레이션 재로딩)
                        └─▶ Phase 5 (Update/Delete + 손상 JSON 정책)
```

각 phase는 이전 phase의 테스트가 모두 통과한 상태에서만 시작한다. Phase 1의
엔티티/필드명이 이후 모든 phase와 다른 PoC의 기준이 되므로 가장 먼저 확정한다.

## Phase 요약표

| Phase | 이름 | 목표 | 완료 기준(DoD) | 세부 문서 |
|---|---|---|---|---|
| 1 | 엔티티 & 직렬화 | Sample/Order 구조체 정의, JSON 변환(to/from) round-trip 검증 | 모든 필드가 직렬화→역직렬화 후 원본과 동일함을 gtest로 확인 | [phase1-serialization.md](./phase1-serialization.md) |
| 2 | 빈 상태 / 파일 없음 | 최초 실행(파일 없음) 시 빈 컬렉션 반환, 빈 컬렉션 저장 형식 확정 | 파일이 없어도 예외 없이 빈 목록 반환, 저장 후 빈 배열 JSON 생성 확인 | [phase2-empty-state.md](./phase2-empty-state.md) |
| 3 | Create/Read (인메모리 왕복) | Repository에 Add 후 같은 인스턴스에서 조회/저장 파일 생성 확인 | Add→GetAll/FindById 결과 일치, 파일이 디스크에 생성됨 | [phase3-create-read.md](./phase3-create-read.md) |
| 4 | 영속성 핵심(재시작 재현) | 저장 후 **새 Repository 인스턴스**로 재로딩 시 동일 데이터 확인 | 새 인스턴스 로딩 결과가 저장 전 데이터와 완전히 일치 | [phase4-persistence-reload.md](./phase4-persistence-reload.md) |
| 5 | Update/Delete + 손상 JSON 정책 | Update/Delete가 파일에 반영, 손상된 JSON 파일 처리 정책 확정 | Update/Delete 후 재로딩해도 반영됨, 손상 파일에 대한 정의된 동작(예외 vs 빈 값) 테스트로 고정 | [phase5-update-delete-corruption.md](./phase5-update-delete-corruption.md) |

## 공통 스키마 개요 (모든 phase에서 공유, 상세는 phase1 문서 참고)

- 저장 경로: `data/samples.json`, `data/orders.json` (실행 파일 기준 상대경로가
  아니라, Repository 생성 시 주입되는 경로. 기본값은 `data/` 하위)
- Sample: `sampleId`, `name`, `avgProductionTime`, `yield`, `stock`
- Order: `orderId`, `sampleId`, `customerName`, `quantity`, `status`, `createdAt`
- Order.status는 문자열 enum: `RESERVED`, `REJECTED`, `PRODUCING`, `CONFIRMED`,
  `RELEASE`
- 필드명은 camelCase, 최상위는 `{"samples": [...]}` / `{"orders": [...]}` 형태의
  배열 래핑 객체

## 범위 밖

- 콘솔 메뉴/화면 흐름 (다른 PoC/Main의 책임)
- 동시성 제어 (단일 프로세스 가정)
- 생산 큐/재고 계산 로직 자체 (Repository는 저장/조회만 담당, 계산 로직은
  `Main`의 서비스 계층 책임)

# Phase 4: 영속성 핵심 — 재시작(새 Repository 인스턴스) 재로딩 검증

[← 전체 계획](./Plan.md) | 이전: [Phase 3 — Create/Read](./phase3-create-read.md) | 다음: [Phase 5 — Update/Delete + 손상 JSON 정책](./phase5-update-delete-corruption.md)

## 목표

이 PoC의 존재 이유인 "애플리케이션을 재시작해도 데이터가 유지되는가"를
직접 증명한다. 테스트에서 프로세스 재시작을 흉내 내기 위해 **동일 파일
경로를 가리키는 새 Repository 인스턴스**를 생성해 데이터가 그대로 로딩되는지
확인한다.

## 작성할 테스트 목록 (Red 단계)

1. `SampleRepository_Restart_ReloadsPreviouslySavedSamples` — 인스턴스 A로
   Sample 여러 건 `Add` → 인스턴스 A를 스코프 밖으로 내보냄(소멸) → 같은 경로로
   인스턴스 B 생성 → `B.GetAll()`이 A가 저장한 것과 필드 단위로 완전히 동일
2. `OrderRepository_Restart_ReloadsPreviouslySavedOrders` — Order도 동일
   (Status enum, CreatedAt 문자열까지 포함해 동일성 확인)
3. `SampleRepository_Restart_PreservesInsertionOrder` — 재로딩 후에도
   추가했던 순서가 유지되는지 확인 (배열 순서 보존 정책 고정)
4. `SampleRepository_Restart_MultipleReloadsAreIdempotent` — B에서 다시
   `GetAll()`을 여러 번 호출해도(추가 저장 없이) 항상 같은 결과 (side-effect 없음)
5. `SampleAndOrderRepository_Restart_IndependentFiles` — Sample 파일과 Order
   파일이 서로 영향을 주지 않고 독립적으로 재로딩됨을 동시 확인
6. `SampleRepository_Restart_AfterMultipleAddsAcrossInstances` — 인스턴스 A가
   1건 저장 후 소멸, 인스턴스 B가 그 파일을 로드하고 1건 더 추가 후 소멸,
   인스턴스 C가 로드했을 때 총 2건이 모두 존재 (여러 번의 재시작 누적 시나리오)

## 구현 관련 참고

새로운 클래스 추가는 없다. Phase 2~3에서 구현한 `SampleRepository`/
`OrderRepository`의 생성자(파일 경로를 받아 `LoadFromFile()`을 초기 상태로
사용하는 방식)가 이미 이 요구를 만족하도록 설계되어 있어야 한다. 이 phase는
그 설계가 실제로 "재시작"에 해당하는 시나리오에서 깨지지 않는지를 통합 테스트
수준에서 검증하는 것이 핵심이다.

만약 Phase 2/3 구현이 인메모리 캐시만 사용하고 파일을 지연 로딩하지 않는
구조라면, 이 phase에서 다음을 재확인한다:
- 생성자 시점에 파일이 있으면 즉시 `LoadFromFile()`을 호출해 내부 캐시를
  채우는지
- `GetAll()`/`FindById()`가 매 호출 파일을 다시 읽는 게 아니라 생성자에서
  로드한 캐시를 사용하는지(성능/일관성 정책을 문서화)

## 완료 기준

- 위 6개 테스트 모두 통과
- "저장 → 새 인스턴스 → 재로딩 → 동일 데이터 확인"이라는 형태가 Sample과
  Order 양쪽에 대해 명시적으로 존재함 (이 PoC의 핵심 증거)
- 순서 보존, 다중 재시작 누적 등 엣지 케이스도 커버됨

## 다음 phase와의 연결점

Phase 5는 이 재로딩 메커니즘 위에서 Update/Delete가 파일에 반영되고, 그 다음
재로딩에서도 반영된 상태가 유지되는지 검증한다. 또한 재로딩 도중 파일이
손상되어 있으면 어떻게 동작해야 하는지(이 phase에서는 정상 파일만 다룸)를
Phase 5에서 다룬다.

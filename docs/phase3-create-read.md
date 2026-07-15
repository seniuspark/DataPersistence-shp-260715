# Phase 3: Create/Read — 저장 후 같은 Repository 인스턴스에서 조회

[← 전체 계획](./Plan.md) | 이전: [Phase 2 — 빈 상태](./phase2-empty-state.md) | 다음: [Phase 4 — 영속성 핵심(재시작 재현)](./phase4-persistence-reload.md)

## 목표

Repository에 엔티티를 추가(Create)하면 같은 인스턴스 내에서 즉시 조회(Read)할
수 있고, 그 시점에 디스크 파일에도 반영되는지 확인한다. 아직 "재시작 후에도
유지되는지"는 검증하지 않는다(그건 Phase 4의 책임) — 이 phase는 인메모리
상태와 파일 쓰기 그 자체에 집중한다.

## 작성할 테스트 목록 (Red 단계)

1. `SampleRepository_Add_ThenGetAll_ContainsAddedSample` — `Add(sample)` 후
   `GetAll()`에 해당 sample이 포함됨
2. `SampleRepository_Add_ThenFindById_ReturnsSample` — `FindById`로 방금 추가한
   항목 조회 가능
3. `SampleRepository_Add_DuplicateSampleId_PolicyDefined` — 이미 존재하는
   `sampleId`로 `Add` 호출 시 정책 확정(예: 예외 던짐 또는 덮어쓰기) — 정책을
   정하고 테스트로 고정
4. `SampleRepository_Add_CreatesFileOnDisk` — `Add` 호출 후 지정된 파일 경로에
   실제 JSON 파일이 생성됨(파일 존재 여부 + 내용에 sampleId 포함 확인)
5. `SampleRepository_Add_CreatesMissingParentDirectory` — `data/` 디렉토리가
   없는 상태에서도 `Add` 시 디렉토리를 생성하고 파일을 씀
6. `OrderRepository_Add_ThenGetAll_ContainsAddedOrder` — Order도 동일하게 검증
7. `OrderRepository_Add_ThenFindById_ReturnsOrder`
8. `SampleRepository_GetAll_ReturnsCopiesNotInternalReference` — `GetAll()`이
   반환한 벡터를 수정해도 Repository 내부 상태에 영향 없음 (값 시맨틱 확인)

## 구현해야 할 클래스/함수/파일

- `SampleRepository`에 메서드 추가:
  - `void Add(const Sample& sample);` — 내부 캐시에 추가 + 즉시 파일에 저장
  - `void SaveAll(const std::vector<Sample>& samples);` (내부에서 사용할 수도
    있는 저수준 저장 함수, 필요 시 private)
- `OrderRepository`에 동일하게 `Add(const Order&)` 추가
- 파일 쓰기는 "전체 컬렉션을 매번 통째로 직렬화하여 덮어쓰기"하는 단순한
  전략을 채택 (동시성 미고려, PoC 범위에 부합)
- 디렉토리 생성: `std::filesystem::create_directories(path.parent_path())`

## 완료 기준

- 위 8개 테스트 모두 통과
- 중복 ID 처리 정책이 문서와 테스트에 일관되게 반영됨
- 파일 쓰기가 항상 부모 디렉토리 생성을 보장함

## 다음 phase와의 연결점

Phase 4는 이 phase에서 만든 `Add`로 저장한 파일을, **새로운 Repository
인스턴스**를 만들어 다시 읽었을 때 동일한 데이터가 나오는지 검증한다. 이
phase의 파일 쓰기 포맷(전체 덮어쓰기 방식)이 Phase 4의 재로딩 테스트 대상이
된다.

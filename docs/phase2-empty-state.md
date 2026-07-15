# Phase 2: 빈 상태 / 파일 없음(최초 실행) 처리

[← 전체 계획](./Plan.md) | 이전: [Phase 1 — 엔티티 & 직렬화](./phase1-serialization.md) | 다음: [Phase 3 — Create/Read](./phase3-create-read.md)

## 목표

Repository가 아직 한 번도 저장되지 않은 상태(파일이 존재하지 않음)에서
조회할 때 예외 없이 빈 컬렉션을 반환하도록 만든다. 이는 앱을 처음 실행하는
시나리오(최초 실행)를 대표한다.

## 작성할 테스트 목록 (Red 단계)

1. `SampleRepository_FileDoesNotExist_GetAllReturnsEmpty` — 존재하지 않는
   경로로 `SampleRepository` 생성 후 `GetAll()`이 빈 벡터를 반환
2. `OrderRepository_FileDoesNotExist_GetAllReturnsEmpty` — Order도 동일
3. `SampleRepository_FileDoesNotExist_FindByIdReturnsNullopt` — 없는 파일
   상태에서 `FindById("S-001")`이 `std::optional`의 empty를 반환
4. `SampleRepository_ConstructingDoesNotCreateFileImmediately` — Repository
   생성만으로는 파일을 생성하지 않음(부작용 없음)을 확인 — 실제 파일 생성은
   최초 저장(Save/Add) 시점에만 일어나야 함
5. `SampleRepository_DirectoryDoesNotExist_GetAllReturnsEmpty` — `data/` 디렉토리
   자체가 없는 경우에도 예외 없이 빈 목록 반환 (디렉토리 생성은 저장 시점에)

## 구현해야 할 클래스/함수/파일

- `Project1/Project1/SampleRepository.h/.cpp`
  - 생성자: `explicit SampleRepository(std::filesystem::path filePath);`
  - `std::vector<Sample> GetAll() const;`
  - `std::optional<Sample> FindById(const std::string& sampleId) const;`
  - 내부 private 헬퍼: `std::vector<Sample> LoadFromFile() const;` — 파일이
    없으면 빈 벡터 반환(예외 던지지 않음)
- `Project1/Project1/OrderRepository.h/.cpp` — 위와 동일한 형태로 Order용
- 기본 저장 경로 상수: `Sample -> "data/samples.json"`, `Order -> "data/orders.json"`
  (다만 테스트에서는 매 테스트마다 임시 디렉토리 경로를 주입해 격리)

## 완료 기준

- 위 5개 테스트 모두 통과
- 파일이 없을 때 예외를 던지지 않고 빈 값(빈 벡터/`std::nullopt`)을 반환하는
  정책이 코드 주석이 아니라 테스트로 고정됨
- Repository 생성자는 side-effect(파일/디렉토리 생성)가 없음이 테스트로 보장됨

## 다음 phase와의 연결점

Phase 3에서는 이 Repository에 `Add`/`Update`/`Save` 계열 메서드를 추가해
최초로 파일에 데이터를 쓰는 동작을 구현한다. 이 phase에서 만든 "파일 없음 →
빈 목록" 경로는 Phase 3 이후에도 "Add 이전 상태"의 사전 조건으로 재사용된다.

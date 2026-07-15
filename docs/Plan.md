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

## PoC 완료 요약 (Phase 5 종료 = Json PoC 전체 완료)

Phase 1~5 전체가 TDD(Red → Green → Refactor)로 완료되었고, `Project1Tests.exe`
기준 **51/51 테스트 통과**했다 (Phase 1~4: 36개 + Phase 5: 15개[Update/Delete 8개,
손상 JSON 정책 7개]).

### 확정된 JSON 스키마 (Monitor/Dummy/Main 공유 기준)

- 저장 경로: `SampleRepository`/`OrderRepository` 생성자에 주입되는
  `std::filesystem::path` (기본값 예시 `data/samples.json`, `data/orders.json`)
- `data/samples.json`:
  ```json
  { "samples": [
      { "sampleId": "S-001", "name": "...", "avgProductionTime": 12.5,
        "yield": 0.95, "stock": 100 }
  ] }
  ```
- `data/orders.json`:
  ```json
  { "orders": [
      { "orderId": "ORD-20260416-0043", "sampleId": "S-001",
        "customerName": "...", "quantity": 10, "status": "RESERVED",
        "createdAt": "2026-04-16T09:00:00" }
  ] }
  ```
- `status`는 문자열 enum: `RESERVED`, `REJECTED`, `PRODUCING`, `CONFIRMED`,
  `RELEASE` (`Order.h`의 `ToString`/`ParseOrderStatus` 참고)
- 필드명은 camelCase, 최상위는 배열을 감싸는 객체(`{"samples": [...]}` /
  `{"orders": [...]}`)

### 확정된 Repository 인터페이스

`SampleRepository`, `OrderRepository` 모두 동일한 형태:

```cpp
std::vector<T> GetAll() const;
std::optional<T> FindById(const std::string& id) const;
void Add(const T& entity);              // 중복 ID면 std::invalid_argument
bool Update(const T& entity);           // 성공 true / 대상 없음 false
bool Delete(const std::string& id);     // 성공 true / 대상 없음 false
```

### 최종 확정 정책

| 상황 | 정책 |
|---|---|
| 파일 없음(최초 실행) | 빈 컬렉션 반환, 파일은 `Add`/`Update`/`Delete` 최초 성공 시 생성 |
| 파일 내용이 빈 문자열(0바이트) | 빈 컬렉션 반환 |
| JSON 문법 오류(파싱 실패) | 빈 컬렉션 반환, **원본 손상 파일은 덮어쓰지 않고 보존**. 이후 `Add`/`Update`/`Delete`가 성공하면 그 시점부터 정상 파일로 덮어써짐 |
| 최상위 구조가 기대와 다름(배열 자체이거나 `samples`/`orders` 키 없음) | 빈 컬렉션 반환 |
| 배열 안 개별 원소의 필수 필드 누락/파싱 실패 | 해당 원소만 건너뛰고(skip) `std::cerr`에 경고 출력, 나머지 원소는 정상 반영 |
| `Add`에 중복 ID | `std::invalid_argument` 예외 |
| `Update`/`Delete`에 존재하지 않는 ID | 예외 없이 `false` 반환 |

### CRUD 완전성 체크 (CLAUDE.md 목표 대비)

- [x] Sample: Create(Phase 3)/Read(Phase 2~4)/Update(Phase 5)/Delete(Phase 5) 모두 테스트로 검증됨
- [x] Order: Create/Read/Update/Delete 모두 테스트로 검증됨 (Order의 Delete는
      실제 업무 요구사항엔 없을 수 있으나 Repository 완전성을 위해 포함, CLAUDE.md 방침 그대로)
- [x] 재시작 시뮬레이션(새 Repository 인스턴스로 재로딩) 테스트 통과 (Phase 4,
      Phase 5의 Update/Delete 테스트도 모두 재로딩까지 검증)
- [x] 손상/누락/빈 파일에 대한 동작이 정의되고 테스트로 고정됨 (Phase 5)
- [x] JSON round-trip(직렬화→역직렬화) 검증됨 (Phase 1)

이로써 `Json/CLAUDE.md`가 요구한 목표(재시작해도 유지되는 저장 구조, Sample/Order
CRUD 완전성, round-trip 검증)를 모두 충족했다고 판단한다. **Json PoC(Phase 1~5) 완료.**

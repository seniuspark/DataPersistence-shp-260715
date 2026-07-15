# DataPersistence-shp-260715

반도체 시료 생산주문관리 시스템의 **데이터 영속성(JSON) PoC**. 애플리케이션을
재시작해도(새 프로세스로 다시 실행해도) 시료(Sample)/주문(Order) 데이터가
JSON 파일로 유지되는지를 Repository 패턴 + TDD로 검증한다.

이 저장소는 상위 저장소(`producing/`)의 `Json/` 디렉토리가 독립된 Git
Repository로 분리된 것이며, 여기서 확정한 JSON 스키마와 Repository
인터페이스는 `Monitor/`, `Dummy/`, `Main/` PoC가 공유하는 데이터 포맷의
기준이 된다.

## 디렉토리 구조

```
Json/
├── CLAUDE.md                       # 이 PoC의 목표/설계 방향/TDD 진행 방식
├── docs/
│   ├── Plan.md                     # Phase 1~5 전체 계획 및 완료 요약(확정 스키마 포함)
│   ├── phase1-serialization.md
│   ├── phase2-empty-state.md
│   ├── phase3-create-read.md
│   ├── phase4-persistence-reload.md
│   └── phase5-update-delete-corruption.md
└── Project1/                       # Visual Studio 솔루션(C++20)
    ├── Project1.slnx
    ├── Project1/                   # 콘솔 exe 프로젝트 (Repository/엔티티 헤더 + smoke test main)
    │   ├── Project1.vcxproj
    │   ├── Sample.h                # Sample 엔티티 + ToJson/SampleFromJson
    │   ├── Order.h                 # Order 엔티티 + OrderStatus enum + ToJson/OrderFromJson
    │   ├── SampleRepository.h      # 시료 JSON 파일 CRUD
    │   ├── OrderRepository.h       # 주문 JSON 파일 CRUD
    │   └── main.cpp                # 엔티티/직렬화 smoke test용 최소 main (콘솔 메뉴 없음)
    ├── Project1Tests/               # gtest 테스트 프로젝트
    │   ├── Project1Tests.vcxproj
    │   ├── SampleSerializationTests.cpp
    │   ├── OrderSerializationTests.cpp
    │   ├── SampleRepositoryTests.cpp
    │   ├── OrderRepositoryTests.cpp
    │   ├── PersistenceReloadTests.cpp
    │   ├── UpdateDeleteTests.cpp
    │   └── CorruptedJsonTests.cpp
    └── packages/                    # NuGet 패키지 (gtest, nlohmann-json)
```

엔티티/Repository 구현은 헤더 온리(`.h`)로 되어 있으며, `Project1` 콘솔
프로젝트와 `Project1Tests` 테스트 프로젝트가 동일한 소스 파일을 공유(참조)한다.

## 확정된 JSON 스키마

저장 경로는 `SampleRepository`/`OrderRepository` 생성자에 `std::filesystem::path`로
주입한다(하드코딩된 상대경로가 아님). 기본 사용 예시는 `data/samples.json`,
`data/orders.json`이다.

필드명은 camelCase, 최상위는 배열을 감싸는 객체(`{"samples": [...]}` /
`{"orders": [...]}`) 형태다.

### `data/samples.json`

```json
{
  "samples": [
    {
      "sampleId": "S-001",
      "name": "웨이퍼 A",
      "avgProductionTime": 12.5,
      "yield": 0.95,
      "stock": 100
    }
  ]
}
```

`Sample` 구조체(`Sample.h`) 필드: `sampleId`(string), `name`(string),
`avgProductionTime`(double), `yield`(double), `stock`(int).

### `data/orders.json`

```json
{
  "orders": [
    {
      "orderId": "ORD-20260416-0043",
      "sampleId": "S-001",
      "customerName": "고객사A",
      "quantity": 50,
      "status": "RESERVED",
      "createdAt": "2026-04-16T09:00:00"
    }
  ]
}
```

`Order` 구조체(`Order.h`) 필드: `orderId`(string), `sampleId`(string),
`customerName`(string), `quantity`(int), `status`(`OrderStatus` enum,
JSON에서는 문자열), `createdAt`(string, ISO 8601 텍스트를 그대로 저장하며
파싱하지 않음).

`status`는 문자열 enum이며 다음 5개 값만 허용된다(`Order.h`의 `ToString`/
`ParseOrderStatus` 참고): `RESERVED`, `REJECTED`, `PRODUCING`, `CONFIRMED`,
`RELEASE`. 이 외의 문자열은 `ParseOrderStatus`에서 `std::invalid_argument`를
던지며, `LoadFromFile` 내부에서는 이 예외를 잡아 해당 항목만 스킵한다.

## Repository API

`SampleRepository`(`SampleRepository.h`)와 `OrderRepository`
(`OrderRepository.h`)는 동일한 형태의 API를 제공한다.

```cpp
explicit SampleRepository(std::filesystem::path filePath);

std::vector<Sample> GetAll() const;
std::optional<Sample> FindById(const std::string& sampleId) const;
void Add(const Sample& sample);            // 중복 sampleId → std::invalid_argument
bool Update(const Sample& sample);          // 성공 true / 대상 없음 false
bool Delete(const std::string& sampleId);   // 성공 true / 대상 없음 false
```

(`OrderRepository`도 `orderId` 기준으로 동일한 시그니처를 가진다.)

모든 메서드는 매 호출마다 파일 전체를 읽고(`LoadFromFile`) 쓰는(`SaveAll`)
단순한 구현이다(캐시 없음, 동시성 고려 없음 — 단일 프로세스 콘솔 앱 전제).

### 동작 정책 (테스트로 고정됨)

| 상황 | 정책 | 근거 테스트 |
|---|---|---|
| 파일이 존재하지 않음(최초 실행) | `GetAll()`이 빈 벡터 반환, 예외 없음. 파일은 `Add`/`Update`/`Delete`가 처음 성공할 때 생성됨 | phase2, `SampleRepositoryTests`/`OrderRepositoryTests` |
| 파일 내용이 빈 문자열(0바이트) | 빈 벡터 반환 | `CorruptedJsonTests` |
| JSON 문법 오류(파싱 실패) | 빈 벡터 반환. **원본 손상 파일은 덮어쓰지 않고 그대로 보존**(읽기 전용 동작), 이후 `Add`/`Update`/`Delete`가 성공하면 그 시점에 정상 파일로 덮어써짐 | phase5, `CorruptedJsonTests` |
| 최상위가 배열 자체이거나 `samples`/`orders` 키가 없음 | 빈 벡터 반환 | `CorruptedJsonTests` |
| 배열 안 개별 원소의 필수 필드 누락/타입 불일치로 파싱 실패 | 해당 원소만 건너뛰고(skip) `std::cerr`에 경고 출력, 나머지 원소는 정상 반영 | `CorruptedJsonTests` |
| `Add`에 이미 존재하는 ID | `std::invalid_argument` 예외 발생, 파일 미변경 | `SampleRepositoryTests`/`OrderRepositoryTests` |
| `Update`/`Delete`에 존재하지 않는 ID | 예외 없이 `false` 반환, 파일 미변경 | `UpdateDeleteTests` |

### 영속성 핵심 테스트

`PersistenceReloadTests.cpp`가 이 PoC의 핵심 증거다. 한 `Repository` 인스턴스로
저장한 뒤, **새로운 `Repository` 인스턴스**(재시작을 흉내)를 같은 파일 경로로
생성해 `GetAll()`/`FindById()` 결과가 저장 전 데이터와 완전히 일치하는지
검증한다. `UpdateDeleteTests.cpp`의 Update/Delete 테스트들도 동일하게 재로딩까지
포함해 검증한다.

## 사용 라이브러리

- [nlohmann/json](https://github.com/nlohmann/json) 3.11.3 — NuGet 패키지
  `nlohmann.json` (`Project1/packages/nlohmann.json.3.11.3`), JSON 파싱/직렬화
- [GoogleTest](https://github.com/google/googletest) 1.8.1 — NuGet 패키지
  `Microsoft.googletest.v140.windesktop.msvcstl.static.rt-dyn`
  (`Project1/packages/Microsoft.googletest.v140.windesktop.msvcstl.static.rt-dyn.1.8.1.8`)
- C++20 표준 라이브러리의 `<filesystem>` — 경로/디렉토리 처리

## 빌드 방법

Visual Studio 2022 이상(Platform Toolset `v145`, `WindowsTargetPlatformVersion 10.0`,
언어 표준 `stdcpp20`)에서 `Project1/Project1.slnx`를 열어 빌드하거나,
Developer Command Prompt에서 MSBuild로 빌드한다.

```
msbuild Json\Project1\Project1.slnx /p:Configuration=Debug /p:Platform=x64
```

NuGet 패키지는 `packages.config`(`Project1/Project1/packages.config`,
`Project1/Project1Tests/packages.config`) 기준으로 `nuget restore`가 필요할 수
있다(리포지토리에는 `packages/`가 `.gitignore`로 제외되어 있으므로 최초
빌드 전 복원 필요).

## 테스트 실행 방법

`Project1Tests` 프로젝트를 빌드하면 `Project1Tests.exe`가 생성된다. Visual
Studio의 테스트 탐색기에서 실행하거나, 빌드 산출물을 직접 실행한다.

```
Json\Project1\Project1Tests\x64\Debug\Project1Tests.exe
```

## TDD 진행 과정 (Phase 구성)

`docs/Plan.md` 및 각 `docs/phaseN-*.md`에 상세 계획이 있다. Red → Green →
Refactor 순서로 아래 5개 phase를 순차 진행했다(각 phase는 이전 phase의 테스트가
모두 통과한 상태에서만 시작).

1. **Phase 1 — 엔티티 & 직렬화**: `Sample`/`Order` 구조체 정의, `ToJson`/
   `SampleFromJson`·`OrderFromJson`으로 JSON round-trip 검증.
2. **Phase 2 — 빈 상태 / 파일 없음**: 파일이 없을 때 예외 없이 빈 컬렉션 반환,
   빈 컬렉션 저장 형식 확정.
3. **Phase 3 — Create/Read(같은 인스턴스)**: `Add` 후 같은 `Repository`
   인스턴스에서 `GetAll`/`FindById`로 조회되고, 파일이 디스크에 생성되는지 검증.
4. **Phase 4 — 영속성 핵심(재시작 재현)**: 저장 후 새 `Repository` 인스턴스로
   재로딩했을 때 데이터가 완전히 일치하는지 검증(`PersistenceReloadTests.cpp`).
5. **Phase 5 — Update/Delete + 손상 JSON 정책**: `Update`/`Delete`가 파일에
   반영되고 재로딩 후에도 유지되는지 검증(`UpdateDeleteTests.cpp`), 손상/누락/
   빈 파일에 대한 정책을 정의하고 고정(`CorruptedJsonTests.cpp`).

Phase 1~5 전체가 완료된 시점 기준 `Project1Tests.exe`는 7개 테스트 파일에
걸쳐 총 51개의 테스트 케이스로 구성되어 있으며(`docs/Plan.md` 기준), 모두
통과한다.

## 범위 밖 (다른 PoC/Main의 책임)

- 콘솔 메뉴/화면 흐름은 이 PoC의 관심사가 아니다. `main.cpp`는 엔티티/직렬화가
  올바르게 연결되어 있는지 확인하는 최소한의 smoke test만 수행한다.
- 동시성 제어는 고려하지 않는다(단일 프로세스 콘솔 앱을 전제).
- 생산 큐/재고 계산 등 도메인 로직은 Repository의 책임이 아니다. Repository는
  저장/조회(CRUD)만 담당하며, 계산 로직은 `Main`의 서비스 계층에서 구현한다.
- 이 PoC에서 확정한 JSON 스키마와 Repository 인터페이스를 `Monitor/`, `Dummy/`,
  `Main/`이 공유하는 데이터 포맷의 기준으로 삼는다.

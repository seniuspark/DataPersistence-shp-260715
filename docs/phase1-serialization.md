# Phase 1: 엔티티 정의 & JSON 직렬화/역직렬화 round-trip

[← 전체 계획](./Plan.md) | 다음: [Phase 2 — 빈 상태 / 파일 없음](./phase2-empty-state.md)

## 목표

`Sample`, `Order` 엔티티를 C++20 구조체로 정의하고, JSON 문자열 ↔ 객체 간
변환이 정보 손실 없이 왕복(round-trip)되는지 검증한다. 이 phase에서 확정하는
필드명/타입이 이후 모든 phase와 `Monitor`/`Dummy`/`Main`의 공유 기준이 된다.

## JSON 스키마 확정

### Sample — `data/samples.json`

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

| JSON 필드 | C++ 타입 | 도메인 필드 |
|---|---|---|
| sampleId | `std::string` | SampleId |
| name | `std::string` | Name |
| avgProductionTime | `double` | AvgProductionTime (min/ea) |
| yield | `double` (0.0~1.0) | Yield |
| stock | `int` | Stock |

### Order — `data/orders.json`

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

| JSON 필드 | C++ 타입 | 도메인 필드 |
|---|---|---|
| orderId | `std::string` | OrderId |
| sampleId | `std::string` | SampleId |
| customerName | `std::string` | CustomerName |
| quantity | `int` | Quantity |
| status | `std::string` enum(`RESERVED`,`REJECTED`,`PRODUCING`,`CONFIRMED`,`RELEASE`) | Status |
| createdAt | `std::string` (ISO8601, `YYYY-MM-DDTHH:MM:SS`) | CreatedAt |

`status`는 C++ 쪽에서 `enum class OrderStatus`로 표현하고, 직렬화 시
문자열로 변환한다 (다른 PoC와의 상호운용을 위해 JSON 상에서는 항상 문자열).

## 작성할 테스트 목록 (Red 단계)

1. `Sample_ToJson_ProducesExpectedFields` — Sample 객체 → JSON 변환 시 5개
   필드가 모두 올바른 키/값으로 존재하는지 확인
2. `Sample_RoundTrip_PreservesAllFields` — Sample → JSON 문자열 → 역직렬화 →
   원본과 필드별 동일 확인 (경계값: yield=0.0, yield=1.0, stock=0 포함)
3. `Order_ToJson_ProducesExpectedFields` — Order 객체 → JSON 변환 필드 확인
4. `Order_RoundTrip_PreservesAllFields` — Order round-trip, 5개 Status 값
   전부에 대해 파라미터화 테스트(RESERVED/REJECTED/PRODUCING/CONFIRMED/RELEASE)
5. `Order_InvalidStatusString_ThrowsOrHandlesDefined` — 알 수 없는 status
   문자열을 역직렬화할 때의 동작을 정의하고 고정 (예: 예외 던짐)
6. `SampleList_ToJsonArray_RoundTrip` — 여러 Sample을 배열로 직렬화 후
   역직렬화했을 때 순서/개수/내용 일치
7. `OrderList_ToJsonArray_RoundTrip` — 여러 Order 배열 round-trip

## 구현해야 할 클래스/함수/파일

- `Project1/Project1/Sample.h` — `struct Sample { std::string sampleId; std::string name; double avgProductionTime; double yield; int stock; };`
- `Project1/Project1/Order.h` — `enum class OrderStatus`, `struct Order`, 및
  `OrderStatus`↔문자열 변환 함수 (`ToString(OrderStatus)`, `ParseOrderStatus(std::string)`)
- `Project1/Project1/JsonSerializer.h/.cpp` (또는 nlohmann-json 사용 시
  `to_json`/`from_json` free function을 `Sample.h`/`Order.h`에 정의)
  - `nlohmann::json ToJson(const Sample&)`, `Sample SampleFromJson(const nlohmann::json&)`
  - `nlohmann::json ToJson(const Order&)`, `Order OrderFromJson(const nlohmann::json&)`
- 테스트 파일: `Project1/Project1Tests/SampleSerializationTests.cpp`,
  `Project1/Project1Tests/OrderSerializationTests.cpp` (테스트 프로젝트 경로는
  실제 vcxproj 구성 시 확정)

## 완료 기준

- 위 7개 테스트가 모두 통과
- Sample/Order 필드명·타입이 본 문서의 표와 완전히 일치
- `OrderStatus` 5개 값 모두 문자열 변환이 왕복 검증됨

## 다음 phase와의 연결점

Phase 2에서는 이 phase의 `ToJson`/`FromJson` 함수를 사용해 "파일이 없을 때
빈 컬렉션을 반환"하는 Repository의 최소 동작을 구현한다. 엔티티 자체는 이
phase에서 변경하지 않는다.

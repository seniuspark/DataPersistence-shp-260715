# Phase 5: Update/Delete 및 손상된 JSON 처리 정책 확정

[← 전체 계획](./Plan.md) | 이전: [Phase 4 — 영속성 핵심(재시작 재현)](./phase4-persistence-reload.md)

## 목표

Repository의 CRUD를 완성한다(Update/Delete)는 물론, 손상되거나 예상치 못한
형식의 JSON 파일을 만났을 때의 동작을 명확히 정의하고 테스트로 고정한다.
이 정책은 `Monitor`가 실행 중 파일을 읽다가 쓰기 도중 상태를 만날 수 있는
상황과도 연관되므로 신중히 정한다.

## Update/Delete 테스트 목록 (Red 단계)

1. `SampleRepository_Update_ExistingSample_ChangesPersistAfterReload` —
   `Update(sample)` 호출 후 새 인스턴스로 재로딩 시 변경된 필드(Stock 등) 반영
2. `SampleRepository_Update_NonExistentId_PolicyDefined` — 존재하지 않는
   `sampleId`로 `Update` 호출 시 정책 확정(예: 예외 또는 false 반환)
3. `SampleRepository_Delete_ExistingSample_RemovedAfterReload` — `Delete(id)`
   호출 후 재로딩 시 해당 항목이 목록에서 사라짐
4. `SampleRepository_Delete_NonExistentId_PolicyDefined` — 존재하지 않는 ID
   삭제 시도 시 정책 확정(예: false 반환, 예외 없음)
5. `OrderRepository_Update_StatusTransition_PersistsAfterReload` — Order의
   `Status`를 `RESERVED`→`CONFIRMED`로 변경 후 재로딩해 반영 확인 (도메인
   상태 흐름과 연동되는 대표 시나리오)
6. `OrderRepository_Delete_ExistingOrder_RemovedAfterReload` — CLAUDE.md상
   "주문 Delete는 실제 요구사항에 없을 수 있으나 Repository 완전성을 위해
   포함"이라는 방침에 따라 Order도 동일하게 검증

## 손상/이상 파일 처리 테스트 목록 (Red 단계)

7. `SampleRepository_CorruptedJsonFile_GetAllPolicyDefined` — 문법이 깨진
   JSON(예: 중괄호 누락) 파일을 만났을 때의 동작을 정의:
   - 채택 정책(안): 예외를 던지지 않고 **빈 컬렉션을 반환** + 원본 손상 파일은
     보존(덮어쓰지 않음, 사용자가 복구할 수 있도록). 단, 이후 `Add`/`Update`가
     호출되면 그 시점부터는 정상 파일로 덮어써짐.
   - 테스트는 이 정책을 코드로 고정한다 (예외를 던지는 정책으로 바꾸더라도
     반드시 여기 문서와 함께 갱신)
8. `SampleRepository_JsonWithMissingRequiredField_PolicyDefined` — 필수 필드가
   빠진 항목이 배열 안에 섞여 있을 때: 해당 항목만 건너뛰는지, 전체를 무효로
   보는지 정책 확정 (채택 정책안: 개별 항목 단위로 파싱 실패 시 그 항목만
   무시하고 로그성 경고만 남김 — PoC 범위에서는 표준에러 출력 정도로 충분)
9. `SampleRepository_JsonWithUnknownTopLevelStructure_PolicyDefined` — 최상위가
   배열이 아니거나 `samples` 키가 없는 경우 빈 컬렉션으로 처리
10. `OrderRepository_CorruptedJsonFile_SamePolicyAsSample` — Order도 Sample과
    동일한 손상 파일 정책을 따름을 확인 (일관성 검증)
11. `SampleRepository_EmptyFileContent_GetAllReturnsEmpty` — 파일은 존재하나
    내용이 빈 문자열(0바이트)인 경우도 빈 컬렉션으로 처리

## 구현해야 할 클래스/함수/파일

- `SampleRepository`/`OrderRepository`에 메서드 추가:
  - `bool Update(const T& entity);` — 성공 시 true, 대상 없음 시 false
  - `bool Delete(const std::string& id);` — 성공 시 true, 대상 없음 시 false
- `LoadFromFile()` 내부에 예외 처리 추가:
  - JSON 파싱 자체가 실패(문법 오류) → catch 후 빈 벡터 반환
  - 최상위 구조가 기대와 다름(배열 아님/키 없음) → 빈 벡터 반환
  - 개별 원소 파싱 실패 → 해당 원소만 skip, 나머지는 정상 반영
- 위 정책은 `Sample.h`/`Order.h`의 `SampleFromJson`/`OrderFromJson` 및
  `SampleRepository`/`OrderRepository`의 `LoadFromFile()`에 일관되게 적용

## 완료 기준

- 위 11개 테스트 모두 통과
- Update/Delete 각각 "존재하는 대상"과 "존재하지 않는 대상" 양쪽 케이스가
  테스트로 커버됨
- 손상 파일에 대한 정책이 Sample/Order 양쪽에서 동일하게 적용되고, 정책
  내용이 이 문서에 최종 확정되어 있음

## PoC 완료 판정 (이 phase 완료 = Json PoC 전체 완료)

- Sample/Order 각각 Create/Read/Update/Delete가 모두 테스트로 검증됨
  (Phase 3: Create/Read, Phase 5: Update/Delete)
- 재시작 시뮬레이션(새 Repository 인스턴스) 재로딩 테스트가 통과함 (Phase 4)
- 손상/누락 파일에 대한 동작이 정의되고 테스트됨 (Phase 5)
- 확정된 JSON 스키마(Phase 1)를 이 저장소의 `CLAUDE.md` 또는 별도 요약에 남겨,
  다른 PoC(`Monitor`/`Dummy`/`Main`)가 참고할 수 있게 한다

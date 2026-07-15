#include <iostream>

#include "Order.h"
#include "Sample.h"

// Phase 1 smoke check: entities + JSON round-trip are wired up correctly.
// The console menu/UI is out of scope for this PoC (see CLAUDE.md).
int main() {
    Sample sample{"S-001", "웨이퍼 A", 12.5, 0.95, 100};
    Order order{"ORD-20260416-0043", "S-001", "고객사A", 50, OrderStatus::RESERVED, "2026-04-16T09:00:00"};

    std::cout << ToJson(sample).dump() << std::endl;
    std::cout << ToJson(order).dump() << std::endl;
    return 0;
}

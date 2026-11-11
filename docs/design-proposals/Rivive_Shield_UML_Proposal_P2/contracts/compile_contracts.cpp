// Declaration checks only: no production behavior is implemented or linked.
#include "rivive_shield_proposed.hpp"
#include <type_traits>
using namespace rivive;
static_assert(std::is_abstract<BookingRepository>::value, "Repository boundary must remain an interface");
static_assert(std::is_abstract<IncidentRepository>::value, "Repository boundary must remain an interface");
static_assert(std::is_abstract<DemoTargetAdapter>::value, "Target boundary must remain an interface");
static_assert(std::is_abstract<RepairCommand>::value, "Command boundary must remain an interface");
static_assert(std::is_base_of<RepairCommand, RestartDemoServiceCommand>::value, "Retain Command realization");
static_assert(std::is_base_of<RepairCommand, RestoreBookingInvariantCommand>::value, "Retain Command realization");
static_assert(!std::is_abstract<SqliteBookingRepository>::value, "Concrete adapter declares every override");
static_assert(!std::is_abstract<SqliteIncidentRepository>::value, "Concrete adapter declares every override");
static_assert(!std::is_abstract<FixedDemoTargetAdapter>::value, "Concrete adapter declares every override");
static_assert(std::is_same<decltype(RepairPlan{}.evidenceReferences()), std::vector<EvidenceId>>::value, "Typed retained method");
static_assert(std::is_same<decltype(Evidence{}.reference()), EvidenceId>::value, "Typed retained method");
static_assert(std::is_abstract<OwnedDemoProcess>::value, "OS ownership boundary");
static_assert(!std::is_abstract<WindowsOwnedDemoProcess>::value, "Windows declarations complete");
static_assert(!std::is_abstract<MacOwnedDemoProcess>::value, "macOS declarations complete");

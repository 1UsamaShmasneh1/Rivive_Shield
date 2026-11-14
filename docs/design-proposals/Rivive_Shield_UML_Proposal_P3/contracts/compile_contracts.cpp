#include "p3_draft.hpp"
#include <type_traits>
using namespace rivive::p3_draft;
static_assert(std::is_default_constructible_v<DatabaseVerifier>);
static_assert(std::is_default_constructible_v<IncidentStateMachine>);
static_assert(std::is_default_constructible_v<WindowsConnectionObserver>);
static_assert(std::is_default_constructible_v<MacConnectionObserver>);
static_assert(!std::is_copy_constructible_v<WindowsOwnedDemoProcess>);
static_assert(!std::is_move_constructible_v<WindowsOwnedDemoProcess>);
static_assert(!std::is_copy_constructible_v<MacOwnedDemoProcess>);
static_assert(!std::is_move_constructible_v<MacOwnedDemoProcess>);
static_assert(!std::is_copy_constructible_v<PcapLoopbackCapture>);
static_assert(!std::is_move_constructible_v<PcapLoopbackCapture>);
static_assert(!std::is_copy_constructible_v<TcpStreamDecoder>);
static_assert(!std::is_move_constructible_v<TcpStreamDecoder>);
static_assert(!std::is_copy_constructible_v<HttpSessionTracker>);
static_assert(!std::is_move_constructible_v<HttpSessionTracker>);
static_assert(!std::is_copy_constructible_v<BookingHttpServer>);
static_assert(!std::is_move_constructible_v<BookingHttpServer>);
static_assert(!std::is_copy_constructible_v<WxDesktopView>);
static_assert(!std::is_move_constructible_v<WxDesktopView>);
static_assert(!std::is_copy_constructible_v<SqliteBookingRepository>);
static_assert(!std::is_move_constructible_v<SqliteBookingRepository>);
static_assert(!std::is_copy_constructible_v<SqliteIncidentRepository>);
static_assert(!std::is_move_constructible_v<SqliteIncidentRepository>);
int main() { return 0; }

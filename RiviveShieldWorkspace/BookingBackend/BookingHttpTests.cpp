#include "../../docs/design-proposals/Rivive_Shield_UML_Proposal_P3/contracts/p3_draft.hpp"

#include <vector>
#include <iostream>
#include <thread>
#include <chrono>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <optional>



namespace rivive::p3_draft
{
    class FakeBookingRepository final : public BookingRepository
    {
    public:
        Result<std::vector<Room>> listRooms() const override
        {
            return std::vector<Room>{
                {"room-01", "Room 1"},
                { "room-02", "Room 2" }
            };
        }


        Result<BookingSnapshot> snapshot() const override
        {
            if (savedBooking_)
            {
                return BookingSnapshot{ 1,{ *savedBooking_ } };
            }

            return BookingSnapshot{ 0,{} };
        }


        Result<BookingCreateResult> createIfAvailable(
            const BookingRequest& request,
            UtcTime now) override
        {
            if (request.roomId != "room-01" &&
                request.roomId != "room-02")
            {
                return Error{
                    ErrorCode::NotFound,
                    "The requested room does not exist."
                };
            }

            if (request.id == "req-test-020" &&
                savedBooking_ &&
                request.roomId == savedBooking_->roomId &&
                BookingService::overlaps(
                    savedBooking_->interval,
                    request.interval))
            {
                return Error{
                    ErrorCode::OverlapRejected,
                    "Test repository: booking overlap."
                };
            }

            if (request.id != "req-test-009")
            {
                return Error{
                    ErrorCode::StorageUnavailable,
                    "Test repository: storage is unavailable."
                };
            }

            if (savedBooking_)
            {
                const Booking& previous = *savedBooking_;

                if (previous.roomId != request.roomId ||
                    previous.interval.start != request.interval.start ||
                    previous.interval.end != request.interval.end ||
                    previous.syntheticLabel != request.syntheticLabel)
                {
                    return Error{
                        ErrorCode::Conflict,
                        "Request ID reused with different data."
                    };
                }

                // Return original booking for identical retry.
                return BookingCreateResult{ previous, false };
            }

            // Create the booking for the first time.
            savedBooking_ = Booking{
                request.id,
                request.roomId,
                request.interval,
                request.syntheticLabel,
                now,
                1
            };

            return BookingCreateResult{ *savedBooking_, true };
        }

    private:
        std::optional<Booking> savedBooking_;
    };
}



int main()
{
    using namespace rivive::p3_draft;

    FakeBookingRepository repository;

    BookingService service(
        repository,
        []() {
            return UtcTime{ std::chrono::milliseconds{ 0 } };
        }
    );

    auto result = service.listRooms();

    const auto* rooms =
        std::get_if<std::vector<Room>>(&result);

    const bool passed =
        rooms != nullptr &&
        rooms->size() == 2 &&
        (*rooms)[0].id == "room-01" &&
        (*rooms)[1].id == "room-02";

    std::cout << "Test 1 (list rooms): "
        << (passed ? "PASS" : "FAIL")
        << '\n';

    auto bookingsResult = service.listBookings();

    const auto* snapshot =
        std::get_if<BookingSnapshot>(&bookingsResult);

    const bool test2 =
        snapshot != nullptr &&
        snapshot->bookings.empty();

    std::cout << "Test 2 (empty bookings): "
        << (test2 ? "PASS" : "FAIL")
        << '\n';

    BookingRequest request{
        "test-request-01",
        "room-01",
        TimeRange{
            UtcTime{ std::chrono::hours{ 10 } },
            UtcTime{ std::chrono::hours{ 11 } }
        },
        "Test booking"
    };

    auto createResult = service.createBooking(request);

    const auto* createError =
        std::get_if<Error>(&createResult);

    const bool test3 =
        createError != nullptr &&
        createError->code == ErrorCode::StorageUnavailable;

    std::cout << "Test 3 (storage unavailable): "
        << (test3 ? "PASS" : "FAIL")
        << '\n';

    // Test 4: GET /api/rooms over HTTP.
    HttpServerConfiguration configuration{
        "127.0.0.1",
        18081,
        "",
        8192,
        std::chrono::milliseconds{ 5000 }
    };

    BookingHttpServer httpServer(service, configuration);

    Status serverStatus = std::monostate{};

    std::thread serverThread([&]()
        {
            serverStatus = httpServer.run();
        });

    httplib::Client client("127.0.0.1", 18081);

    bool test4 = false;

    for (int attempt = 0; attempt < 30; ++attempt)
    {
        auto response = client.Get("/api/rooms");

        if (response)
        {
            auto json = nlohmann::json::parse(
                response->body, nullptr, false);

            if (response->status == 200 &&
                json.is_object() &&
                json.contains("rooms") &&
                json["rooms"].is_array())
            {
                const auto& rooms = json["rooms"];

                test4 =
                    rooms.size() == 2 &&
                    rooms[0].is_object() &&
                    rooms[1].is_object() &&
                    rooms[0].value("id", std::string{}) == "room-01" &&
                    rooms[1].value("id", std::string{}) == "room-02";
            }

            break;
        }

        std::this_thread::sleep_for(
            std::chrono::milliseconds{ 100 });
    }

    // Test 5: GET /api/bookings over HTTP.
    bool test5 = false;

    auto bookingsResponse = client.Get("/api/bookings");

    if (bookingsResponse && bookingsResponse->status == 200)
    {
        auto json = nlohmann::json::parse(
            bookingsResponse->body, nullptr, false);

        test5 =
            json.is_object() &&
            json.contains("bookings") &&
            json["bookings"].is_array() &&
            json["bookings"].empty();
    }

    std::cout << "Test 5 (HTTP GET bookings): "
        << (test5 ? "PASS" : "FAIL")
        << '\n';

    // Test 6: POST booking - storage unavailable.
    bool test6 = false;

    const std::string bookingJson = R"({
        "requestId": "req-test-006",
        "roomId": "room-01",
        "startsAt": "2026-11-26T08:00:00.000Z",
        "endsAt": "2026-11-26T09:00:00.000Z",
        "label": "Test booking"
    })";

    auto postResponse = client.Post(
        "/api/bookings",
        bookingJson,
        "application/json"
    );

    if (postResponse && postResponse->status == 503)
    {
        auto json = nlohmann::json::parse(
            postResponse->body, nullptr, false);

        if (json.is_object() &&
            json.contains("error") &&
            json["error"].is_object())
        {
            test6 =
                json["error"].value(
                    "code", std::string{}) ==
                "STORAGE_UNAVAILABLE";
        }
    }

    std::cout << "Test 6 (HTTP POST storage error): "
        << (test6 ? "PASS" : "FAIL")
        << '\n';

    // Test 7: POST booking with missing required field.
    bool test7 = false;

    const std::string invalidBookingJson = R"({
        "requestId": "req-test-007",
        "roomId": "room-01",
        "startsAt": "2026-11-26T08:00:00.000Z",
        "endsAt": "2026-11-26T09:00:00.000Z"
    })";

    auto invalidResponse = client.Post(
        "/api/bookings",
        invalidBookingJson,
        "application/json"
    );

    if (invalidResponse && invalidResponse->status == 400)
    {
        auto json = nlohmann::json::parse(
            invalidResponse->body, nullptr, false);

        if (json.is_object() &&
            json.contains("error") &&
            json["error"].is_object())
        {
            test7 =
                json["error"].value(
                    "code", std::string{}) ==
                "INVALID_REQUEST";
        }
    }

    std::cout << "Test 7 (HTTP missing field): "
        << (test7 ? "PASS" : "FAIL")
        << '\n';

    // Test 8: POST booking with invalid time interval.
    bool test8 = false;

    const std::string invalidTimeJson = R"({
        "requestId": "req-test-008",
        "roomId": "room-01",
        "startsAt": "2026-11-26T10:00:00.000Z",
        "endsAt": "2026-11-26T09:00:00.000Z",
        "label": "Invalid time test"
    })";

    auto timeResponse = client.Post(
        "/api/bookings",
        invalidTimeJson,
        "application/json"
    );

    if (timeResponse && timeResponse->status == 400)
    {
        auto json = nlohmann::json::parse(
            timeResponse->body, nullptr, false);

        if (json.is_object() &&
            json.contains("error") &&
            json["error"].is_object())
        {
            test8 =
                json["error"].value(
                    "code", std::string{}) ==
                "INVALID_REQUEST";
        }
    }

    std::cout << "Test 8 (HTTP invalid time): "
        << (test8 ? "PASS" : "FAIL")
        << '\n';

    // Test 9: Successful HTTP booking.
    bool test9 = false;

    const std::string validBookingJson = R"({
        "requestId": "req-test-009",
        "roomId": "room-01",
        "startsAt": "2026-11-26T08:00:00.000Z",
        "endsAt": "2026-11-26T09:00:00.000Z",
        "label": "Successful booking"
    })";

    auto successResponse = client.Post(
        "/api/bookings",
        validBookingJson,
        "application/json"
    );

    if (successResponse && successResponse->status == 201)
    {
        auto json = nlohmann::json::parse(
            successResponse->body, nullptr, false);

        if (json.is_object() &&
            json.contains("booking") &&
            json["booking"].is_object())
        {
            test9 =
                json["booking"].value(
                    "id", std::string{}) == "req-test-009" &&
                json["booking"].value(
                    "roomId", std::string{}) == "room-01";
        }
    }

    std::cout << "Test 9 (HTTP POST created): "
        << (test9 ? "PASS" : "FAIL")
        << '\n';

    // Test 10: Identical booking retry.
    bool test10 = false;

    auto retryResponse = client.Post(
        "/api/bookings",
        validBookingJson,
        "application/json"
    );

    if (successResponse && retryResponse &&
        retryResponse->status == 200)
    {
        auto firstJson = nlohmann::json::parse(
            successResponse->body, nullptr, false);

        auto retryJson = nlohmann::json::parse(
            retryResponse->body, nullptr, false);

        test10 =
            firstJson.is_object() &&
            retryJson.is_object() &&
            firstJson.contains("booking") &&
            retryJson.contains("booking") &&
            firstJson["booking"].is_object() &&
            retryJson["booking"].is_object() &&
            firstJson["booking"] == retryJson["booking"];
    }

    std::cout << "Test 10 (HTTP identical retry): "
        << (test10 ? "PASS" : "FAIL")
        << '\n';

    // Test 11: Same request ID with different data.
    bool test11 = false;

    const std::string conflictingBookingJson = R"({
        "requestId": "req-test-009",
        "roomId": "room-01",
        "startsAt": "2026-11-26T08:00:00.000Z",
        "endsAt": "2026-11-26T09:00:00.000Z",
        "label": "Different booking label"
    })";

    auto conflictResponse = client.Post(
        "/api/bookings",
        conflictingBookingJson,
        "application/json"
    );

    if (conflictResponse && conflictResponse->status == 409)
    {
        auto json = nlohmann::json::parse(
            conflictResponse->body, nullptr, false);

        if (json.is_object() &&
            json.contains("error") &&
            json["error"].is_object())
        {
            test11 =
                json["error"].value(
                    "code", std::string{}) ==
                "REQUEST_ID_CONFLICT";
        }
    }

    std::cout << "Test 11 (HTTP request ID conflict): "
        << (test11 ? "PASS" : "FAIL")
        << '\n';

    // Test 12: GET bookings after successful creation.
    bool test12 = false;

    auto savedBookingsResponse = client.Get("/api/bookings");

    if (savedBookingsResponse &&
        savedBookingsResponse->status == 200)
    {
        auto json = nlohmann::json::parse(
            savedBookingsResponse->body, nullptr, false);

        if (json.is_object() &&
            json.contains("bookings") &&
            json["bookings"].is_array() &&
            json["bookings"].size() == 1 &&
            json["bookings"][0].is_object())
        {
            test12 =
                json["bookings"][0].value(
                    "id", std::string{}) == "req-test-009" &&
                json["bookings"][0].value(
                    "roomId", std::string{}) == "room-01";
        }
    }

    std::cout << "Test 12 (HTTP GET saved booking): "
        << (test12 ? "PASS" : "FAIL")
        << '\n';

    // Test 13: Reject an HTTP request larger than 8192 bytes.
    bool test13 = false;

    nlohmann::json oversizedRequest = {
        { "requestId", "req-test-013" },
        { "roomId", "room-01" },
        { "startsAt", "2026-11-26T08:00:00.000Z" },
        { "endsAt", "2026-11-26T09:00:00.000Z" },
        { "label", std::string(8300, 'A') }
    };

    std::string oversizedBody = oversizedRequest.dump();

    auto oversizedResponse = client.Post(
        "/api/bookings",
        oversizedBody,
        "application/json"
    );

    if (oversizedResponse && oversizedResponse->status == 413)
    {
        auto json = nlohmann::json::parse(
            oversizedResponse->body, nullptr, false);

        if (json.is_object() &&
            json.contains("error") &&
            json["error"].is_object())
        {
            test13 =
                json["error"].value(
                    "code", std::string{}) ==
                "REQUEST_TOO_LARGE";
        }
    }

    std::cout << "Test 13 (HTTP request too large): "
        << (test13 ? "PASS" : "FAIL")
        << '\n';

    if (!test13)
    {
        if (oversizedResponse)
        {
            std::cout << "HTTP status: "
                << oversizedResponse->status << '\n';

            std::cout << "Response body: "
                << oversizedResponse->body.substr(0, 200)
                << '\n';
        }
        else
        {
            std::cout << "No HTTP response received.\n";
        }
    }

    // Test 14: Reject an invalid calendar date.
    bool test14 = false;

    const std::string invalidDateJson = R"({
        "requestId": "req-test-014",
        "roomId": "room-01",
        "startsAt": "2026-02-30T08:00:00.000Z",
        "endsAt": "2026-02-30T09:00:00.000Z",
        "label": "Invalid date test"
    })";

    auto dateResponse = client.Post(
        "/api/bookings",
        invalidDateJson,
        "application/json"
    );

    if (dateResponse && dateResponse->status == 400)
    {
        auto json = nlohmann::json::parse(
            dateResponse->body, nullptr, false);

        if (json.is_object() &&
            json.contains("error") &&
            json["error"].is_object())
        {
            test14 =
                json["error"].value(
                    "code", std::string{}) ==
                "INVALID_REQUEST";
        }
    }

    std::cout << "Test 14 (HTTP invalid calendar date): "
        << (test14 ? "PASS" : "FAIL")
        << '\n';

    if (!test14 && dateResponse)
    {
        std::cout << "HTTP status: "
            << dateResponse->status << '\n';

        std::cout << "Response body: "
            << dateResponse->body << '\n';
    }

    // Test 15: Reject empty request ID.
    bool test15 = false;

    const std::string emptyIdJson = R"({
        "requestId": "",
        "roomId": "room-01",
        "startsAt": "2026-11-26T08:00:00.000Z",
        "endsAt": "2026-11-26T09:00:00.000Z",
        "label": "Empty ID test"
    })";

    auto emptyIdResponse = client.Post(
        "/api/bookings",
        emptyIdJson,
        "application/json"
    );

    if (emptyIdResponse && emptyIdResponse->status == 400)
    {
        auto json = nlohmann::json::parse(
            emptyIdResponse->body, nullptr, false);

        if (json.is_object() &&
            json.contains("error") &&
            json["error"].is_object())
        {
            test15 =
                json["error"].value(
                    "code", std::string{}) ==
                "INVALID_REQUEST";
        }
    }

    std::cout << "Test 15 (HTTP empty request ID): "
        << (test15 ? "PASS" : "FAIL")
        << '\n';

    // Test 16: Booking overlap and adjacency.
    const UtcTime baseTime{
        std::chrono::milliseconds{ 1000000 }
    };

    const TimeRange existing{
        baseTime,
        baseTime + std::chrono::hours{ 1 }
    };

    const TimeRange overlapping{
        baseTime + std::chrono::minutes{ 30 },
        baseTime + std::chrono::minutes{ 90 }
    };

    const TimeRange adjacent{
        baseTime + std::chrono::hours{ 1 },
        baseTime + std::chrono::hours{ 2 }
    };

    const bool test16 =
        BookingService::overlaps(existing, overlapping) &&
        BookingService::overlaps(overlapping, existing) &&
        !BookingService::overlaps(existing, adjacent) &&
        !BookingService::overlaps(adjacent, existing);

    std::cout << "Test 16 (overlap and adjacency): "
        << (test16 ? "PASS" : "FAIL")
        << '\n';

    // Test 17: Label length boundary.
    nlohmann::json validLabelRequest = {
        { "requestId", "req-test-017" },
        { "roomId", "room-01" },
        { "startsAt", "2026-11-26T08:00:00.000Z" },
        { "endsAt", "2026-11-26T09:00:00.000Z" },
        { "label", std::string(128, 'A') }
    };

    auto validLabelResponse = client.Post(
        "/api/bookings",
        validLabelRequest.dump(),
        "application/json"
    );

    validLabelRequest["label"] = std::string(129, 'A');

    auto invalidLabelResponse = client.Post(
        "/api/bookings",
        validLabelRequest.dump(),
        "application/json"
    );

    bool test17 =
        validLabelResponse &&
        validLabelResponse->status == 503 &&
        invalidLabelResponse &&
        invalidLabelResponse->status == 400;

    std::cout << "Test 17 (label length boundary): "
        << (test17 ? "PASS" : "FAIL")
        << '\n';

    // Test 18: Unicode scalar length validation.
    BookingRequest unicodeRequest{
        "req-test-018",
        "room-01",
        TimeRange{
            UtcTime{ std::chrono::milliseconds{ 0 } },
            UtcTime{ std::chrono::hours{ 1 } }
        },
        ""
    };

    // One emoji is one Unicode scalar (4 UTF-8 bytes).
    const std::string emoji = "\xF0\x9F\x98\x80";

    for (int i = 0; i < 128; ++i)
    {
        unicodeRequest.syntheticLabel += emoji;
    }

    const bool valid128 =
        std::holds_alternative<std::monostate>(
            BookingService::validate(unicodeRequest));

    unicodeRequest.syntheticLabel += emoji;

    const bool invalid129 =
        std::holds_alternative<Error>(
            BookingService::validate(unicodeRequest));

    // 'e' plus combining accent counts as two scalars.
    unicodeRequest.syntheticLabel =
        std::string(127, 'A') + "e\xCC\x81";

    const bool combiningRejected =
        std::holds_alternative<Error>(
            BookingService::validate(unicodeRequest));

    const bool test18 =
        valid128 && invalid129 && combiningRejected;

    std::cout << "Test 18 (Unicode scalar limits): "
        << (test18 ? "PASS" : "FAIL")
        << '\n';

    // Test 19: Booking a room that does not exist.
    bool test19 = false;

    const std::string missingRoomJson = R"({
        "requestId": "req-test-019",
        "roomId": "room-does-not-exist",
        "startsAt": "2026-11-26T08:00:00.000Z",
        "endsAt": "2026-11-26T09:00:00.000Z",
        "label": "Missing room test"
    })";

    auto missingRoomResponse = client.Post(
        "/api/bookings",
        missingRoomJson,
        "application/json"
    );

    if (missingRoomResponse &&
        missingRoomResponse->status == 404)
    {
        auto json = nlohmann::json::parse(
            missingRoomResponse->body, nullptr, false);

        if (json.is_object() &&
            json.contains("error") &&
            json["error"].is_object())
        {
            test19 =
                json["error"].value(
                    "code", std::string{}) ==
                "ROOM_NOT_FOUND";
        }
    }

    std::cout << "Test 19 (HTTP room not found): "
        << (test19 ? "PASS" : "FAIL")
        << '\n';

    // Test 20: Reject overlapping booking over HTTP.
    bool test20 = false;

    const std::string overlappingBookingJson = R"({
        "requestId": "req-test-020",
        "roomId": "room-01",
        "startsAt": "2026-11-26T08:30:00.000Z",
        "endsAt": "2026-11-26T09:30:00.000Z",
        "label": "Overlapping booking"
    })";

    auto overlapResponse = client.Post(
        "/api/bookings",
        overlappingBookingJson,
        "application/json"
    );

    if (overlapResponse && overlapResponse->status == 409)
    {
        auto json = nlohmann::json::parse(
            overlapResponse->body, nullptr, false);

        if (json.is_object() &&
            json.contains("error") &&
            json["error"].is_object())
        {
            test20 =
                json["error"].value(
                    "code", std::string{}) ==
                "BOOKING_OVERLAP";
        }
    }

    std::cout << "Test 20 (HTTP booking overlap): "
        << (test20 ? "PASS" : "FAIL")
        << '\n';

    httpServer.requestStop();
    serverThread.join();

    test4 = test4 &&
        std::holds_alternative<std::monostate>(serverStatus);

    std::cout << "Test 4 (HTTP GET rooms): "
        << (test4 ? "PASS" : "FAIL")
        << '\n';

    return (passed && test2 && test3 && test4 &&
        test5 && test6 && test7 && test8 &&
        test9 && test10 && test11 && test12 &&
        test13 && test14 && test15 && test16 &&
        test17 && test18 && test19 && test20) ? 0 : 1;
}

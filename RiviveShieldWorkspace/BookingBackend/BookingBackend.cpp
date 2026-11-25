#include <iostream>
#include <sqlite3.h>
#include <httplib.h>
#include "../../docs/design-proposals/Rivive_Shield_UML_Proposal_P3/contracts/p3_draft.hpp"

int main()
{
    std::cout << "SQLite version: " << sqlite3_libversion() << '\n';

    httplib::Server server;
    std::cout << "cpp-httplib server object created successfully.\n";

    const auto base = rivive::p3_draft::UtcTime{
        std::chrono::milliseconds{ 0 }
    };

    const rivive::p3_draft::TimeRange first{
        base + std::chrono::hours{ 10 },
        base + std::chrono::hours{ 11 }
    };

    const rivive::p3_draft::TimeRange second{
        base + std::chrono::hours{ 10 } + std::chrono::minutes{ 30 },
        base + std::chrono::hours{ 11 } + std::chrono::minutes{ 30 }
    };

    const rivive::p3_draft::TimeRange third{
        base + std::chrono::hours{ 11 },
        base + std::chrono::hours{ 12 }
    };

    bool test1 = rivive::p3_draft::BookingService::overlaps(first, second);
    bool test2 = rivive::p3_draft::BookingService::overlaps(first, third);

    std::cout << "Test 1 (overlap expected): "
        << (test1 ? "PASS" : "FAIL") << '\n';

    std::cout << "Test 2 (adjacent expected): "
        << (!test2 ? "PASS" : "FAIL") << '\n';

    const rivive::p3_draft::TimeRange fourth{
        base + std::chrono::hours{ 9 },
        base + std::chrono::hours{ 10 }
    };

    const rivive::p3_draft::TimeRange fifth{
        base + std::chrono::hours{ 10 } + std::chrono::minutes{ 15 },
        base + std::chrono::hours{ 10 } + std::chrono::minutes{ 45 }
    };

    bool test3 = rivive::p3_draft::BookingService::overlaps(first, fourth);
    bool test4 = rivive::p3_draft::BookingService::overlaps(first, fifth);

    std::cout << "Test 3 (reverse adjacency): "
        << (!test3 ? "PASS" : "FAIL") << '\n';

    std::cout << "Test 4 (contained booking): "
        << (test4 ? "PASS" : "FAIL") << '\n';

    if (!test1 || test2 || test3 || !test4)
    {
        return 1;
    }

    // Booking validation tests

    rivive::p3_draft::BookingRequest validRequest{
        "req-001", "room-01", first, "Demo"
    };

    auto emptyIdRequest = validRequest;
    emptyIdRequest.id = "";

    auto emptyRoomRequest = validRequest;
    emptyRoomRequest.roomId = "";

    auto invalidTimeRequest = validRequest;
    invalidTimeRequest.interval.end =
        invalidTimeRequest.interval.start;

    auto expectInvalid = [](const rivive::p3_draft::BookingRequest& request)
        {
            auto result = rivive::p3_draft::BookingService::validate(request);
            const auto* error =
                std::get_if<rivive::p3_draft::Error>(&result);

            return error != nullptr &&
                error->code == rivive::p3_draft::ErrorCode::InvalidArgument;
        };

    bool test5 = expectInvalid(emptyIdRequest);
    bool test6 = expectInvalid(emptyRoomRequest);
    bool test7 = expectInvalid(invalidTimeRequest);

    auto validResult =
        rivive::p3_draft::BookingService::validate(validRequest);

    bool test8 = std::holds_alternative<std::monostate>(validResult);

    std::cout << "Test 5 (empty booking ID): "
        << (test5 ? "PASS" : "FAIL") << '\n';

    std::cout << "Test 6 (empty room ID): "
        << (test6 ? "PASS" : "FAIL") << '\n';

    std::cout << "Test 7 (invalid time): "
        << (test7 ? "PASS" : "FAIL") << '\n';

    std::cout << "Test 8 (valid booking): "
        << (test8 ? "PASS" : "FAIL") << '\n';

    if (!test5 || !test6 || !test7 || !test8)
    {
        return 1;
    }

    auto reversedTimeRequest = validRequest;

    reversedTimeRequest.interval.start =
        base + std::chrono::hours{ 12 };

    reversedTimeRequest.interval.end =
        base + std::chrono::hours{ 11 };

    bool test9 = expectInvalid(reversedTimeRequest);

    std::cout << "Test 9 (end before start): "
        << (test9 ? "PASS" : "FAIL") << '\n';

    if (!test9)
    {
        return 1;
    }

    auto longLabelRequest = validRequest;

    longLabelRequest.syntheticLabel = std::string(129, 'A');

    bool test10 = expectInvalid(longLabelRequest);

    std::cout << "Test 10 (label over 128 characters): "
        << (test10 ? "PASS" : "FAIL") << '\n';

    auto unicodeRequest = validRequest;
    unicodeRequest.syntheticLabel.clear();

    const std::string emoji = "\xF0\x9F\x98\x80";

    for (int i = 0; i < 128; ++i)
    {
        unicodeRequest.syntheticLabel += emoji;
    }

    auto unicodeResult =
        rivive::p3_draft::BookingService::validate(unicodeRequest);

    bool test11 =
        std::holds_alternative<std::monostate>(unicodeResult);

    unicodeRequest.syntheticLabel += emoji;

    bool test12 = expectInvalid(unicodeRequest);

    std::cout << "Test 11 (128 Unicode characters): "
        << (test11 ? "PASS" : "FAIL") << '\n';

    std::cout << "Test 12 (129 Unicode characters): "
        << (test12 ? "PASS" : "FAIL") << '\n';

    if (!test11 || !test12)
    {
        return 1;
    }

    auto invalidUnicodeRequest = validRequest;

    // Invalid UTF-8 sequence
    invalidUnicodeRequest.syntheticLabel = "\xC0\xAF";

    bool test13 = expectInvalid(invalidUnicodeRequest);

    std::cout << "Test 13 (invalid UTF-8): "
        << (test13 ? "PASS" : "FAIL") << '\n';

    if (!test13)
    {
        return 1;
    }

    server.Get("/api/rooms",
        [](const httplib::Request&, httplib::Response& res)
        {
            res.status = 503;
            res.set_content(
                R"({"error":{"code":"STORAGE_UNAVAILABLE","message":"Booking storage is not connected yet."}})",
                "application/json"
            );
        });

    std::cout << "Starting temporary HTTP test on http://127.0.0.1:8080\n";

    if (!server.listen("127.0.0.1", 8080))
    {
        std::cerr << "Failed to start HTTP server.\n";
        return 1;
    }

    return 0;
}


namespace rivive::p3_draft
{
    bool BookingService::overlaps(
        const TimeRange& first,
        const TimeRange& second)
    {
        return first.start < second.end &&
            second.start < first.end;
    }

    Status BookingService::validate(const BookingRequest& request)
    {
        if (request.id.empty() || request.roomId.empty())
        {
            return Error{
                ErrorCode::InvalidArgument,
                "Booking ID and Room ID cannot be empty."
            };
        }

        if (request.interval.start >= request.interval.end)
        {
            return Error{
                ErrorCode::InvalidArgument,
                "Start time must be before end time."
            };
        }

        // Validate UTF-8 and limit each string to 128 Unicode scalars.
        const auto validText = [](const std::string& text) -> bool
            {
                std::size_t count = 0;

                for (std::size_t i = 0; i < text.size();)
                {
                    const unsigned char first =
                        static_cast<unsigned char>(text[i]);

                    std::size_t width = 0;

                    if (first <= 0x7F)
                        width = 1;
                    else if (first >= 0xC2 && first <= 0xDF)
                        width = 2;
                    else if (first >= 0xE0 && first <= 0xEF)
                        width = 3;
                    else if (first >= 0xF0 && first <= 0xF4)
                        width = 4;
                    else
                        return false;

                    if (text.size() - i < width)
                        return false;

                    for (std::size_t j = 1; j < width; ++j)
                    {
                        const unsigned char byte =
                            static_cast<unsigned char>(text[i + j]);

                        if ((byte & 0xC0) != 0x80)
                            return false;
                    }

                    if (width > 1)
                    {
                        const unsigned char second =
                            static_cast<unsigned char>(text[i + 1]);

                        if (width == 3 &&
                            ((first == 0xE0 && second < 0xA0) ||
                                (first == 0xED && second > 0x9F)))
                            return false;

                        if (width == 4 &&
                            ((first == 0xF0 && second < 0x90) ||
                                (first == 0xF4 && second > 0x8F)))
                            return false;
                    }

                    ++count;

                    if (count > 128)
                        return false;

                    i += width;
                }

                return true;
            };

        if (!validText(request.id) ||
            !validText(request.roomId) ||
            !validText(request.syntheticLabel))
        {
            return Error{
                ErrorCode::InvalidArgument,
                "Invalid Unicode or text exceeds 128 characters."
            };
        }

        return std::monostate{};
    }

    BookingService::BookingService(
        BookingRepository& repository,
        Now now)
        : repository_(repository),
        now_(std::move(now))
    {
    }

    Result<std::vector<Room>> BookingService::listRooms() const
    {
        return repository_.listRooms();
    }

    Result<BookingSnapshot> BookingService::listBookings() const
    {
        return repository_.snapshot();
    }

    Result<BookingCreateResult> BookingService::createBooking(
        const BookingRequest& request)
    {
        Status validation = validate(request);

        if (const auto* error = std::get_if<Error>(&validation))
        {
            return *error;
        }

        return repository_.createIfAvailable(request, now_());
    }

    BookingHttpServer::BookingHttpServer(
        BookingService& service,
        HttpServerConfiguration configuration)
        : service_(service),
        configuration_(std::move(configuration)),
        server_(new httplib::Server())
    {
    }

    BookingHttpServer::~BookingHttpServer()
    {
        delete server_;
    }

    void BookingHttpServer::requestStop()
    {
        server_->stop();
    }

    Status BookingHttpServer::run()
    {
        if (configuration_.bindAddress != "127.0.0.1" ||
            configuration_.port == 0)
        {
            return Error{
                ErrorCode::InvalidArgument,
                "Invalid local HTTP server configuration."
            };
        }

        Status routesStatus = registerRoutes();

        if (const auto* error = std::get_if<Error>(&routesStatus))
        {
            return *error;
        }

        if (!server_->listen(
            configuration_.bindAddress,
            configuration_.port))
        {
            return Error{
                ErrorCode::InternalError,
                "Failed to start HTTP server."
            };
        }

        return std::monostate{};
    }

    Status BookingHttpServer::registerRoutes()
    {
        server_->Get("/api/rooms",
            [](const httplib::Request&, httplib::Response& res)
            {
                res.status = 503;
                res.set_content(
                    R"({"error":{"code":"STORAGE_UNAVAILABLE","message":"Booking storage is not connected yet."}})",
                    "application/json"
                );
            });

        return std::monostate{};
    }

}

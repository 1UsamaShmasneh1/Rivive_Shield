#include <iostream>
#include <sqlite3.h>
#include <httplib.h>
#include "../../docs/design-proposals/Rivive_Shield_UML_Proposal_P3/contracts/p3_draft.hpp"
#include <nlohmann/json.hpp>
#include <ctime>
#include <iomanip>
#include <sstream>

#ifndef RS08_HTTP_TESTS

int main()
{
    std::cout << "SQLite version: " << sqlite3_libversion() << '\n';

    httplib::Server server;
    std::cout << "cpp-httplib server object created successfully.\n";

    nlohmann::json jsonTest = {
        { "status", "JSON_READY" },
        { "test", "RS-08" }
    };

    std::cout << "JSON test: " << jsonTest.dump() << '\n';

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


#endif

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

        if (configuration_.maxRequestBytes != 8192)
        {
            return Error{
                ErrorCode::InvalidArgument,
                "Booking request limit must be 8192 bytes."
            };
        }

        server_->set_payload_max_length(
            configuration_.maxRequestBytes);

        server_->set_error_handler(
            [](const httplib::Request&, httplib::Response& response)
            {
                if (response.status == 413)
                {
                    response.set_content(
                        R"({"error":{"code":"REQUEST_TOO_LARGE","message":"Request body exceeds the 8192-byte limit."}})",
                        "application/json"
                    );
                }
            }
        );

        if (configuration_.requestTimeout <=
            std::chrono::milliseconds::zero())
        {
            return Error{
                ErrorCode::InvalidArgument,
                "HTTP timeout must be positive."
            };
        }

        server_->set_read_timeout(
            configuration_.requestTimeout);

        server_->set_write_timeout(
            configuration_.requestTimeout);

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

        const auto toUtcString = [](UtcTime value) -> std::string
            {
                const auto elapsed = value.time_since_epoch();

                const auto seconds =
                    std::chrono::floor<std::chrono::seconds>(elapsed);

                const auto milliseconds =
                    std::chrono::duration_cast<std::chrono::milliseconds>(
                        elapsed - seconds).count();

                const std::time_t raw =
                    static_cast<std::time_t>(seconds.count());

                std::tm utc{};


#ifdef _WIN32
                if (gmtime_s(&utc, &raw) != 0)
                {
                    return {};
                }
#else
                if (gmtime_r(&raw, &utc) == nullptr)
                {
                    return {};
                }
#endif


                std::ostringstream output;

                output << std::put_time(&utc, "%Y-%m-%dT%H:%M:%S")
                    << '.'
                    << std::setfill('0')
                    << std::setw(3)
                    << milliseconds
                    << 'Z';

                return output.str();
            };

        server_->Get("/api/rooms",
            [this](const httplib::Request&, httplib::Response& res)
            {
                auto result = service_.listRooms();

                if (const auto* rooms =
                    std::get_if<std::vector<Room>>(&result))
                {
                    nlohmann::json roomList = nlohmann::json::array();

                    for (const Room& room : *rooms)
                    {
                        roomList.push_back({
                            { "id", room.id },
                            { "name", room.name }
                            });
                    }

                    nlohmann::json response = {
                        { "rooms", roomList }
                    };

                    res.status = 200;
                    res.set_content(response.dump(), "application/json");
                    return;
                }

                nlohmann::json errorResponse = {
                    { "error",{
                        { "code", "STORAGE_UNAVAILABLE" },
                        { "message", "Unable to load rooms." }
                    } }
                };

                res.status = 503;
                res.set_content(errorResponse.dump(), "application/json");
            });

        server_->Get("/api/bookings",
            [this, toUtcString](
                const httplib::Request&,
                httplib::Response& res)
            {
                auto result = service_.listBookings();

                if (const auto* snapshot =
                    std::get_if<BookingSnapshot>(&result))
                {
                    nlohmann::json bookingList =
                        nlohmann::json::array();

                    for (const Booking& booking : snapshot->bookings)
                    {
                        const std::string startsAt =
                            toUtcString(booking.interval.start);

                        const std::string endsAt =
                            toUtcString(booking.interval.end);

                        const std::string createdAt =
                            toUtcString(booking.createdAt);

                        if (startsAt.empty() ||
                            endsAt.empty() ||
                            createdAt.empty())
                        {
                            res.status = 503;
                            res.set_content(
                                R"({"error":{"code":"STORAGE_UNAVAILABLE","message":"Unable to prepare bookings."}})",
                                "application/json");
                            return;
                        }

                        bookingList.push_back({
                            { "id", booking.id },
                            { "roomId", booking.roomId },
                            { "startsAt", startsAt },
                            { "endsAt", endsAt },
                            { "label", booking.syntheticLabel },
                            { "createdAt", createdAt }
                            });
                    }

                    nlohmann::json response = {
                        { "bookings", bookingList }
                    };

                    res.status = 200;
                    res.set_content(
                        response.dump(), "application/json");
                    return;
                }

                res.status = 503;
                res.set_content(
                    R"({"error":{"code":"STORAGE_UNAVAILABLE","message":"Unable to load bookings."}})",
                    "application/json");
            });

        server_->Post("/api/bookings",
            [this, toUtcString](const httplib::Request& req, httplib::Response& res)
            {
                if (req.body.size() > 8192)
                {
                    res.status = 413;
                    res.set_content(
                        R"({"error":{"code":"REQUEST_TOO_LARGE","message":"Request body exceeds 8192 bytes."}})",
                        "application/json"
                    );
                    return;
                }

                auto body = nlohmann::json::parse(
                    req.body, nullptr, false);

                if (!body.is_object())
                {
                    res.status = 400;
                    res.set_content(
                        R"({"error":{"code":"INVALID_REQUEST","message":"Invalid JSON request."}})",
                        "application/json"
                    );
                    return;
                }

                const char* requiredFields[] = {
                    "requestId",
                    "roomId",
                    "startsAt",
                    "endsAt",
                    "label"
                };

                bool validFields = (body.size() == 5);

                for (const char* field : requiredFields)
                {
                    if (!body.contains(field) ||
                        !body.at(field).is_string())
                    {
                        validFields = false;
                        break;
                    }
                }

                if (!validFields)
                {
                    res.status = 400;
                    res.set_content(
                        R"({"error":{"code":"INVALID_REQUEST","message":"Missing, extra, or invalid booking fields."}})",
                        "application/json"
                    );
                    return;
                }

                const auto hasUtcFormat = [](const std::string& value)
                    {
                        if (value.size() != 24)
                            return false;

                        for (std::size_t i = 0; i < value.size(); ++i)
                        {
                            char expected = '\0';

                            switch (i)
                            {
                            case 4:
                            case 7:  expected = '-'; break;
                            case 10: expected = 'T'; break;
                            case 13:
                            case 16: expected = ':'; break;
                            case 19: expected = '.'; break;
                            case 23: expected = 'Z'; break;
                            }

                            if (expected != '\0')
                            {
                                if (value[i] != expected)
                                    return false;
                            }
                            else if (value[i] < '0' || value[i] > '9')
                            {
                                return false;
                            }
                        }

                        return true;
                    };

                if (!hasUtcFormat(body.at("startsAt").get<std::string>()) ||
                    !hasUtcFormat(body.at("endsAt").get<std::string>()))
                {
                    res.status = 400;
                    res.set_content(
                        R"({"error":{"code":"INVALID_REQUEST","message":"Invalid UTC timestamp format."}})",
                        "application/json"
                    );
                    return;
                }

                // Check actual UTC calendar and clock values.
                const auto isValidUtcDate = [](const std::string& value)
                    {
                        // The 24-character format was checked above.
                        const auto numberAt = [&value](int pos, int length)
                            {
                                int result = 0;

                                for (int i = 0; i < length; ++i)
                                {
                                    result = result * 10 + (value[pos + i] - '0');
                                }

                                return result;
                            };

                        const int year = numberAt(0, 4);
                        const int month = numberAt(5, 2);
                        const int day = numberAt(8, 2);
                        const int hour = numberAt(11, 2);
                        const int minute = numberAt(14, 2);
                        const int second = numberAt(17, 2);

                        if (month < 1 || month > 12 ||
                            day < 1 || hour > 23 ||
                            minute > 59 || second > 59)
                        {
                            return false;
                        }

                        const int daysInMonth[] = {
                            31, 28, 31, 30, 31, 30,
                            31, 31, 30, 31, 30, 31
                        };

                        const bool leapYear =
                            year % 4 == 0 &&
                            (year % 100 != 0 || year % 400 == 0);

                        int maxDay = daysInMonth[month - 1];

                        if (month == 2 && leapYear)
                        {
                            maxDay = 29;
                        }

                        return day <= maxDay;
                    };

                if (!isValidUtcDate(body.at("startsAt").get<std::string>()) ||
                    !isValidUtcDate(body.at("endsAt").get<std::string>()))
                {
                    res.status = 400;
                    res.set_content(
                        R"({"error":{"code":"INVALID_REQUEST","message":"Invalid calendar date or clock time."}})",
                        "application/json"
                    );
                    return;
                }

                // Convert a validated UTC timestamp to UtcTime.
                const auto parseUtcTime = [](const std::string& value) -> UtcTime
                    {
                        const auto numberAt = [&value](int pos, int length)
                            {
                                std::int64_t result = 0;

                                for (int i = 0; i < length; ++i)
                                {
                                    result = result * 10 + (value[pos + i] - '0');
                                }

                                return result;
                            };

                        std::int64_t year = numberAt(0, 4);
                        const std::int64_t month = numberAt(5, 2);
                        const std::int64_t day = numberAt(8, 2);

                        const std::int64_t hour = numberAt(11, 2);
                        const std::int64_t minute = numberAt(14, 2);
                        const std::int64_t second = numberAt(17, 2);
                        const std::int64_t millisecond = numberAt(20, 3);

                        // Gregorian date to days since 1970-01-01.
                        year -= (month <= 2);

                        const std::int64_t era =
                            (year >= 0 ? year : year - 399) / 400;

                        const std::int64_t yearOfEra = year - era * 400;
                        const std::int64_t adjustedMonth =
                            month + (month > 2 ? -3 : 9);

                        const std::int64_t dayOfYear =
                            (153 * adjustedMonth + 2) / 5 + day - 1;

                        const std::int64_t daysOfEra =
                            yearOfEra * 365 + yearOfEra / 4 -
                            yearOfEra / 100 + dayOfYear;

                        const std::int64_t days =
                            era * 146097 + daysOfEra - 719468;

                        const std::int64_t totalMilliseconds =
                            days * 86400000LL +
                            hour * 3600000LL +
                            minute * 60000LL +
                            second * 1000LL +
                            millisecond;

                        return UtcTime{
                            std::chrono::milliseconds{ totalMilliseconds }
                        };
                    };

                const UtcTime startsAt =
                    parseUtcTime(body.at("startsAt").get<std::string>());

                const UtcTime endsAt =
                    parseUtcTime(body.at("endsAt").get<std::string>());

                BookingRequest bookingRequest{
                    body.at("requestId").get<std::string>(),
                    body.at("roomId").get<std::string>(),
                    TimeRange{ startsAt, endsAt },
                    body.at("label").get<std::string>()
                };

                Status validationStatus =
                    BookingService::validate(bookingRequest);

                if (std::holds_alternative<Error>(validationStatus))
                {
                    res.status = 400;
                    res.set_content(
                        R"({"error":{"code":"INVALID_REQUEST","message":"Invalid booking data or time interval."}})",
                        "application/json"
                    );
                    return;
                }

                

                auto result = service_.createBooking(bookingRequest);

                if (const auto* created =
                    std::get_if<BookingCreateResult>(&result))
                {
                    const Booking& booking = created->booking;

                    const std::string start =
                        toUtcString(booking.interval.start);
                    const std::string end =
                        toUtcString(booking.interval.end);
                    const std::string createdAt =
                        toUtcString(booking.createdAt);

                    if (start.empty() || end.empty() || createdAt.empty())
                    {
                        res.status = 503;
                        res.set_content(
                            R"({"error":{"code":"STORAGE_UNAVAILABLE","message":"Unable to prepare booking response."}})",
                            "application/json");
                        return;
                    }

                    nlohmann::json response = {
                        { "booking",{
                            { "id", booking.id },
                            { "roomId", booking.roomId },
                            { "startsAt", start },
                            { "endsAt", end },
                            { "label", booking.syntheticLabel },
                            { "createdAt", createdAt }
                        } }
                    };

                    res.status = created->created ? 201 : 200;
                    res.set_content(response.dump(), "application/json");
                    return;
                }

                const Error& error = std::get<Error>(result);

                int status = 503;
                std::string code = "STORAGE_UNAVAILABLE";

                switch (error.code)
                {
                case ErrorCode::InvalidArgument:
                    status = 400;
                    code = "INVALID_REQUEST";
                    break;
                case ErrorCode::NotFound:
                    status = 404;
                    code = "ROOM_NOT_FOUND";
                    break;
                case ErrorCode::OverlapRejected:
                    status = 409;
                    code = "BOOKING_OVERLAP";
                    break;
                case ErrorCode::Conflict:
                    status = 409;
                    code = "REQUEST_ID_CONFLICT";
                    break;
                case ErrorCode::OutcomeUncertain:
                    status = 503;
                    code = "WRITE_OUTCOME_UNCERTAIN";
                    break;
                default:
                    break;
                }

                nlohmann::json errorResponse = {
                    { "error",{
                        { "code", code },
                        { "message", "Booking request could not be completed." }
                    } }
                };

                res.status = status;
                res.set_content(
                    errorResponse.dump(), "application/json");

              
            });

        return std::monostate{};
    }


}

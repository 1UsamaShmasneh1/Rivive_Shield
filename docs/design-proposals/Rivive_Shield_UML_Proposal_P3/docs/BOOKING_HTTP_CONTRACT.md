# Booking HTTP contract (RS-06 Item 3)

This design contract incorporates the [owner-approved booking rules](https://trello.com/c/3M4GExqE#action-6b01e553838edc22cacd0d33), [four wire clarifications](https://trello.com/c/3M4GExqE#action-6b01e85d469bab3c73d93d94) and [independent design acceptance](https://trello.com/c/3M4GExqE#action-6b01e9f720575438cd59eed9). It specifies the wire behavior, not an implemented API.

## Scope and payloads

- `GET /api/rooms`: 200, `{"rooms":[{"id":"room-01","name":"Room 1"}]}`.
- `GET /api/bookings`: 200, `{"bookings":[...]}` with booking objects of the form below.
- `POST /api/bookings`: five required fields only, as defined by the [request schema](../schemas/booking_request.schema.json). No cancellation endpoint in this milestone.

Example POST:

```json
{"requestId":"req-001","roomId":"room-01","startsAt":"2026-11-21T08:00:00.000Z","endsAt":"2026-11-21T09:00:00.000Z","label":"Demo"}
```

Example POST success (201 newly committed; 200 identical retry):

```json
{"booking":{"id":"req-001","roomId":"room-01","startsAt":"2026-11-21T08:00:00.000Z","endsAt":"2026-11-21T09:00:00.000Z","label":"Demo","createdAt":"2026-11-21T07:00:00.000Z"}}
```

The wire `requestId` maps to C++ `BookingRequest::id` and to the returned booking `id`. The wire `label` maps to `syntheticLabel`; `startsAt` and `endsAt` map to `TimeRange::start` and `end`. `Booking::version` and `BookingSnapshot::revision` are internal and must not appear in HTTP responses.

## Validation and atomicity

- Require valid UTC timestamps in the exact `YYYY-MM-DDTHH:mm:ss.SSSZ` form for request and response timestamps. Validate actual calendar and clock values, not just the shape. Store UTC and display in the client device timezone. If a local time occurs twice, React presents both occurrences and requires an explicit choice. If nonexistent, prompt for another input. Do not send the request until the input is resolved and converted to UTC.
- Require `startsAt < endsAt`. Same-room intervals are half-open; adjacency is allowed.
- Require nonempty IDs. Each non-time POST string has a maximum of 128 valid Unicode scalar values after JSON decoding, not bytes or UTF-16 code units. A single-scalar emoji such as 😀 counts as one and a letter plus a combining mark counts as two. Reject invalid scalar sequences. The POST body limit is separately 8192 bytes; an oversized body receives 413. The schema alone does not check the byte limit, interval ordering or scalar validity.
- For an existing request ID, compare room, both UTC instants at millisecond precision, and the exact decoded label. Ignore JSON field order and whitespace outside strings. An identical retry returns the original booking with unchanged `createdAt`; changed data under the same ID is a conflict. Deduplication, overlap checking and persistence of the booking/outcome belong in one SQLite write transaction. Do not report success on an uncertain commit.

## Error envelope and status mapping

Errors use `{"error":{"code":"BOOKING_OVERLAP","message":"The room is already booked for this time."}}`. React branches on the stable `error.code`, never the message. Do not expose internal SQL, paths or storage details.

| HTTP | error.code |
|---|---|
| 400 | `INVALID_REQUEST` |
| 404 | `ROOM_NOT_FOUND` |
| 409 | `BOOKING_OVERLAP` or `REQUEST_ID_CONFLICT` |
| 413 | `REQUEST_TOO_LARGE` |
| 503 | `STORAGE_UNAVAILABLE` or `WRITE_OUTCOME_UNCERTAIN` |

An uncertain write does not authorize automatic retry or a new request ID for the same attempt. Implementation, runtime and integration acceptance remain separate.

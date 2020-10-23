/*++

    Copyright (c) Microsoft Corporation.
    Licensed under the MIT License.

Abstract:

    QUIC Perf RPS Client declaration. Defines the functions and
    variables used in the LatencyClient class.

--*/


#pragma once

#include "PerfHelpers.h"
#include "PerfBase.h"
#include "PerfCommon.h"

class LatencyClient : public PerfBase {
public:
    QUIC_STATUS
    Init(
        _In_ int argc,
        _In_reads_(argc) _Null_terminated_ char* argv[]
        ) override;

    QUIC_STATUS
    Start(
        _In_ QUIC_EVENT* StopEvent
        ) override;

    QUIC_STATUS
    Wait(
        _In_ int Timeout
        ) override;

private:
    struct ConnectionContext {
        ConnectionContext(
            _In_ LatencyClient* Client)
            : Client{Client} { }
        LatencyClient* Client;
        uint32_t OutstandingCount {0};
        uint32_t IsInitialStream {FALSE};
#if DEBUG
        uint8_t Padding[12];
#endif
    };

    struct StreamContext {
        StreamContext(
            _In_ LatencyClient* Client,
            _In_ ConnectionContext* ConnContext,
            _In_ uint64_t StartTime)
            : Client{Client}, ConnContext{ConnContext}, StartTime{StartTime} { }
        LatencyClient* Client;
        ConnectionContext* ConnContext;
        uint64_t StartTime;
#if DEBUG
        uint8_t Padding[8];
#endif
    };

    QUIC_STATUS
    ConnectionCallback(
        _In_ ConnectionContext* ConnContext,
        _In_ HQUIC ConnectionHandle,
        _Inout_ QUIC_CONNECTION_EVENT* Event
        );

    QUIC_STATUS
    StreamCallback(
        _In_ StreamContext* StrmContext,
        _In_ HQUIC StreamHandle,
        _Inout_ QUIC_STREAM_EVENT* Event
        );

    QUIC_STATUS
    SendRequest(
        _In_ HQUIC Handle,
        _In_ ConnectionContext* ConnContext
        );

    MsQuicRegistration Registration {true};
    MsQuicConfiguration Configuration {
        Registration,
        MsQuicAlpn(PERF_ALPN),
        MsQuicSettings()
            .SetDisconnectTimeoutMs(PERF_DEFAULT_DISCONNECT_TIMEOUT)
            .SetIdleTimeoutMs(PERF_DEFAULT_IDLE_TIMEOUT)
            .SetSendBufferingEnabled(false),
        MsQuicCredentialConfig(
            QUIC_CREDENTIAL_FLAG_CLIENT |
            QUIC_CREDENTIAL_FLAG_NO_CERTIFICATE_VALIDATION)};
    uint16_t Port {PERF_DEFAULT_PORT};
    UniquePtr<char[]> Target;
    uint32_t RunTime {RPS_DEFAULT_RUN_TIME};
    uint32_t ConnectionCount {RPS_DEFAULT_CONNECTION_COUNT};
    uint32_t ParallelRequests {RPS_DEFAULT_PARALLEL_REQUEST_COUNT};
    uint32_t RequestLength {RPS_DEFAULT_REQUEST_LENGTH};
    uint32_t ResponseLength {RPS_DEFAULT_RESPONSE_LENGTH};

    struct QuicBufferScopeQuicAlloc {
        QUIC_BUFFER* Buffer;
        QuicBufferScopeQuicAlloc() noexcept : Buffer(nullptr) { }
        operator QUIC_BUFFER* () noexcept { return Buffer; }
        ~QuicBufferScopeQuicAlloc() noexcept { if (Buffer) { QUIC_FREE(Buffer); } }
    };

    QuicBufferScopeQuicAlloc RequestBuffer;
    QUIC_EVENT* CompletionEvent {nullptr};
    QUIC_ADDR LocalAddresses[RPS_MAX_CLIENT_PORT_COUNT];
    uint32_t ActiveConnections {0};
    EventScope AllConnected {true};
    uint64_t StartedRequests {0};
    uint64_t SendCompletedRequests {0};
    uint64_t CompletedRequests {0};
    uint32_t AfterStreamCount {20};
    UniquePtr<uint32_t[]> LatencyValues {nullptr};
    uint64_t MaxLatencyIndex {0};
    QuicPoolAllocator<ConnectionContext> ConnectionContextAllocator;
    QuicPoolAllocator<StreamContext> StreamContextAllocator;
    UniquePtr<ConnectionScope[]> Connections {nullptr};
    bool Running {true};
};

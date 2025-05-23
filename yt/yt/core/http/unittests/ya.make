GTEST(unittester-core-http)

INCLUDE(${ARCADIA_ROOT}/yt/ya_cpp.make.inc)

PROTO_NAMESPACE(yt)

SRCS(
    http_ut.cpp
)

INCLUDE(${ARCADIA_ROOT}/yt/opensource.inc)

PEERDIR(
    library/cpp/testing/common
    yt/yt/core
    yt/yt/core/http
    yt/yt/core/https
    yt/yt/core/net/mock
    yt/yt/core/test_framework
)

REQUIREMENTS(
    ram:32
    ram:32
    ram_disk:1
)

FORK_TESTS()

SPLIT_FACTOR(5)

SIZE(MEDIUM)

IF (OS_DARWIN)
    SIZE(LARGE)
    TAG(
        ya:fat
        ya:force_sandbox
        ya:exotic_platform
        ya:large_tests_on_single_slots
    )
ENDIF()

REQUIREMENTS(ram:32)

END()

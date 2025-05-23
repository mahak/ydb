PY3TEST()

FORK_TEST_FILES()
SIZE(MEDIUM)

ENV(YDB_DRIVER_BINARY="ydb/apps/ydbd/ydbd")
ENV(YDB_HARD_MEMORY_LIMIT_BYTES="8000000000")

TEST_SRCS(
    test_session_pool.py
    test_crud.py
    test_indexes.py
    test_discovery.py
    test_execute_scheme.py
    test_insert.py
    test_isolation.py
    test_public_api.py
    test_read_table.py
    test_session_grace_shutdown.py
)

DEPENDS(
    ydb/apps/ydbd
)

PEERDIR(
    contrib/python/requests
    contrib/python/tornado/tornado-4
    ydb/tests/library
    ydb/tests/oss/ydb_sdk_import
    ydb/public/sdk/python
)

IF (SANITIZER_TYPE)
    REQUIREMENTS(ram:10)
ENDIF()

END()

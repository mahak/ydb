LIBRARY()

SRCS(
    issue_helpers.h
    issue_helpers.cpp
)

PEERDIR(
    ydb/library/ydb_issue/proto
    ydb/library/yql/public/ydb_issue
)

RESOURCE(
    ydb/library/ydb_issue/ydb_issue.txt ydb_issue.txt
)

END()

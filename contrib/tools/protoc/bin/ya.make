PROGRAM(protoc)

VERSION(None)

LICENSE(BSD-3-Clause)

NO_COMPILER_WARNINGS()

PEERDIR(
    contrib/libs/protoc
)
SRCDIR(
    contrib/libs/protoc
)

SRCS(
    src/google/protobuf/compiler/main.cc
)

INCLUDE(${ARCADIA_ROOT}/contrib/tools/protoc/ya.make.induced_deps)

END()

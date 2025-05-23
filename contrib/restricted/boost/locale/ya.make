# Generated by devtools/yamaker from nixpkgs 24.05.

LIBRARY()

LICENSE(BSL-1.0)

LICENSE_TEXTS(.yandex_meta/licenses.list.txt)

VERSION(1.88.0)

ORIGINAL_SOURCE(https://github.com/boostorg/locale/archive/boost-1.88.0.tar.gz)

PEERDIR(
    contrib/libs/icu
    contrib/restricted/boost/assert
    contrib/restricted/boost/charconv
    contrib/restricted/boost/config
    contrib/restricted/boost/core
    contrib/restricted/boost/iterator
    contrib/restricted/boost/predef
    contrib/restricted/boost/thread
)

ADDINCL(
    GLOBAL contrib/restricted/boost/locale/include
    contrib/restricted/boost/locale/src
)

NO_COMPILER_WARNINGS()

NO_UTIL()

CFLAGS(
    -DBOOST_LOCALE_WITH_ICU
)

IF (DYNAMIC_BOOST)
    CFLAGS(
        GLOBAL -DBOOST_LOCALE_DYN_LINK
    )
ENDIF()

IF (OS_ANDROID)
    CFLAGS(
        -DBOOST_LOCALE_NO_POSIX_BACKEND
        -DBOOST_LOCALE_NO_WINAPI_BACKEND
    )
ELSEIF (OS_WINDOWS)
    CFLAGS(
        -DBOOST_LOCALE_NO_POSIX_BACKEND
    )
    SRCS(
        src/win32/collate.cpp
        src/win32/converter.cpp
        src/win32/lcid.cpp
        src/win32/numeric.cpp
        src/win32/win_backend.cpp
    )
ELSE()
    CFLAGS(
        -DBOOST_LOCALE_NO_WINAPI_BACKEND
    )
    SRCS(
        src/posix/codecvt.cpp
        src/posix/collate.cpp
        src/posix/converter.cpp
        src/posix/numeric.cpp
        src/posix/posix_backend.cpp
    )
ENDIF()

SRCS(
    src/encoding/codepage.cpp
    src/icu/boundary.cpp
    src/icu/codecvt.cpp
    src/icu/collator.cpp
    src/icu/conversion.cpp
    src/icu/date_time.cpp
    src/icu/formatter.cpp
    src/icu/formatters_cache.cpp
    src/icu/icu_backend.cpp
    src/icu/numeric.cpp
    src/shared/date_time.cpp
    src/shared/format.cpp
    src/shared/formatting.cpp
    src/shared/generator.cpp
    src/shared/iconv_codecvt.cpp
    src/shared/ids.cpp
    src/shared/localization_backend.cpp
    src/shared/message.cpp
    src/shared/mo_lambda.cpp
    src/std/codecvt.cpp
    src/std/collate.cpp
    src/std/converter.cpp
    src/std/numeric.cpp
    src/std/std_backend.cpp
    src/util/codecvt_converter.cpp
    src/util/default_locale.cpp
    src/util/encoding.cpp
    src/util/gregorian.cpp
    src/util/info.cpp
    src/util/locale_data.cpp
)

END()

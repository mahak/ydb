#pragma once

#include "public.h"

#include <yt/yt/core/logging/log.h>

namespace NYT::NNodeTrackerClient {

////////////////////////////////////////////////////////////////////////////////

YT_DEFINE_GLOBAL(const NLogging::TLogger, NodeTrackerClientLogger, "NodeTrackerClient");

////////////////////////////////////////////////////////////////////////////////

} // namespace NYT::NNodeTrackerClient

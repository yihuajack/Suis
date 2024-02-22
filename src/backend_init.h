#include <filesystem>

#include "Config.h"

namespace OpenSCSim {
    const std::filesystem::path OPENSCSIM_ROOT = "..";
    const std::filesystem::path default_config = OPENSCSIM_ROOT / "openscsim.ini";
    OpenSCSimConfig config;
}

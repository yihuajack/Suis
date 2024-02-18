#include <filesystem>
#include "../Parameter.h"

// ELECTRONIC_ONLY:
// 0 = runs full equilibrate protocol
// 1 = skips ion equilibration
void equilibrate(Parameter &par, bool electronic_only = false);

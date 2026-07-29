// Anchor TU for header-only host modules that need a compile unit in the static lib.
#include "bms_adc_pipeline.hpp"
#include "bms_can1_scheduler.hpp"
#include "bms_can2_filter.hpp"
#include "bms_dma_circular.hpp"
#include "bms_pwm_controller.hpp"

namespace bms::logic {
void peripheral_host_link_anchor() {}
}

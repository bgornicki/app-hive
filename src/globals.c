#include "globals.h"
#include "os.h"
#include "ux.h"

uint8_t G_io_seproxyhal_spi_buffer[IO_SEPROXYHAL_BUFFER_SIZE_B];
io_state_e G_io_state;
ux_state_t G_ux;
bolos_ux_params_t G_ux_params;
global_ctx_t G_context;
const settings_t N_settings_nvram;

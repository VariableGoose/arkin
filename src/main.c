#include "arkin_core.h"
#include "arkin_log.h"

i32_t main(void) {
    arkin_init(&(arkin_core_desc_t) {0});
    al_add_fp(AL_LOG_LEVEL_TRACE, true, stdout);

    al_fatal("%d", 42);
    al_error("hehe");
    al_warn("hehe");
    al_info("hehe");
    al_debug("hehe");
    al_trace("hehe");

    arkin_terminate();
    return 0;
}

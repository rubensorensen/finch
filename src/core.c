#include "core.h"

static LgLogger global_engine_logger;
static LgLogger global_application_logger;

LgLogger* fc_get_engine_logger()
{
    return &global_engine_logger;
}

LgLogger* fc_get_application_logger()
{
    return &global_application_logger;
}

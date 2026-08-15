#pragma once
#include "draw.h"


void init_input_callbacks(rendering_context_t *rctx);
void set_input_callbacks(rendering_context_t *rctx);
void restore_input_callbacks(rendering_context_t *rctx);
void handle_input(rendering_context_t *rctx);
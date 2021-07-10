#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// this is the bridge function, jump to dl functions:
// 1. save original lr address to the stack
// 2. store fake_lr_address to the lr address
// 3. jump to execute real_function
// so the dl functions will use the fake_lr_address as the lr address, and allow this execution.
void *dlfcn_trampoline(const void *arg0, const void *arg1,
                       const void *fake_lr_address, const void *real_function);

#ifdef __cplusplus
}
#endif
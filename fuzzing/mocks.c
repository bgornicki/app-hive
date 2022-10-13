#include "cx.h"

void __wrap_os_longjmp(unsigned int exception) {
    longjmp(try_context_get()->jmp_buf, exception);
}

try_context_t *current_context = NULL;
try_context_t *try_context_get(void) {
    return current_context;
}

try_context_t *try_context_set(try_context_t *ctx) {
    try_context_t *previous_ctx = current_context;
    current_context = ctx;
    return previous_ctx;
}

void *__wrap_pic(void *link_address) {
    return link_address;
}

cx_err_t __wrap_cx_hash_no_throw(cx_hash_t *hash, uint32_t mode, const uint8_t *in, size_t len, uint8_t *out, size_t out_len) {
    return CX_OK;
}

size_t __wrap_cx_hash_get_size(int fd) {
    return 32;
}

cx_err_t __wrap_cx_ripemd160_init_no_throw(cx_ripemd160_t *hash) {
    return CX_OK;
}

int __wrap_cx_ecdsa_sign ( const cx_ecfp_private_key_t * pvkey, int mode, cx_md_t hashID, const unsigned char * hash, unsigned int hash_len, unsigned char * sig, unsigned int sig_len, unsigned int * info ) {
    return CX_OK;
}

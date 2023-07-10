#include "os_io_seproxyhal.h"
#include "os.h"
#include "cx.h"
#include "ux.h"

ux_state_t G_ux;
bolos_ux_params_t G_ux_params;
unsigned char G_io_seproxyhal_spi_buffer[IO_SEPROXYHAL_BUFFER_SIZE_B];

extern unsigned int app_stack_canary;
#define STACK_CANARY (*((volatile uint32_t*) &app_stack_canary))

#define SIGN_HASH_SIZE 32
uint8_t payload_hash[SIGN_HASH_SIZE];

void run_test(void);
void exit_app(void);

__attribute__((section(".boot"))) int main(void) {
    // exit critical section
    __asm volatile("cpsie i");

    // ensure exception will work as planned
    os_boot();

#if 0
    UX_INIT();

    for (;;) {
      BEGIN_TRY {
        TRY {
          io_seproxyhal_init();

#ifdef TARGET_NANOX
          // grab the current plane mode setting
          // requires "--appFlag 0x240" to be set in makefile
          G_io_app.plane_mode = os_setting_get(OS_SETTING_PLANEMODE, NULL, 0);
#endif  // TARGET_NANOX

          USB_power(0);
          USB_power(1);

#ifdef HAVE_BLE
          BLE_power(0, NULL);
          BLE_power(1, "Nano X");
#endif  // HAVE_BLE

          run_test();
        }
        CATCH(EXCEPTION_IO_RESET) {
          // reset IO and UX
          continue;
        }
        CATCH_OTHER(e) {
          break;
        }
        FINALLY {
        }
      }
      END_TRY;
    }
#else
    run_test();
#endif
    exit_app();
}


void
print_stack(const char *ckpt)
{
    size_t i;

    PRINTF("[DEBUG]  ckpt %s\n", ckpt);
    PRINTF("[PTR]    stack cur: 0x%p\n", &i);
    PRINTF("[PTR]    stack: 0x%p\n", &app_stack_canary);
    PRINTF("[CANARY] stack canary: 0x%x\n", STACK_CANARY);
}

#define STACKSEARCH	256
static void
print_memory(const char *pfix)
{
    size_t i;
    volatile uint32_t *volatile p;
    uint32_t last;

    p = &last;

    for (i=STACKSEARCH; i > 0; i--)
	if (*(p-i))
	    PRINTF("[%s] %04u *(0x%p) = 0x%08x\n", pfix, i, p-i, *(p-i));
}

static void
call_it(cx_ecfp_private_key_t *pk)
{
    size_t ret;
    size_t sig_len = 100;
    unsigned int info;

    PRINTF("[DEBUG] payload_hash = 0x%p\n", payload_hash);
    print_stack("enter call_it");
    print_memory("BEFORE");
    ret = cx_ecdsa_sign_no_throw(pk, CX_LAST | CX_RND_RFC6979, CX_SHA256,
                                 payload_hash, sizeof(payload_hash),
                                 &G_io_apdu_buffer[32], &sig_len, &info);
    PRINTF("[DEBUG] cx_ecdsa_sign() returned %u, %u, %u\n", ret, sig_len, info);
    print_memory("AFTER");
    print_stack("leave call_it");
}

void
run_test(void)
{
    cx_ecfp_private_key_t pk;
    size_t i;

    STACK_CANARY = 0xDEADBEEF;

    print_stack("ENTER THE DRAGON!");

    for (i=0; i < sizeof(payload_hash); i++)
	payload_hash[i] = i;

    pk.curve = CX_CURVE_SECP256K1;
    pk.d_len = 32;
    pk.d[0]  = 0x34; pk.d[1]  = 0xac; pk.d[2]  = 0x5d; pk.d[3]  = 0x78;
    pk.d[4]  = 0x4e; pk.d[5]  = 0xbb; pk.d[6]  = 0x4d; pk.d[7]  = 0xf4;
    pk.d[8]  = 0x72; pk.d[9]  = 0x7b; pk.d[10] = 0xcd; pk.d[11] = 0xdf;
    pk.d[12] = 0x6a; pk.d[13] = 0x67; pk.d[14] = 0x43; pk.d[15] = 0xf5;
    pk.d[16] = 0xd5; pk.d[17] = 0xd4; pk.d[18] = 0x6d; pk.d[19] = 0x83;
    pk.d[20] = 0xdd; pk.d[21] = 0x74; pk.d[22] = 0xaa; pk.d[23] = 0x82;
    pk.d[24] = 0x58; pk.d[25] = 0x66; pk.d[26] = 0x39; pk.d[27] = 0x0c;
    pk.d[28] = 0x69; pk.d[29] = 0x4f; pk.d[30] = 0x29; pk.d[31] = 0x38;

    call_it(&pk);
}

void
exit_app(void)
{
    BEGIN_TRY_L(exit) {
	TRY_L(exit) {
	    os_sched_exit(-1);
	}
	FINALLY_L(exit) {
	}
    }
    END_TRY_L(exit);
    THROW(0);
}

#if 0
unsigned short io_exchange_al(unsigned char channel, unsigned short tx_len) {
    switch (channel & ~(IO_FLAGS)) {
        case CHANNEL_KEYBOARD:
            break;

        case CHANNEL_SPI:
            if (tx_len) {
                io_seproxyhal_spi_send(G_io_apdu_buffer, tx_len);

                if (channel & IO_RESET_AFTER_REPLIED) {
                    reset();
                }
                return 0;  // nothing received from the master so far (it's a tx
                           // transaction)
            } else {
                return io_seproxyhal_spi_recv(G_io_apdu_buffer, sizeof(G_io_apdu_buffer), 0);
            }

        default:
            THROW(INVALID_PARAMETER);
    }
    return 0;
}
#endif

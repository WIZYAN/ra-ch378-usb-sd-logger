/* generated vector source file - do not edit */
#include "bsp_api.h"
/* Do not build these data structures if no interrupts are currently allocated because IAR will have build errors. */
#if VECTOR_DATA_IRQ_COUNT > 0
        BSP_DONT_REMOVE const fsp_vector_t g_vector_table[BSP_ICU_VECTOR_MAX_ENTRIES] BSP_PLACE_IN_SECTION(BSP_SECTION_APPLICATION_VECTORS) =
        {
                        [0] = r_icu_isr, /* ICU IRQ10 (External pin interrupt 10) */
            [1] = sci_uart_rxi_isr, /* SCI2 RXI (Received data full) */
            [2] = sci_uart_txi_isr, /* SCI2 TXI (Transmit data empty) */
            [3] = sci_uart_tei_isr, /* SCI2 TEI (Transmit end) */
            [4] = sci_uart_eri_isr, /* SCI2 ERI (Receive error) */
            [5] = gpt_counter_overflow_isr, /* GPT1 COUNTER OVERFLOW (Overflow) */
        };
        const bsp_interrupt_event_t g_interrupt_event_link_select[BSP_ICU_VECTOR_MAX_ENTRIES] =
        {
            [0] = BSP_PRV_IELS_ENUM(EVENT_ICU_IRQ10), /* ICU IRQ10 (External pin interrupt 10) */
            [1] = BSP_PRV_IELS_ENUM(EVENT_SCI2_RXI), /* SCI2 RXI (Received data full) */
            [2] = BSP_PRV_IELS_ENUM(EVENT_SCI2_TXI), /* SCI2 TXI (Transmit data empty) */
            [3] = BSP_PRV_IELS_ENUM(EVENT_SCI2_TEI), /* SCI2 TEI (Transmit end) */
            [4] = BSP_PRV_IELS_ENUM(EVENT_SCI2_ERI), /* SCI2 ERI (Receive error) */
            [5] = BSP_PRV_IELS_ENUM(EVENT_GPT1_COUNTER_OVERFLOW), /* GPT1 COUNTER OVERFLOW (Overflow) */
        };
        #endif

/**************************************************************************//**
 * @file     uart_transfer.c
 * @version  V1.00
 * $Date: 20/06/30 2:21p $
 * @brief    General UART ISP slave Sample file
 *
 * @note
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (C) 2017 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/

/*!<Includes */
#include <string.h>
#include "NuMicro.h"
#include "targetdev.h"
#include "uart_transfer.h"

#ifdef __ICCARM__
#pragma data_alignment=4
uint8_t uart_rcvbuf[MAX_PKT_SIZE] = {0};
#else
__attribute__((aligned(4))) uint8_t uart_rcvbuf[MAX_PKT_SIZE] = {0};
#endif

uint8_t volatile bUartDataReady = 0;
uint8_t volatile bufhead = 0;


/* please check "targetdev.h" for chip specifc define option */

/*---------------------------------------------------------------------------------------------------------*/
/* INTSTS to handle UART Channel 0 interrupt event                                                            */
/*---------------------------------------------------------------------------------------------------------*/
void UART_ISP_IRQHandler(void)
{
    /*----- Determine interrupt source -----*/
    uint32_t u32IntSrc = UART_ISP->INTSTS;

    if (u32IntSrc & 0x11)   /*RDA FIFO interrupt & RDA timeout interrupt*/
    {
        while (((UART_ISP->FIFOSTS & UART_FIFOSTS_RXEMPTY_Msk) == 0) && (bufhead < MAX_PKT_SIZE))      /*RX fifo not empty*/
        {
            uart_rcvbuf[bufhead++] = UART_ISP->DAT;
        }
    }

    if (bufhead == MAX_PKT_SIZE)
    {
        bUartDataReady = TRUE;
        bufhead = 0;
    }
    else if (u32IntSrc & 0x10)
    {
        bufhead = 0;
    }
}
#ifdef __ICCARM__
#pragma data_alignment=4
extern uint8_t response_buff[64];
#else
extern __attribute__((aligned(4))) uint8_t response_buff[64];
#endif

void PutString(void)
{
    uint32_t i;

    for (i = 0; i < MAX_PKT_SIZE; i++)
    {
        while ((UART_ISP->FIFOSTS & UART_FIFOSTS_TXFULL_Msk));

        UART_ISP->DAT = response_buff[i];
    }
}

void UART_Init()
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init UART                                                                                               */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Select UART function mode */
    UART_ISP->FUNCSEL = ((UART_ISP->FUNCSEL & (~UART_FUNCSEL_FUNCSEL_Msk)) | UART_FUNCSEL_MODE);
    /* Set UART line configuration */
    UART_ISP->LINE = UART_WORD_LEN_8 | UART_PARITY_NONE | UART_STOP_BIT_1;
    /* Set UART Rx and RTS trigger level */
    UART_ISP->FIFO = UART_FIFO_RFITL_14BYTES | UART_FIFO_RTSTRGLV_1BYTE;
    /* Set UART baud rate */
    UART_ISP->BAUD = (UART_BAUD_MODE2 | UART_BAUD_MODE2_DIVIDER(__HIRC, 115200));
    /* Set time-out interrupt comparator */
    UART_ISP->TOUT = (UART_ISP->TOUT & ~UART_TOUT_TOIC_Msk) | (0x40);
    NVIC_SetPriority(UART_ISP_IRQn, 2);
    NVIC_EnableIRQ(UART_ISP_IRQn);
    /* 0x0811 */
    UART_ISP->INTEN = (UART_INTEN_TOCNTEN_Msk | UART_INTEN_RXTOIEN_Msk | UART_INTEN_RDAIEN_Msk);
}


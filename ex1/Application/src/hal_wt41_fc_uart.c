#include "hal_wt41_fc_uart.h"
#include "tools.h"

/*
Port B SPI
Port D USART1
Port H USART2
Port J USART3
every USART can operate also in SPI mode
*/

/*
 This functions initializes UART3 as listed above, and prepares the ringbuffer of the receiving
part. It also resets the Bluetooth module by pulling the reset pin PJ5 low for 5 ms. Whenever the
module receives a character from the Bluetooth module it has to be put it into a ringbuffer. The
buffer should be able to hold at least 32 bytes. For every character put in the ringbuffer, the
rcvCallback callback function must be called. Re-enable the interrupts before calling the callback
function. Make sure you do not call rcvCallback again before the previous call has returned.
If there are less than 5 bytes free in the buffer, the HAL module should trigger the flow control,
by setting CTS to high, to indicate that it currently cannot handle anymore data. If the buffer gets
at least half empty (less than 16 bytes stored in the case of a 32 byte buffer) the module should
release flow control by setting CTS to low.*/
error_t halWT41FcUartInit(void (*sndCallback)(), void (*rcvCallback)(uint8_t)) { return SUCCESS; }

/*
error_t halWT41FcUartSend(uint8_t byte);
This function should send the byte given as a parameter to the Bluetooth module. The corresponding
sndCallback callback function will be called when the byte has been copied into the shift register
of the UART, i.e., the byte is currently being sent to the Bluetooth module. You have to ensure that
the callback is not called if there was no preceding send, e.g., after a reset. When the WT 41
Bluetooth module sets RTS to high, no more data must be sent to the module.6 When the Bluetooth
module clears RTS, transmission of data to the module can resume. While the check for the low RTS
pin can be done right before a character is sent (copied into the UART data buffer), the recognition
of the change on the RTS pin from high to low has to be done interrupt-driven using a pin change
interrupt. In case halWT41FcUartSend is called while the reset of the WT 41 Bluetooth module is
still performed, the data bytes has to be buffered. After the reset has concluded, the buffered
bytes has to be sent and sndCallback called. This approach must also be taken when the hardware flow
control of the WT 41 (RTS) is active. The Bluetooth stack will send data byte-wise and continue with
the next byte only when sndCallback is called, so no extra buffering needs to be done.
*/
error_t halWT41FcUartSend(uint8_t byte) { return SUCCESS; }

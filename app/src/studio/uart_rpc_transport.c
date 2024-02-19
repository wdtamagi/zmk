/*
 * Copyright (c) 2024 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/sys/ring_buffer.h>

#include <zephyr/logging/log.h>
#include <zmk/studio/rpc.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

/* change this to any other UART peripheral if desired */
#define UART_DEVICE_NODE DT_CHOSEN(zmk_studio_rpc_uart)

static const struct device *const uart_dev = DEVICE_DT_GET(UART_DEVICE_NODE);

static void tx_notify(struct ring_buf *tx_ring_buf, size_t written, bool msg_done,
                      void *user_data) {
    if (msg_done || (ring_buf_size_get(tx_ring_buf) > (ring_buf_capacity_get(tx_ring_buf) / 2))) {
        uart_irq_tx_enable(uart_dev);
    }
}

static bool handling_rx = false;

static int start_rx() {
    handling_rx = true;
    return 0;
}

static int stop_rx(void) {
    handling_rx = false;
    return 0;
}

ZMK_RPC_TRANSPORT(uart, ZMK_TRANSPORT_USB, start_rx, stop_rx, NULL, tx_notify);

/*
 * Read characters from UART until line end is detected. Afterwards push the
 * data to the message queue.
 */
static void serial_cb(const struct device *dev, void *user_data) {
    if (!uart_irq_update(uart_dev)) {
        return;
    }

    if (uart_irq_rx_ready(uart_dev) && handling_rx) {
        /* read until FIFO empty */
        uint32_t last_read = 0;
        struct ring_buf *buf = zmk_rpc_get_rx_buf();
        do {
            uint8_t *buffer;
            uint32_t len = ring_buf_put_claim(buf, &buffer, buf->size);
            if (len == 0) {
                zmk_rpc_rx_notify();
                continue;
            }
            last_read = uart_fifo_read(uart_dev, buffer, len);

            ring_buf_put_finish(buf, last_read);
        } while (last_read > 0);

        zmk_rpc_rx_notify();
    }

    if (uart_irq_tx_ready(uart_dev)) {
        struct ring_buf *tx_buf = zmk_rpc_get_tx_buf();
        uint8_t len;
        while ((len = ring_buf_size_get(tx_buf)) > 0) {
            uint8_t *buf;
            uint8_t claim_len = ring_buf_get_claim(tx_buf, &buf, tx_buf->size);

            if (claim_len == 0) {
                continue;
            }

            int sent = uart_fifo_fill(uart_dev, buf, claim_len);

            ring_buf_get_finish(tx_buf, MAX(sent, 0));
        }
    }
}

static int uart_rpc_interface_init(void) {
    if (!device_is_ready(uart_dev)) {
        LOG_ERR("UART device not found!");
        return -ENODEV;
    }

    /* configure interrupt and callback to receive data */
    int ret = uart_irq_callback_user_data_set(uart_dev, serial_cb, NULL);

    if (ret < 0) {
        if (ret == -ENOTSUP) {
            printk("Interrupt-driven UART API support not enabled\n");
        } else if (ret == -ENOSYS) {
            printk("UART device does not support interrupt-driven API\n");
        } else {
            printk("Error setting UART callback: %d\n", ret);
        }
        return ret;
    }

    uart_irq_rx_enable(uart_dev);

    return 0;
}

SYS_INIT(uart_rpc_interface_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);

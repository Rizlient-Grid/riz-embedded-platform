#include "riz/hal/errcode.h"
#include "riz/hal/uart_observer.h"
#include <riz/hal/uart.h>

#include <stm32h7xx_hal.h>

#include <cstdint>
#include <utility>

using namespace riz::hal;

static IRQn_Type to_irqn(USART_TypeDef* inst) noexcept {
    if (inst == USART1) return USART1_IRQn;
    else if (inst == USART2) return USART2_IRQn;
    else if (inst == USART3) return USART3_IRQn;
    else if (inst == UART4) return UART4_IRQn;
    else if (inst == UART5) return UART5_IRQn;
    else if (inst == USART6) return USART6_IRQn;
    else if (inst == UART7) return UART7_IRQn;
    else if (inst == UART8) return UART8_IRQn;
    else if (inst == LPUART1) return LPUART1_IRQn;
    return NonMaskableInt_IRQn;
}

void uart::set_observer(uart_observer* obs) noexcept {
    observer_ = obs;
}

void uart::enable_irq() noexcept {
    auto* huart = static_cast<UART_HandleTypeDef*>(dev_);
    __HAL_UART_CLEAR_IDLEFLAG(huart);
    __HAL_UART_ENABLE_IT(huart, UART_IT_IDLE);
    HAL_NVIC_EnableIRQ(to_irqn(huart->Instance));
}

void uart::disable_irq() noexcept {
    auto* huart = static_cast<UART_HandleTypeDef*>(dev_);
    __HAL_UART_DISABLE_IT(huart, UART_IT_IDLE);
    HAL_NVIC_DisableIRQ(to_irqn(huart->Instance));
}

int uart::start_transmit(const void* data, std::size_t len) noexcept {
    auto* huart = static_cast<UART_HandleTypeDef*>(dev_);
    auto tx_data = static_cast<uint8_t*>(const_cast<void*>(data));
    HAL_StatusTypeDef rc = HAL_ERROR;
    switch (mode_) {
    case transfer_mode::dma:
        rc = HAL_UART_Transmit_DMA(huart, tx_data, len);
        break;
    case transfer_mode::it:
        rc = HAL_UART_Transmit_IT(huart, tx_data, len);
        break;
    default:
        assert(false);
    }
    return (rc == HAL_OK) ? 0 : -1;
}

int uart::abort_transmit() noexcept {
    auto* huart = static_cast<UART_HandleTypeDef*>(dev_);
    return (HAL_UART_AbortTransmit(huart) == HAL_OK) ? 0 : -1;
}

int uart::start_receive(void* data, std::size_t len) noexcept {
    auto* huart = static_cast<UART_HandleTypeDef*>(dev_);
    HAL_StatusTypeDef rc = HAL_ERROR;
    switch (mode_) {
    case transfer_mode::dma:
        rc = HAL_UART_Receive_DMA(huart, static_cast<uint8_t*>(data), len);
        break;
    case transfer_mode::it:
        rc = HAL_UART_Receive_IT(huart, static_cast<uint8_t*>(data), len);
        break;
    default:
        assert(false);
    }
    return (rc == HAL_OK) ? 0 : -1;
}

void uart::on_transmit_complete_isr() noexcept {
    if (observer_) observer_->on_tx_complete();
}

void uart::on_receive_complete_isr() noexcept {
    if (!observer_) return;
    observer_->on_rx_complete(rx_buffer_ + rx_read_offset_, rx_buffer_size_ - rx_read_offset_);
    rx_read_offset_ = 0;
}

void uart::on_receive_idle_isr() noexcept {
    if (!observer_) return;
    auto* huart = static_cast<UART_HandleTypeDef*>(dev_);
    std::size_t remaining = __HAL_DMA_GET_COUNTER(huart->hdmarx);
    std::size_t write_pos = rx_buffer_size_ - remaining;
    if (write_pos <= rx_read_offset_) return;
    observer_->on_rx_idle(rx_buffer_ + rx_read_offset_, write_pos - rx_read_offset_);
    rx_read_offset_ = write_pos;
}

void uart::on_error_isr() noexcept {
    if (!observer_) return;
    auto* huart = static_cast<UART_HandleTypeDef*>(dev_);
    uint32_t flags = huart->ErrorCode;
    errcode err = errcode::ok;
    if (flags & HAL_UART_ERROR_PE)   err |= errcode::parity;
    if (flags & HAL_UART_ERROR_NE)   err |= errcode::noise;
    if (flags & HAL_UART_ERROR_FE)   err |= errcode::framing;
    if (flags & HAL_UART_ERROR_ORE)  err |= errcode::overflow;
    if (flags & HAL_UART_ERROR_DMA)  err |= errcode::dma;
    if (err != errcode::ok) {
        observer_->on_rx_error(err);
    }
}

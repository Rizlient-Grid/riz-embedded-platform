#include "riz/errcode.h"
#include "riz/hal/uart_observer.h"
#include <riz/hal/uart.h>

#include <stm32h7xx_hal.h>

#include <cstdint>
#include <utility>

using namespace riz::hal;

static IRQn_Type to_irqn(USART_TypeDef* inst) noexcept {
    if (inst == USART1)
        return USART1_IRQn;
    else if (inst == USART2)
        return USART2_IRQn;
    else if (inst == USART3)
        return USART3_IRQn;
    else if (inst == UART4)
        return UART4_IRQn;
    else if (inst == UART5)
        return UART5_IRQn;
    else if (inst == USART6)
        return USART6_IRQn;
    else if (inst == UART7)
        return UART7_IRQn;
    else if (inst == UART8)
        return UART8_IRQn;
    else if (inst == LPUART1)
        return LPUART1_IRQn;
    return NonMaskableInt_IRQn;
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

std::size_t uart::get_rx_transfer_remaining() noexcept {
    auto* huart = static_cast<UART_HandleTypeDef*>(dev_);
    switch (mode_) {
    case transfer_mode::dma:
        return __HAL_DMA_GET_COUNTER(huart->hdmarx);
    case transfer_mode::it:
        return huart->RxXferCount;
    }
    return 0;
}

riz::errcode uart::get_and_clear_hw_error() noexcept {
    auto huart = static_cast<UART_HandleTypeDef*>(dev_);
    uint32_t flags = huart->ErrorCode;
    huart->ErrorCode = HAL_UART_ERROR_NONE;
    if (flags & HAL_UART_ERROR_DMA) {
        return errcode::uart_dma;
    }
    if (flags & HAL_UART_ERROR_ORE) {
        return errcode::uart_overflow;
    }
    if (flags & HAL_UART_ERROR_FE) {
        return errcode::uart_framing;
    }
    if (flags & HAL_UART_ERROR_PE) {
        return errcode::uart_parity;
    }
    if (flags & HAL_UART_ERROR_NE) {
        return errcode::uart_noise;
    }
    return errcode::success;
}

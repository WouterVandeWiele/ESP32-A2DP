
#include "BluetoothA2DPSinkQueued.h"

#if A2DP_I2S_SUPPORT

void ccall_i2s_task_handler(void *arg) {
  ESP_LOGD(BT_TAG, "%s", __func__);
  if (actual_bluetooth_a2dp_sink)
    static_cast<BluetoothA2DPSinkQueued*>(actual_bluetooth_a2dp_sink)->i2s_task_handler(arg);
}


void BluetoothA2DPSinkQueued::bt_i2s_task_start_up(void) {
    ESP_LOGI(BT_TAG, "ringbuffer data empty! mode changed: RINGBUFFER_MODE_PREFETCHING");
    ringbuffer_mode = RINGBUFFER_MODE_PREFETCHING;
    if ((s_i2s_write_semaphore = xSemaphoreCreateBinary()) == NULL) {
        ESP_LOGE(BT_TAG, "%s, Semaphore create failed", __func__);
        return;
    }
    if ((s_ringbuf_i2s = xRingbufferCreate(i2s_ringbuffer_size, RINGBUF_TYPE_BYTEBUF)) == NULL) {
        ESP_LOGE(BT_TAG, "%s, ringbuffer create failed", __func__);
        return;
    }
    // if no specific priority has been defined
    if (i2s_task_priority==0){
        i2s_task_priority = task_priority;
    }
    ESP_LOGI(BT_TAG, "BtI2STask priority: %d", i2s_task_priority);
    //xTaskCreate(bt_i2s_task_handler, "BtI2STask", 2048, NULL, configMAX_PRIORITIES - 3, &s_bt_i2s_task_handle);
    BaseType_t result = xTaskCreatePinnedToCore(ccall_i2s_task_handler, "BtI2STask", i2s_stack_size, NULL, i2s_task_priority, &s_bt_i2s_task_handle, task_core);
    if (result!=pdPASS){
        ESP_LOGE(BT_TAG, "xTaskCreatePinnedToCore");
    } else {
        ESP_LOGI(BT_TAG, "BtI2STask Started");
    }
}

void BluetoothA2DPSinkQueued::bt_i2s_task_shut_down(void) {
    if (s_bt_i2s_task_handle) {
        vTaskDelete(s_bt_i2s_task_handle);
        s_bt_i2s_task_handle = nullptr;
    }
    if (s_ringbuf_i2s) {
        vRingbufferDelete(s_ringbuf_i2s);
        s_ringbuf_i2s = nullptr;
    }
    if (s_i2s_write_semaphore) {
        vSemaphoreDelete(s_i2s_write_semaphore);
        s_i2s_write_semaphore = NULL;
    }

    ESP_LOGI(BT_TAG, "BtI2STask shutdown");
}

/* NEW I2S Task & ring buffer */

void BluetoothA2DPSinkQueued::i2s_task_handler(void *arg) {
    uint8_t *data = NULL;
    size_t item_size = 0;
    /**
     * The total length of DMA buffer of I2S is:
     * `dma_frame_num * dma_desc_num * i2s_channel_num * i2s_data_bit_width / 8`.
     * Transmit `dma_frame_num * dma_desc_num` bytes to DMA is trade-off.
     */
    size_t bytes_written = 0;
    is_starting = true;

    while (true) {
        if (is_starting){
            // wait for ringbuffer to be filled
            if (pdTRUE != xSemaphoreTake(s_i2s_write_semaphore, portMAX_DELAY)){
                continue;
            }
            is_starting = false;
        }
        // xSemaphoreTake was succeeding here, so we have the buffer filled up
        item_size = 0;

        // receive data from ringbuffer and write it to I2S DMA transmit buffer 
        data = (uint8_t *)xRingbufferReceiveUpTo(s_ringbuf_i2s, &item_size, (TickType_t)pdMS_TO_TICKS(i2s_ticks), i2s_write_size_upto);
        if (item_size == 0) {
            ESP_LOGI(BT_TAG, "ringbuffer underflowed! mode changed: RINGBUFFER_MODE_PREFETCHING");
            ringbuffer_mode = RINGBUFFER_MODE_PREFETCHING;
            continue;
        } 

        // if i2s is not active we just consume the buffer w/o output
        if (is_i2s_active){
            size_t written = i2s_write_data(data, item_size);
#if A2DP_DEBUG_AUDIO
            ESP_LOGD(BT_TAG, "i2s_task_handler: %d->%d", item_size, written);
#endif
            if (written==0){
                ESP_LOGE(BT_TAG, "i2s_write_data failed %d->%d", item_size, written);
                continue;
            }
        }

        yield();
        vRingbufferReturnItem(s_ringbuf_i2s, (void *)data);
    }
}

size_t BluetoothA2DPSinkQueued::write_audio(const uint8_t *data, size_t size)
{
#if A2DP_DEBUG_AUDIO
    ESP_LOGD(BT_TAG, "write_audio: %d", size); 
#endif
    size_t item_size = 0;
    BaseType_t done = pdFALSE;

    // This should not really happen!
    if (!is_i2s_active){
        ESP_LOGW(BT_TAG, "i2s is not active: we try to activate it");
        set_i2s_active(true);
        yield();
    }

    if (ringbuffer_mode == RINGBUFFER_MODE_DROPPING) {
        ESP_LOGW(BT_TAG, "ringbuffer is full, drop this packet!");
        vRingbufferGetInfo(s_ringbuf_i2s, NULL, NULL, NULL, NULL, &item_size);
        if (item_size <= i2s_ringbuffer_prefetch_size()) {
            ESP_LOGI(BT_TAG, "ringbuffer data decreased! mode changed: RINGBUFFER_MODE_PROCESSING");
            ringbuffer_mode = RINGBUFFER_MODE_PROCESSING;
        }
        return 0;
    }

    done = xRingbufferSend(s_ringbuf_i2s, (void *)data, size, (TickType_t)pdMS_TO_TICKS(i2s_ticks));

    if (!done) {
        ESP_LOGW(BT_TAG, "ringbuffer overflowed, ready to decrease data! mode changed: RINGBUFFER_MODE_DROPPING");
        ringbuffer_mode = RINGBUFFER_MODE_DROPPING;
    }

    if (ringbuffer_mode == RINGBUFFER_MODE_PREFETCHING) {
        vRingbufferGetInfo(s_ringbuf_i2s, NULL, NULL, NULL, NULL, &item_size);
        if (item_size >= i2s_ringbuffer_prefetch_size()) {
            ESP_LOGI(BT_TAG, "ringbuffer data increased! mode changed: RINGBUFFER_MODE_PROCESSING");
            ringbuffer_mode = RINGBUFFER_MODE_PROCESSING;
            if (pdFALSE == xSemaphoreGive(s_i2s_write_semaphore)) {
                ESP_LOGE(BT_TAG, "semphore give failed");
            }
        }
    }

    return done ? size : 0;
}

#endif
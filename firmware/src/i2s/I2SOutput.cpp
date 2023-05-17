#include <Arduino.h>
#include "driver/i2s.h"
#include <math.h>
#include <SPIFFS.h>
#include <FS.h>
#include "I2SOutput.h"
#include "SampleSource.h"

// number of frames to try and send at once (a frame is a left and right sample)
#define NUM_FRAMES_TO_SEND 124

TaskHandle_t I2SOutput::m_i2sWriterTaskHandle;
QueueHandle_t I2SOutput::m_i2sQueue;
i2s_port_t I2SOutput::m_i2sPort;

Voice *I2SOutput::m_sample_generator = nullptr;
void *I2SOutput::mEventHandler = nullptr;

void IRAM_ATTR i2sWriterTask(void *param)
{
    int availableBytes = 0;
    int buffer_position = 0;
    Frame_t *frames = (Frame_t *)malloc(sizeof(Frame_t) * NUM_FRAMES_TO_SEND);
    while (true)
    {
        // wait for some data to be requested
        i2s_event_t evt;
        if (xQueueReceive(I2SOutput::m_i2sQueue, &evt, portMAX_DELAY) == pdPASS)
        {
            if (evt.type == I2S_EVENT_TX_DONE)
            {
                size_t bytesWritten = 0;
                do
                {
                    if (availableBytes == 0)
                    {
                        // get some frames from the wave file - a frame consists of a 16 bit left and right sample
                        I2SOutput::m_sample_generator->getFrames(frames, NUM_FRAMES_TO_SEND);
                        // how maby bytes do we now have to send
                        availableBytes = NUM_FRAMES_TO_SEND * sizeof(uint32_t);
                        // reset the buffer position back to the start
                        buffer_position = 0;
                    }
                    // do we have something to write?
                    if (availableBytes > 0)
                    {
                        I2SOutput::dispatchEvent();
                        // write data to the i2s peripheral
                        i2s_write(I2SOutput::m_i2sPort, buffer_position + (uint8_t *)frames,
                                  availableBytes, &bytesWritten, portMAX_DELAY);
                        availableBytes -= bytesWritten;
                        buffer_position += bytesWritten;
                    }
                } while (bytesWritten > 0);
            }
        }
    }
}

void I2SOutput::start(i2s_port_t i2sPort, i2s_pin_config_t &i2sPins, Voice *sample_generator)
{
    m_sample_generator = sample_generator;
    // i2s config for writing both channels of I2S
    i2s_config_t i2sConfig = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = m_sample_generator->getSampleRate(),
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_LSB),
        .intr_alloc_flags = (ESP_INTR_FLAG_IRAM | ESP_INTR_FLAG_LEVEL3),
        .dma_buf_count = 4,
        .dma_buf_len = NUM_FRAMES_TO_SEND / 2,
        .use_apll = true};

    m_i2sPort = i2sPort;
    // install and start i2s driver
    i2s_driver_install(m_i2sPort, &i2sConfig, 2, &m_i2sQueue);
    // set up the i2s pins
    i2s_set_pin(m_i2sPort, &i2sPins);
    // clear the DMA buffers
    i2s_zero_dma_buffer(m_i2sPort);
    // start a task to write samples to the i2s peripheral
    TaskHandle_t writerTaskHandle;
    xTaskCreatePinnedToCore(i2sWriterTask, "i2s Task", 2048 * 5, NULL, configMAX_PRIORITIES - 1, &writerTaskHandle, 1); /// pinned on core 1
}

void I2SOutput::dispatchEvent()
{
    IChangeHandler *eventHandler = reinterpret_cast<IChangeHandler *>(mEventHandler);
    eventHandler->handleChangeEvent();
}
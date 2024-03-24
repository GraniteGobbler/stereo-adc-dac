#include <inttypes.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <string.h>
#include <encoder.h>
#include <esp_idf_lib_helpers.h>
#include <esp_log.h>

// Connect common encoder pin to ground
#if HELPER_TARGET_IS_ESP8266
#define RE0_A_GPIO   14
#define RE0_B_GPIO   12
#define RE0_BTN_GPIO 13

#elif HELPER_TARGET_IS_ESP32
// #define RE0_A_GPIO   9
// #define RE0_B_GPIO   10
// #define RE0_BTN_GPIO 11
// #define RE1_A_GPIO   12
// #define RE1_B_GPIO   13
// #define RE1_BTN_GPIO 14
#define RE0_A_GPIO   42
#define RE0_B_GPIO   41
#define RE0_BTN_GPIO 40
#define RE1_A_GPIO   39
#define RE1_B_GPIO   38
#define RE1_BTN_GPIO 37

#else
#error Unknown platform
#endif

#define EV_QUEUE_LEN 5

static const char *TAG = "encoder_example";

static QueueHandle_t event_queue;
static rotary_encoder_t re0;
static rotary_encoder_t re1;

void test(void *arg)
{
    // Create event queue for rotary encoders
    event_queue = xQueueCreate(EV_QUEUE_LEN, sizeof(rotary_encoder_event_t));

    // Setup rotary encoder library
    ESP_ERROR_CHECK(rotary_encoder_init(event_queue));

    // Add one encoder
    memset(&re0, 0, sizeof(rotary_encoder_t));
    re0.pin_a = RE0_A_GPIO;
    re0.pin_b = RE0_B_GPIO;
    re0.pin_btn = RE0_BTN_GPIO;
    ESP_ERROR_CHECK(rotary_encoder_add(&re0));

    // Add one encoder
    memset(&re1, 0, sizeof(rotary_encoder_t));
    re1.pin_a = RE1_A_GPIO;
    re1.pin_b = RE1_B_GPIO;
    re1.pin_btn = RE1_BTN_GPIO;
    ESP_ERROR_CHECK(rotary_encoder_add(&re1));

    rotary_encoder_event_t e;
    int32_t val0 = 0;
    int32_t val1 = 0;
    int32_t id = 0;

    ESP_LOGI(TAG, "Initial value: %" PRIi32, val0);
    while (1)
    {
        xQueueReceive(event_queue, &e, portMAX_DELAY);
        rotary_encoder_t *sender = e.sender;
        id = sender->index;

        switch (e.type)
        {
            case RE_ET_BTN_PRESSED:
                // ESP_LOGI(TAG, "Button pressed");
                ESP_LOGI(TAG, "Button pressed");
                break;
            case RE_ET_BTN_RELEASED:
                ESP_LOGI(TAG, "Button released");
                break;
            case RE_ET_BTN_CLICKED:
                if (id == 1){
                    ESP_LOGE(TAG, "Encoders status:");
                    ESP_LOGE(TAG, "%d: %d", (int)id - 1, (int)val0);
                    ESP_LOGE(TAG, "%d: %d", (int)id, (int)val1);
                    break;
                }
                ESP_LOGI(TAG, "Button clicked");
                break;
            case RE_ET_BTN_LONG_PRESSED:
                if (id == 1){
                    val0 = 0;
                    val1 = 0;
                    ESP_LOGW(TAG, "Encoders reset");
                    break;
                }
                ESP_LOGI(TAG, "Looooong pressed button");
                break;
            case RE_ET_CHANGED:
                // id = sender->index;
                if (id == 0){
                    val0 -= e.diff;
                    // ESP_LOGI(TAG, "Value = %" PRIi32, val0);
                    ESP_LOGI(TAG, "Encoder %d: %d", (int)id, (int)val0);
                }
                else if (id == 1){
                    val1 -= e.diff;
                    // ESP_LOGI(TAG, "Value = %" PRIi32, val1);
                    ESP_LOGI(TAG, "Encoder %d: %d", (int)id, (int)val1);
                }
                break;
            default:
                break;
        }
    }
}

void app_main()
{
    xTaskCreate(test, TAG, configMINIMAL_STACK_SIZE * 8, NULL, 5, NULL);
}
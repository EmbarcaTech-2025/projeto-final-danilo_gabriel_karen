## Modelos de task e parâmetros

```c
// Protótipos das funções das tarefas
void vTask1(void *pvParameters);

void vTask1(void *pvParameters) {
    (void) pvParameters; 

    for (;;) {
        printf("Testando\n");
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

// Main

int main() {
    stdio_init_all();

    xTaskCreate(vTask1, "NomeTask", 4096, NULL, 2, NULL);

    vTaskStartScheduler();
    return 0;
}
```


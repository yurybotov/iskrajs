# iskrajs
Amperka's Iskra JS board bootloader project
Deveoped on stm32f4-discovery - STM32F407VGT.
Targets:
- Iskra JS - STM32F405RGT
- Iskra JS mini - STM32F411CEU


коррекция ардуино

1. убрать bootloader.h/c
2. подменить хэндлы в USBSerial.h/cpp
3. убить подключение библиотек usb для FS
4. убить все ненужные (кроме F4) семейства контроллеров
5. добавить 411 (и заодно 407 для отладки)
6. Оставить 2 способа загрузки: stlink и MSD

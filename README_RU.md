# Fusion SpaceMouse

DIY SpaceMouse-подобный контроллер для Autodesk Fusion 360. Внутри — RP2040-плата, магнитный датчик TLV493D и механика с магнитами/пружинами. Устройство работает автономно как USB HID, без desktop-helper на компьютере.

Прошивка не использует нативный протокол 3Dconnexion: Fusion 360 на macOS не принимал raw SpaceMouse HID без 3DxWare/navlib. Поэтому текущая рабочая версия эмулирует обычную мышь и клавиатуру: орбита делается через `Shift + средняя кнопка мыши`, а отдельная кнопка отправляет `Cmd+Shift+N` для AddIn в Fusion 360.

## Что умеет

- Отклонение ручки — орбита модели в Fusion 360.
- Отпускание ручки — быстрый replay/recenter курсора.
- Короткое нажатие кнопки на GPIO27 — команда Home View через Fusion AddIn.
- Удержание кнопки на GPIO27 — повторная калибровка нейтрального положения.
- Защита от «залипания»: прошивка периодически отпускает Shift/MMB и имеет watchdog для длинного drag.
- Z-ось сейчас не используется для навигации: pan/zoom отключены ради предсказуемой орбиты.

## Документация

- [Настройка прошивки](Code/firmware/README.md)
- [Прошивка](docs/flashing.md)
- [Подключение](docs/wiring.md)
- [Сборка](docs/assembly.md)
- [Troubleshooting](docs/troubleshooting.md)
- [Пояснения к BOM](BOM/README.md)

Текущие элементы управления, параметры чувствительности и ограничения прошивки см. также в [`Code/firmware/README.md`](Code/firmware/README.md).

## Запчасти

Локальные ссылки вынесены в CSV:

- [`BOM/Parts_BOM.csv`](BOM/Parts_BOM.csv)
- [`BOM/Hardware_BOM.csv`](BOM/Hardware_BOM.csv)

Основной список:

| Деталь | Кол-во | Ссылка |
|---|---:|---|
| RP2040-плата / локальная ссылка на Raspberry Pi Pico | 1 | https://ozon.ru/t/JmF95Xa |
| TLV493D магнитометр | 1 | https://ozon.ru/t/q4Tvne1 |
| Dupont/STEMMA/Qwiic кабель | 1 | https://ozon.ru/t/hoPmWOA |
| Тактовые кнопки | 2 | https://ozon.ru/t/d7ea172 |
| Магниты 6×2 мм | 6 | https://ozon.ru/t/6duZeGv |
| Пружины сжатия/растяжения | 3+3 | https://ozon.ru/t/Ln5cTSh |
| Стальные шарики | комплект | https://ozon.ru/t/Qi6mr0i |
| Втулки/термовставки M2.5 | 24 | https://ozon.ru/t/vswyEtI |
| Винты M2.5 | 24 | https://ozon.ru/t/JmF9HoN |

Также нужны напечатанные детали из `CAD/STLs/`.

## Важное про плату RP2040

BOM содержит локальную ссылку на Raspberry Pi Pico, но текущая прошивка в репозитории настроена и проверена на **Adafruit QT Py RP2040**:

```text
rp2040:rp2040:adafruit_qtpy:usbstack=picosdk
```

TLV493D подключён по STEMMA QT к `Wire1`, адрес датчика — `0x5E`. Кнопка Home подключена между GPIO27 и GND.

Если вместо QT Py используется обычный Raspberry Pi Pico, проверьте распиновку, I2C-шину, GPIO для кнопки и FQBN в `arduino-cli`. Прошивку может потребоваться адаптировать под другую плату.

## Важное про втулки и винты

В оригинальной 3D-модели некоторые посадочные места под втулки оказались больше, чем нужно. В моей сборке часть M2.5-втулок держалась плохо, поэтому в этих местах пришлось поставить **M3-втулки и соответствующие M3-винты**.

Перед финальной сборкой проверьте отверстия на своих распечатках:

- если M2.5-втулка садится плотно — используйте M2.5;
- если втулка болтается — лучше поставить M3;
- не заказывайте только один тип крепежа, если не уверены в точности своей печати.

## Подключение

- TLV493D: STEMMA QT / `Wire1`, адрес `0x5E`.
- Home button: GPIO27 → GND.
- GPIO24 в этой сборке не используется: он оказался постоянно LOW.

## Прошивка

Скомпилировать:

```sh
arduino-cli compile --fqbn rp2040:rp2040:adafruit_qtpy:usbstack=picosdk Code/firmware
```

Залить и открыть serial monitor:

```sh
tools/flash_monitor.py Code/firmware --seconds 20
```

Финальный sketch находится здесь:

```text
Code/firmware/firmware.ino
```

## Fusion 360 AddIn для Home View

Прошивка отправляет `Cmd+Shift+N` при коротком нажатии кнопки на GPIO27. Удержание этой же кнопки запускает повторную калибровку нейтрального положения. Чтобы Home View работал в Fusion 360:

1. Скопируйте `Code/Fusion360 Add-in/Send Home` в папку Fusion 360 AddIns.
2. Запустите AddIn в Fusion 360.
3. Назначьте его команде shortcut `Cmd+Shift+N`.

AddIn вызывает:

```python
activeViewport.goHome(True)
```

## Структура репозитория

```text
Code/firmware/                   финальная прошивка
Code/Fusion360 Add-in/Send Home/ Fusion 360 AddIn для Home View
BOM/                             список деталей
CAD/                             STEP/STL файлы
docs/                            прошивка, подключение, сборка, диагностика
Images/                          изображения проекта
Resources/                       схема подключения
tools/flash_monitor.py           helper для прошивки и serial monitor
```

## Лицензия

Проект распространяется под CC BY-NC-SA 4.0. См. `LICENSE`.

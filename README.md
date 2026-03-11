# Mini Backend Platform

Консольный мини backend в стиле Redis, построенный с модульной архитектурой, потокобезопасностью.

## Архитектура проекта

Проект разделён на несколько логических слоёв:

- Core Layer — парсинг команд и orchestration (Engine, CommandParser).
- Storage Layer — in memory база данных с TTL и фоновым очистителем (Database, TTLManager).
- Cache Layer — LRU кэш для ускорения чтения (LRUCache).
- Limiter Layer — rate limiter для пользователей (RateLimiter).
- Models Layer — модель записи (Entry).
- Utils Layer — потокобезопасное логирование (Logger).
- App Layer — консольный интерфейс и запуск цикла команд (main.cpp).

## Возможности

- обработка команд `SET`, `GET`, `DELETE`, `EXPIRE`, `RATE`, `STATS`
- in memory хранилище с TTL?истечением
- фоновая очистка просроченных ключей
- LRU кэш с O(1) доступом
- rate limiting по пользователю
- статистика кэша (hits/misses, hit rate)
- потокобезопасное логирование с таймстампами

## Требования

Для запуска необходим:

- C++17
- g++

## Запуск проекта

Выполнить в терминале:

```bash
g++ -std=c++17 -pthread src/main.cpp -o backend.exe
./backend.exe
```

Пример команд:

```
SET user Milan
GET user
EXPIRE user 10
RATE user1
STATS
```

## Логирование

Логирование реализовано через `src/utils/Logger.h`.

Используется:

- вывод в консоль

Формат:

```
[YYYY-MM-DD HH:MM:SS] message
```

## Тестирование

Автоматические тесты пока не добавлены.

## Обработка ошибок

Обработка ошибок реализована на двух уровнях:

App уровень

- основной цикл защищён проверкой ввода и командой `exit`
- безопасные ответы на неизвестные команды и некорректный TTL

Storage/Cache/Limiter уровень

- потокобезопасные операции с mutex
- истёкшие ключи автоматически удаляются
- rate limiter возвращает корректный allow/block без исключений

## Структура проекта

```
backend-platform/
L-- src/
    +-- main.cpp
    +-- core/
    ¦   +-- Engine.h
    ¦   L-- CommandParser.h
    +-- database/
    ¦   +-- Database.h
    ¦   L-- TTLManager.h
    +-- cache/
    ¦   L-- LRUCache.h
    +-- limiter/
    ¦   L-- RateLimiter.h
    +-- models/
    ¦   L-- Entry.h
    L-- utils/
        L-- Logger.h
```

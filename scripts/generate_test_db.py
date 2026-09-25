#!/usr/bin/env python3

# AI CREATED SCRIPT

import argparse
import ipaddress
import random
import sqlite3
import string
import sys
from pathlib import Path

FIRST_NAMES = [
    "Александр", "Максим", "Иван", "Дмитрий", "Артём", "Михаил",
    "Егор", "Никита", "Алексей", "Сергей", "Анна", "Мария",
    "Екатерина", "Ольга", "Виктория", "Анастасия", "Дарья", "Полина"
]
LAST_NAMES = [
    "Иванов", "Петров", "Сидоров", "Смирнов", "Кузнецов",
    "Попов", "Васильев", "Соколов", "Морозов", "Новиков",
    "Фёдоров", "Волков", "Алексеев", "Лебедев", "Семёнов"
]
PATRONYMICS = [
    "Александрович", "Максимович", "Иванович", "Дмитриевич",
    "Артёмович", "Михайлович", "Егорович", "Никитич",
    "Алексеевич", "Сергеевич", "Александровна", "Максимовна",
    "Ивановна", "Дмитриевна", "Артёмовна", "Михайловна"
]
CITIES = [
    "Москва", "Санкт-Петербург", "Казань", "Самара", "Саратов",
    "Екатеринбург", "Новосибирск", "Воронеж", "Тула", "Омск"
]
DEVICES = ["Windows PC", "Linux PC", "Android", "iPhone", "MacBook", "Tablet"]
EMAIL_DOMAINS = ["example.com", "test.local", "mail.test", "example.net"]


def random_card():
    # Зарезервированный тестовый диапазон: не использовать для платежей.
    return "9999" + "".join(random.choices(string.digits, k=12))


def random_ip():
    return str(ipaddress.IPv4Address(random.randint(0x0A000001, 0x0AFFFFFF)))


def make_row():
    first = random.choice(FIRST_NAMES)
    last = random.choice(LAST_NAMES)
    patronymic = random.choice(PATRONYMICS)
    city = random.choice(CITIES)
    address = f"{city}, ул. {random.choice(['Лесная', 'Центральная', 'Садовая', 'Школьная', 'Молодёжная'])}, д. {random.randint(1, 250)}, кв. {random.randint(1, 300)}"
    email = f"user{random.randint(1, 10**12)}@{random.choice(EMAIL_DOMAINS)}"

    return (
        last,
        first,
        patronymic,
        random.randint(18, 85),
        address,
        random_card(),
        random.randint(0, 250),
        random.choice(DEVICES),
        random_ip(),
        email,
        random.choice(["active", "inactive", "blocked"]),
        random.randint(0, 500000),
    )


def estimate_size(count):
    # Эвристическая оценка: средний размер строки + индексы + служебные страницы SQLite.
    avg_row_bytes = 330
    indexes_factor = 1.9
    overhead = 1.15
    return int(count * avg_row_bytes * indexes_factor * overhead)


def format_size(size):
    units = ["Б", "КБ", "МБ", "ГБ", "ТБ"]
    value = float(size)
    for unit in units:
        if value < 1024 or unit == units[-1]:
            return f"{value:.1f} {unit}"
        value /= 1024


def ask_confirmation(estimated):
    print(f"Предварительная оценка размера базы: {format_size(estimated)}")
    if estimated > 100 * 1024 * 1024:
        answer = input("Размер превышает 100 МБ. Продолжить создание? [y/N]: ")
        return answer.strip().lower() in {"y", "yes", "д", "да"}
    return True


def create_database(path, count, batch_size=5000):
    if path.exists():
        answer = input(f"Файл {path} уже существует. Перезаписать? [y/N]: ")
        if answer.strip().lower() not in {"y", "yes", "д", "да"}:
            print("Отменено.")
            return

    if path.exists():
        path.unlink()

    conn = sqlite3.connect(path)
    try:
        conn.execute("PRAGMA journal_mode = OFF")
        conn.execute("PRAGMA synchronous = OFF")
        conn.execute("PRAGMA temp_store = MEMORY")
        conn.execute("PRAGMA cache_size = -65536")

        conn.execute("""
            CREATE TABLE users (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                last_name TEXT NOT NULL,
                first_name TEXT NOT NULL,
                patronymic TEXT NOT NULL,
                age INTEGER NOT NULL,
                address TEXT NOT NULL,
                card_number TEXT NOT NULL,
                order_count INTEGER NOT NULL,
                device TEXT NOT NULL,
                last_ip TEXT NOT NULL,
                email TEXT NOT NULL,
                status TEXT NOT NULL,
                total_spent INTEGER NOT NULL
            )
        """)

        sql = """
            INSERT INTO users (
                last_name, first_name, patronymic, age, address,
                card_number, order_count, device, last_ip, email,
                status, total_spent
            ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
        """

        for start in range(0, count, batch_size):
            current = min(batch_size, count - start)
            rows = [make_row() for _ in range(current)]
            conn.executemany(sql, rows)
            if (start + current) % max(batch_size * 10, 1) == 0 or start + current == count:
                print(f"\rСоздано: {start + current:,}/{count:,}", end="", flush=True)

        print("\nСоздание индексов...")
        conn.execute("""
            CREATE INDEX idx_users_fio
            ON users(last_name, first_name, patronymic)
        """)
        conn.execute("CREATE INDEX idx_users_address ON users(address)")
        conn.execute("CREATE INDEX idx_users_card_number ON users(card_number)")
        conn.execute("CREATE INDEX idx_users_last_ip ON users(last_ip)")
        conn.execute("CREATE INDEX idx_users_email ON users(email)")

        conn.commit()
        conn.execute("VACUUM")
        print(f"Готово: {path} ({format_size(path.stat().st_size)})")
    finally:
        conn.close()


def main():
    parser = argparse.ArgumentParser(
        description="Генератор тестовой SQLite-базы с синтетическими данными."
    )
    parser.add_argument(
        "-c", "--count", type=int, default=1_000_000,
        help="Количество записей (по умолчанию: 1000000)"
    )
    parser.add_argument(
        "-o", "--output", type=Path, default=Path("test.db"),
        help="Путь к SQLite-файлу (по умолчанию: test.db)"
    )
    args = parser.parse_args()

    if args.count < 1:
        parser.error("Количество записей должно быть положительным.")

    estimated = estimate_size(args.count)
    print(f"Количество записей: {args.count:,}")
    print(f"Оценочный размер: {format_size(estimated)}")

    if not ask_confirmation(estimated):
        print("Отменено пользователем.")
        sys.exit(0)

    create_database(args.output, args.count)


if __name__ == "__main__":
    main()

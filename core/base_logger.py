# core/base_logger.py
# A simple conceptual python logger to show python library dependencies.

import datetime

def log_info(message: str) -> None:
    """Logs an informational message to the console."""
    print(f"[INFO] {datetime.datetime.now()} - {message}")

def log_error(message: str) -> None:
    """Logs an error message to the console."""
    print(f"[ERROR] {datetime.datetime.now()} - {message}")

import string
# This import relies on the '//core:base_logger' target dependency.
from core.base_logger import log_info, log_error

def format_greeting(name: str) -> str:
    """Formats a greeting strictly and logs the operation."""
    if not name:
        log_error("Attempted to format greeting for empty name!")
        return "Hello, Friend!"
    
    log_info(f"Formatting greeting for: {name}")
    return f"Hello, {string.capwords(name)}!"

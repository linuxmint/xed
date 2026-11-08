# -*- coding: utf-8 -*-
#
#  formatter.py - JSON formatting helpers for the JSON Formatter plugin
#
#  Copyright (C) 2026 Elnaril <elnaril@gmx.com>
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor,
#  Boston, MA 02110-1301, USA.

import json
from typing import Optional, Tuple


def _error_message(error: json.decoder.JSONDecodeError) -> str:
    return "{} (line {}, column {})".format(error.msg, error.lineno, error.colno)


def validate_json(text: str) -> Tuple[bool, Optional[str]]:
    """Return (True, None) if text is valid JSON, else (False, message)."""
    try:
        json.loads(text)
    except json.decoder.JSONDecodeError as error:
        return False, _error_message(error)
    return True, None


def format_json(text: str, indent: int = 4) -> str:
    """Pretty-print JSON. Raises ValueError on invalid input."""
    try:
        data = json.loads(text)
    except json.decoder.JSONDecodeError as error:
        raise ValueError(_error_message(error)) from error
    return json.dumps(data, indent=indent, ensure_ascii=False)


def minify_json(text: str) -> str:
    """Minify JSON. Raises ValueError on invalid input."""
    try:
        data = json.loads(text)
    except json.decoder.JSONDecodeError as error:
        raise ValueError(_error_message(error)) from error
    return json.dumps(data, separators=(",", ":"), ensure_ascii=False)

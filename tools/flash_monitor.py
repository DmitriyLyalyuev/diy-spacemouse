#!/usr/bin/env python3
import argparse
import glob
import os
import select
import subprocess
import sys
import termios
import time
from datetime import datetime
from pathlib import Path

DEFAULT_SKETCH = "Code/firmware"
DEFAULT_FQBN = "rp2040:rp2040:adafruit_qtpy:usbstack=picosdk"
DEFAULT_BAUD = 115200
DEFAULT_SECONDS = 20
SERIAL_GLOB = "/dev/cu.usbmodem*"
BOOTSEL_VOLUME = "/Volumes/RPI-RP2"
UF2_PORT = "UF2_Board"
SERIAL_WAIT_SECONDS = 20

BAUD_RATES = {
    9600: termios.B9600,
    19200: termios.B19200,
    38400: termios.B38400,
    57600: termios.B57600,
    115200: termios.B115200,
    230400: getattr(termios, "B230400", None),
    460800: getattr(termios, "B460800", None),
    921600: getattr(termios, "B921600", None),
}


def parse_args():
    parser = argparse.ArgumentParser(
        description="Upload an Arduino sketch to QT Py RP2040 and capture serial output."
    )
    parser.add_argument("sketch", nargs="?", default=DEFAULT_SKETCH)
    parser.add_argument("--fqbn", default=DEFAULT_FQBN)
    parser.add_argument("--port")
    parser.add_argument("--seconds", type=float, default=DEFAULT_SECONDS)
    parser.add_argument("--baud", type=int, default=DEFAULT_BAUD)
    parser.add_argument("--no-upload", action="store_true", help="Skip upload and monitor only.")
    parser.add_argument("--log", help="Serial log path. Defaults to logs/serial-YYYYmmdd-HHMMSS.log")
    return parser.parse_args()


def serial_ports():
    return sorted(glob.glob(SERIAL_GLOB))


def default_log_path():
    stamp = datetime.now().strftime("%Y%m%d-%H%M%S")
    return Path("logs") / f"serial-{stamp}.log"


def choose_upload_port(explicit_port):
    if explicit_port:
        return explicit_port

    ports = serial_ports()
    if ports:
        return ports[0]

    if os.path.exists(BOOTSEL_VOLUME):
        return UF2_PORT

    print(
        f"No {SERIAL_GLOB} device and {BOOTSEL_VOLUME} is not mounted. "
        "Hold BOOTSEL while connecting the QT Py RP2040, or pass --port.",
        file=sys.stderr,
    )
    sys.exit(2)


def run_board_list():
    print("$ arduino-cli board list")
    result = subprocess.run(["arduino-cli", "board", "list"], text=True)
    print()
    return result.returncode


def upload(sketch, fqbn, port):
    command = ["arduino-cli", "upload", "-p", port, "--fqbn", fqbn, sketch]
    print("$ " + " ".join(command))
    return subprocess.run(command).returncode


def baud_constant(baud):
    constant = BAUD_RATES.get(baud)
    if constant is None:
        raise ValueError(f"Unsupported baud rate for termios on this system: {baud}")
    return constant


def configure_serial(fd, baud):
    attrs = termios.tcgetattr(fd)
    speed = baud_constant(baud)

    attrs[0] = 0
    attrs[1] = 0
    attrs[2] = termios.CS8 | termios.CREAD | termios.CLOCAL
    attrs[3] = 0
    attrs[4] = speed
    attrs[5] = speed
    attrs[6][termios.VMIN] = 0
    attrs[6][termios.VTIME] = 0

    termios.tcsetattr(fd, termios.TCSANOW, attrs)


def open_serial(path, baud):
    fd = os.open(path, os.O_RDWR | os.O_NOCTTY | os.O_NONBLOCK)
    try:
        configure_serial(fd, baud)
    except Exception:
        os.close(fd)
        raise
    return fd


def wait_for_serial(timeout):
    deadline = time.monotonic() + timeout
    while time.monotonic() <= deadline:
        ports = serial_ports()
        if ports:
            return ports[0]
        time.sleep(0.25)
    return None


def reopen_serial_until_deadline(path, baud, deadline):
    while time.monotonic() <= deadline:
        try:
            return open_serial(path, baud)
        except OSError as exc:
            if "Device not configured" not in str(exc):
                raise
            time.sleep(0.25)
    raise TimeoutError(f"Serial device did not recover before timeout: {path}")


def monitor_serial(path, baud, seconds, log_path):
    log_path.parent.mkdir(parents=True, exist_ok=True)
    end_time = time.monotonic() + seconds
    fd = reopen_serial_until_deadline(path, baud, end_time)

    print(f"Monitoring {path} at {baud} baud for {seconds:g}s")
    print(f"Logging to {log_path}")

    try:
        with log_path.open("w", encoding="utf-8", errors="replace") as log_file:
            while time.monotonic() < end_time:
                timeout = min(0.25, max(0.0, end_time - time.monotonic()))
                try:
                    readable, _, _ = select.select([fd], [], [], timeout)
                    if not readable:
                        continue
                    chunk = os.read(fd, 4096)
                except OSError as exc:
                    if "Device not configured" not in str(exc):
                        raise
                    os.close(fd)
                    fd = reopen_serial_until_deadline(path, baud, end_time)
                    continue

                if not chunk:
                    continue

                text = chunk.decode("utf-8", errors="replace")
                sys.stdout.write(text)
                sys.stdout.flush()
                log_file.write(text)
                log_file.flush()
    finally:
        os.close(fd)


def main():
    args = parse_args()
    log_path = Path(args.log) if args.log else default_log_path()

    if not args.no_upload:
        port = choose_upload_port(args.port)
        run_board_list()
        upload_code = upload(args.sketch, args.fqbn, port)
        if upload_code != 0:
            return upload_code

    serial_path = wait_for_serial(SERIAL_WAIT_SECONDS)
    if not serial_path:
        print(
            f"No {SERIAL_GLOB} serial device appeared within {SERIAL_WAIT_SECONDS}s. "
            "Press reset or reconnect the QT Py RP2040.",
            file=sys.stderr,
        )
        return 2

    try:
        monitor_serial(serial_path, args.baud, args.seconds, log_path)
    except ValueError as exc:
        print(str(exc), file=sys.stderr)
        return 2
    except TimeoutError as exc:
        print(str(exc), file=sys.stderr)
        return 2

    return 0


if __name__ == "__main__":
    sys.exit(main())

"""Generate SDI-12 Sensor program overview PDF."""
from fpdf import FPDF
from pathlib import Path

OUT = Path(__file__).resolve().parent / "SDI12_Sensor_Program_Overview.pdf"


class OverviewPDF(FPDF):
    def header(self):
        self.set_font("Helvetica", "B", 11)
        self.cell(0, 8, "SDI-12 Environmental Sensor - Program Overview", align="C", new_x="LMARGIN", new_y="NEXT")
        self.ln(2)

    def footer(self):
        self.set_y(-12)
        self.set_font("Helvetica", "I", 8)
        self.cell(0, 8, f"Page {self.page_no()}/{{nb}}", align="C")

    def section_title(self, title: str):
        self.ln(4)
        self.set_font("Helvetica", "B", 13)
        self.set_fill_color(230, 230, 230)
        self.cell(0, 9, title, new_x="LMARGIN", new_y="NEXT", fill=True)
        self.ln(2)

    def sub_title(self, title: str):
        self.set_font("Helvetica", "B", 11)
        self.cell(0, 7, title, new_x="LMARGIN", new_y="NEXT")
        self.ln(1)

    def body(self, text: str):
        self.set_x(self.l_margin)
        self.set_font("Helvetica", "", 10)
        self.multi_cell(0, 5, text)
        self.ln(1)

    def bullet(self, text: str):
        self.set_x(self.l_margin)
        self.set_font("Helvetica", "", 10)
        self.multi_cell(0, 5, "- " + text)


def build_pdf():
    pdf = OverviewPDF()
    pdf.alias_nb_pages()
    pdf.set_auto_page_break(auto=True, margin=15)
    pdf.add_page()

    pdf.set_font("Helvetica", "", 10)
    pdf.multi_cell(
        0,
        5,
        "This document describes the firmware in the main/ folder: an Arduino-based "
        "environment sensor that reads BME280 and BH1750, acts as an SDI-12 slave, "
        "logs data to SD card with RTC timestamps, and shows a real-time TFT dashboard. "
        "Generated for the REAL ONE project.",
    )
    pdf.ln(3)

    # --- 1. What it does ---
    pdf.section_title("1. What the Program Does")
    pdf.bullet("Read sensors: temperature, humidity, pressure (BME280), light level (BH1750) over I2C.")
    pdf.bullet("SDI-12 slave: respond to a data logger on Serial1 at address 0 (configurable).")
    pdf.bullet("Local logging: append CSV rows to datalog.csv on SD card every 60 seconds.")
    pdf.bullet("Manual control: pushbuttons for manual log entry and clearing the log file.")
    pdf.bullet("TFT dashboard: ST7735 display showing live sensor values.")

    # --- 2. Where to start ---
    pdf.section_title("2. Recommended Reading Order")
    rows = [
        ("1", "main.ino", "Entry point: setup() init order and loop() schedule."),
        ("2", "HardwareConfig.h", "Pin numbers and timing constants."),
        ("3", "SensorReading.h / .cpp", "SensorData struct, readSensors(), getSensorData()."),
        ("4", "sdi12.h / .cpp", "SDI-12 protocol and command parser."),
        ("5", "DataLogger.h / .cpp", "RTC, SD card, buttons, CSV logging."),
        ("6", "Dashboard.h / .cpp", "ST7735 TFT user interface."),
    ]
    pdf.set_font("Helvetica", "B", 9)
    pdf.cell(12, 7, "#", border=1)
    pdf.cell(42, 7, "File", border=1)
    pdf.cell(0, 7, "Purpose", border=1, new_x="LMARGIN", new_y="NEXT")
    pdf.set_font("Helvetica", "", 9)
    for num, fname, purpose in rows:
        pdf.cell(12, 6, num, border=1)
        pdf.cell(42, 6, fname, border=1)
        pdf.cell(0, 6, purpose, border=1, new_x="LMARGIN", new_y="NEXT")

    # --- 3. File tree ---
    pdf.section_title("3. Project File Structure (main/)")
    pdf.set_font("Courier", "", 9)
    tree = """main.ino              Orchestrator (setup + loop)
HardwareConfig.h      Pins and intervals
SensorReading.h/cpp   BME280 + BH1750 driver layer
sdi12.h/cpp           SDI-12 slave on Serial1
DataLogger.h/cpp      RTC, SD card, buttons, CSV
Dashboard.h/cpp       ST7735 TFT display"""
    pdf.multi_cell(0, 4, tree)
    pdf.ln(2)

    # --- 4. main.ino ---
    pdf.section_title("4. main.ino - Program Flow")
    pdf.sub_title("setup() - runs once at boot")
    pdf.bullet("Serial.begin(9600) - USB debug port.")
    pdf.bullet("sensorsInit() - I2C, BME280, BH1750.")
    pdf.bullet("dataLoggerInit() - RTC, SD card, buttons.")
    pdf.bullet("dashboardInit() - TFT display.")
    pdf.bullet("sdi12Init(MY_ADDRESS, DIRO_PIN) - Serial1 @ 1200 7E1, direction pin.")
    pdf.bullet("readSensors() - first sample so buffer is not empty.")

    pdf.sub_title("loop() - runs continuously")
    pdf.bullet("Every 500 ms (kSensorPollMs): readSensors() updates shared buffer.")
    pdf.bullet("sdi12Handle() - process incoming SDI-12 commands (non-blocking).")
    pdf.bullet("dataLoggerUpdate() - 60 s auto-log, button checks.")
    pdf.bullet("dashboardUpdate() - refresh TFT about every 1 second.")

    pdf.sub_title("Timing variables in main.ino")
    pdf.body(
        "kSensorPollMs (500): minimum interval between background sensor reads.\n"
        "lastSensorPollMs: millis() timestamp of the last readSensors() call.\n"
        "These use a non-blocking timer so loop() never calls delay() for polling."
    )

    # --- 5. Data flow ---
    pdf.add_page()
    pdf.section_title("5. Data Flow")
    pdf.set_font("Courier", "", 9)
    flow = """BME280 + BH1750  --I2C-->  readSensors()  -->  sensorBuffer (static)
                                              |
                    +-------------------------+-------------------------+
                    |                         |                         |
              getSensorData()           getSensorData()           getSensorData()
                    |                         |                         |
            dashboardUpdate()         logDataInternal()         SDI-12 parseCommand
                    |                         |                         |
               TFT screen               datalog.csv                  Serial1 host"""
    pdf.multi_cell(0, 4, flow)
    pdf.ln(2)
    pdf.body(
        "SensorReading.cpp owns the buffer. All other modules call readSensors() when they "
        "need fresh data, or rely on the 500 ms background poll in main.ino."
    )

    # --- 6. Modules ---
    pdf.section_title("6. Module Reference")

    pdf.sub_title("SensorReading")
    pdf.bullet("SensorData: temperature, humidity, pressure, lux, ready flag.")
    pdf.bullet("sensorsInit(), readSensors(), getSensorData().")
    pdf.bullet("isBme280Ok(), isBh1750Ok(), getParameterCount() for SDI-12.")

    pdf.sub_title("sdi12")
    pdf.bullet("Serial1 @ 1200 baud, 7 data bits, even parity, 1 stop bit.")
    pdf.bullet("DIRO_PIN: TX/RX direction for line driver (pin 8 in main.ino; pin 7 is TFT_DC).")
    pdf.bullet("Commands assembled until '!' then parsed in parseCommand().")
    pdf.bullet("Debug [RX]/[TX] printed on USB Serial, not on SDI-12 bus.")

    pdf.sub_title("DataLogger")
    pdf.bullet("RTC DS1307 for timestamps.")
    pdf.bullet("SD card via soft SPI (SdFat); file: datalog.csv.")
    pdf.bullet("CSV columns: Timestamp, Temperature_C, Humidity_pct, Pressure_hPa, Lux, Source.")
    pdf.bullet("Source: Periodic (60 s) or Manual (button pin 2).")
    pdf.bullet("Button pin 3: delete log file and recreate CSV header.")

    pdf.sub_title("Dashboard")
    pdf.bullet("Adafruit ST7735 TFT; INITR_BLACKTAB, rotation 1.")
    pdf.bullet("Static labels drawn once in dashboardInit(); values updated in dashboardUpdate().")
    pdf.bullet("Refresh interval: kDashboardRefreshMs (1000 ms) in HardwareConfig.h.")

    # --- 7. Pins ---
    pdf.section_title("7. Hardware Pins (HardwareConfig.h + main.ino)")
    pins = [
        ("SD CS", "A3", "SD card chip select (soft SPI)"),
        ("SD MISO / MOSI / SCK", "12 / 11 / 13", "Soft SPI for SD card"),
        ("Btn manual log", "2", "Active LOW, INPUT_PULLUP"),
        ("Btn clear SD", "3", "Active LOW, INPUT_PULLUP"),
        ("TFT CS / DC / RST", "10 / 7 / 6", "ST7735 control pins"),
        ("TFT MOSI / SCLK", "11 / 13", "Hardware SPI (shared with SD soft SPI)"),
        ("DIRO_PIN", "8", "SDI-12 direction (main.ino; not pin 7)"),
        ("I2C", "SDA/SCL", "BME280 @ 0x76, BH1750, RTC DS1307"),
        ("Serial1", "TX/RX", "SDI-12 data line to host"),
    ]
    pdf.set_font("Helvetica", "B", 9)
    pdf.cell(45, 7, "Signal", border=1)
    pdf.cell(28, 7, "Pin(s)", border=1)
    pdf.cell(0, 7, "Notes", border=1, new_x="LMARGIN", new_y="NEXT")
    pdf.set_font("Helvetica", "", 9)
    for sig, pin, note in pins:
        pdf.cell(45, 6, sig, border=1)
        pdf.cell(28, 6, pin, border=1)
        pdf.cell(0, 6, note, border=1, new_x="LMARGIN", new_y="NEXT")

    # --- 8. SDI-12 ---
    pdf.section_title("8. SDI-12 Commands (address 0)")
    cmds = [
        ("?", "Address query - returns sensor address"),
        ("0M", "Measure - returns 0003 + parameter count, reads sensors"),
        ("0D1", "BME280 data: temp, humidity, pressure"),
        ("0D2", "BH1750 data: lux"),
        ("0R0", "All data: temp, humidity, pressure, lux"),
        ("0An", "Change address to character n"),
    ]
    pdf.set_font("Helvetica", "B", 9)
    pdf.cell(25, 7, "Command", border=1)
    pdf.cell(0, 7, "Description", border=1, new_x="LMARGIN", new_y="NEXT")
    pdf.set_font("Helvetica", "", 9)
    for cmd, desc in cmds:
        pdf.cell(25, 6, cmd, border=1)
        pdf.cell(0, 6, desc, border=1, new_x="LMARGIN", new_y="NEXT")

    # --- 9. Libraries ---
    pdf.section_title("9. Required Arduino Libraries")
    libs = [
        "Adafruit BME280 Library + Adafruit Unified Sensor",
        "BH1750",
        "Adafruit GFX Library + Adafruit ST7735 and ST7789 Library",
        "RTClib (Adafruit)",
        "SdFat (Bill Greiman v2+)",
        "Bounce2",
    ]
    for lib in libs:
        pdf.bullet(lib)

    # --- 10. Debug ---
    pdf.section_title("10. Debugging Tips")
    pdf.bullet("Open USB Serial Monitor at 9600 baud for [RX], [TX], [Log], [Init] messages.")
    pdf.bullet("If TFT is blank: check wiring, INITR_BLACKTAB vs GREENTAB, I2C sensor init.")
    pdf.bullet("If SDI-12 fails: verify DIRO_PIN wiring, Serial1 pins, MY_ADDRESS matches host.")
    pdf.bullet("If RTC time is wrong: replace battery or run rtc.adjust() once in DataLogger.cpp.")
    pdf.bullet("If SD fails: check CS on A3 and soft SPI pins 11/12/13.")

    pdf.ln(4)
    pdf.set_font("Helvetica", "I", 9)
    pdf.cell(0, 5, "Project path: REAL ONE/main/  |  Document auto-generated from firmware structure.")

    pdf.output(OUT)
    return OUT


if __name__ == "__main__":
    path = build_pdf()
    print(f"Wrote: {path}")

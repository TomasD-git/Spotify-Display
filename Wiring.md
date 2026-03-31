## Wiring Diagram

### TFT Display (ST7735S, 1.8" SPI)
| Display Pin | Connects To | Notes |
|------------|-------------|-------|
| VCC        | 3.3V        | Power |
| GND        | GND         | Ground |
| SCL        | GPIO4       | SPI Clock  |
| SDA        | GPIO6       | SPI Data  |
| BL         | 3.3V        | Backlight |

### Key Switches (7x)
All switches share a common ground:

- **One side of each switch → ESP32 GND**

Individual GPIO connections:

| Switch | ESP32 Pin |
|--------|-----------|
| SW1    | GPIO0     |
| SW2    | GPIO1     |
| SW3    | GPIO2     |
| SW4    | GPIO3     |
| SW5    | GPIO5     |
| SW6    | GPIO9     |
| SW7    | GPIO20    |

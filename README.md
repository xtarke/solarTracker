# solarTracker

This application tracks the sun using NREL's Solar Position Algorithm (SPA) (not include in the repository). User interface uses MQTT publish/subscribe.

### Prerequisites

- A BeagleBone black running Debian 9.3.
- PRU development files
- Mosquitto c++ devel files
- Mosquitto broker running in BeagleBone
- A serial GPS with NMEA string format
- A solar tracker with two stepper motors: Azimuth and Zenith

### MQTT USer interface commands:

- Commands (to publish):
    - "solar/cmd": changes application mode
        - "0" -> Solar tracker activated
        - "1" -> Manual
        - "2" -> Turn off system. Azimuth and Zenith axis go to "home position"
        - "3" -> Calibration mode.


    - "solar/cmd/azrepos": reposition of Azimuth axis in pulses. Hardware max/min angles are proteced
        - Value lower than zero rotates clockwise
        - Value higher than zero rotates counterclowise
        - Only works in Manual mode

    - "solar/cmd/zerepos": reposition of Zenith axis in pulses. Hardware max/min angles are proteced
        - Value lower than zero rotates clockwise
        - Value higher than zero rotates counterclowise
        - Only works in Manual mode

    - "solar/cmd/zesethome": Move Zenith axis (pulses). NO HARDWARE PROTECTION.
        - Value lower than zero rotates clockwise
        - Value higher than zero rotates counterclowise
        - Only works in Manual mode

    - "solar/cmd/azsethome": Move Azimuth axis (pulses). NO HARDWARE PROTECTION.
        - Value lower than zero rotates clockwise
        - Value higher than zero rotates counterclowise
        - Only works in Manual mode

- Informations: (to subscribe):
    - "solar/gps/status": current GPS status (updated updated every 1 second)
    - "solar/az": current SPA Azimuth angle (updated updated every 1 second)
    - "solar/ze": current SPA Zenith angle (updated updated every 1 second)
    - "solar/gps/longitude": current GPS longiute (degress) (updated updated every 1 second)
    - "solar/gps/latitude": current GPS latitute (degress) (updated updated every 1 second)
    - "solar/gps/elevation": current GPS elevation (m) (updated updated every 1 second)
    - "solar/debug": CSV format az,ze,gps status, current time ((updated updated every 5 minutes)
    - "solar/norm": Azimuth Zenith AzPulses ZePulses (updated updated every 1 second)
    - "solar/sunrise": SPA sunrise time
    - "solar/sunset": SPA sunset time

### Hardware notes

- BeagleBone pins: (ALL 3V3. Use correct voltage drivers)

    - I2C Interface for Magnetometer, termometer and barometer
        - P9.19 - I2C_SCL
        - P9.20 - I2C_SDA
    - Serial (UART) for NMEA GPS
        - P9.24 (TX) -> GPS (RX)
        - P9.26 (RX) -> GPS (TX)

    - Stepper interface: Internal Sitara PRU
        - P9.27 (PRU) -> Pulses to azimuth driver
        - P9.28 (PRU) -> Pulses to zenith driver
        - P9.29 (PRU) -> Azmith direction
        - P9.30 (PRU) -> Zenith direction

### Notes about last version

- Updated:
    - Using Debian 9.3 now instead of ArchLinux
    - PRU uses uio_pruss and bone kernel
        - Linux beaglebone 4.9.76-bone9
    - Capes using /sys/devices/platform/bone_capemgr/slots are no longer suported
    - DTS are loaded in /boot/uEnv.txt
        - additional custom capes
            - uboot_overlay_addr4=/lib/firmware/BB-UART1-00A0.dtbo
            - uboot_overlay_addr5=/lib/firmware/EBB-PRU-xtarke-00A0.dtbo

### Links:
    - https://github.com/beagleboard/bb.org-overlays/
    - https://github.com/cdsteinkuehler/beaglebone-universal-io
    - https://elinux.org/Beagleboard:BeagleBoneBlack_Debian#U-Boot_Overlays
    - https://github.com/RobertCNelson/dtb-rebuilder

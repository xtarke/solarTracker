# solarTracker

Updated:
    - Using Debian 9.3 now instead of ArchLinux
    - PRU uses uio_pruss and bone kernel
        - Linux beaglebone 4.9.76-bone9
    - Capes using /sys/devices/platform/bone_capemgr/slots are no longer suported
    - DTS are loaded in /boot/uEnv.txt
        ###Additional custom capes
        uboot_overlay_addr4=/lib/firmware/BB-UART1-00A0.dtbo
        uboot_overlay_addr5=/lib/firmware/EBB-PRU-xtarke-00A0.dtbo
    

Links:
    https://github.com/beagleboard/bb.org-overlays/
    https://github.com/cdsteinkuehler/beaglebone-universal-io
    https://elinux.org/Beagleboard:BeagleBoneBlack_Debian#U-Boot_Overlays
    https://github.com/RobertCNelson/dtb-rebuilder

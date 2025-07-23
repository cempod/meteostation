openocd -f interface/stlink.cfg -f target/stm32h7x.cfg -c "program nuttx/nuttx.hex verify reset exit"


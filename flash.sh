openocd -f interface/stlink.cfg -f target/stm32h7x.cfg -c "program build/nuttx.hex verify reset exit"


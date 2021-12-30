# STM32_AHT10_VirtualCOM
Read AHT10 Temperature and Humidity data, then send them back to serial monitor by virtual COM port.

These files are proved by Nucleo-F411RE board:
* AHT10.c: The state machine for initialization AHT10 and read data back per second.
* main.c: check all of codes in "USER CODE BEGIN/END" for add AHT10 state machine process.
* I2C_AHT10.ioc: The setting of Nucleo-F411RE board; It set I2C1 and UART2 port for this project
* AHT10.graphml: state machine diagram in yEd. It should be opened by yEd graph editor.
                 yEd web link: https://www.yworks.com/products/yed
    

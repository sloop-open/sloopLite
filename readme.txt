sloop 精简版

这是一个轻量级 嵌入式裸机 单线程 协作式任务调度 管理框架。

1.保留核心的任务调度系统
2.保留RTT日志输出功能
3.移除驱动包
4.移除命令控制台
5.基于 STM32G0 
6.基于 STM32Cube

demo资源消耗

==============================================================================

Image component sizes


      Code (inc. data)   RO Data    RW Data    ZI Data      Debug   Object Name

        24          4          0          0          0        626   _main.o
        32          4          0          0          0       1193   gpio.o
       168          0          0          0          0       1624   main.o
       998         32         42          0       1113      13980   segger_rtt.o
      1946         64         23          0          0       4009   segger_rtt_printf.o
      5356        292       1845          0       1574       8666   sloop.o
        52         22        192          0       1024        604   startup_stm32g030xx.o
       208          8          0          5          4       6421   stm32g0xx_hal.o
       256         20          0          0          0       6296   stm32g0xx_hal_cortex.o
        60          8          0          0          0       1258   stm32g0xx_hal_msp.o
       124         12          0          0          0       3939   stm32g0xx_hal_pwr_ex.o
      2580        116          0          0          0       8872   stm32g0xx_hal_rcc.o
      1200         44        914          0          0       1690   stm32g0xx_it.o
         2          0         64          4          0       1850   system_stm32g0xx.o
       380          8        163          0          0        585   task_baseinit.o
       600         36         80          0          0        823   task_demo.o
       380         20         10          0          0        571   task_idle.o

==============================================================================

    Total RO  Size (Code + RO Data)                18648 (  18.21kB)
    Total RW  Size (RW Data + ZI Data)              3840 (   3.75kB)
    Total ROM Size (Code + RO Data + RW Data)      18660 (  18.22kB)

==============================================================================

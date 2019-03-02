       .global bootloader_data
bootloader_data:
        .incbin "@file@"

bootloader_data_end:    

        .global bootloader_data_size
bootloader_data_size:
        .int bootloader_data_end - bootloader_data

int main(void) {  
    __asm__ volatile (  
        //   gpio_init(LED_PIN);
        ///  gpio_set_dir(LED_PIN, GPIO_IN);
        //// gpioc_bit_oe_put(LED_PIN, GPIO_OUT);
        "movs r4, #0x10\n"                  // GPIO16
        "movs r5, #0x01\n"                  // bit 1; used for OUT/OE writes
        "mcrr p0, #4, r4, r5, c4\n"         // gpioc_bit_oe_put(16, 1); p102

        //   gpio_set_function(LED_PIN, GPIO_FUNC_SIO);
        ///  hw_write_masked(&pads_bank0_hw->io[LED_PIN],
        ///                  PADS_BANK0_GPIO0_IE_BITS,
        ///                  PADS_BANK0_GPIO0_IE_BITS | PADS_BANK0_GPIO0_OD_BITS
        ///  );
        //// hw_xor_bits(addr, (*addr ^ values) & write_mask);
        "ldr r3, =0x40038044\n"             // &pads_bank0_hw->io[16]; p785, p796
        "ldr r2, [r3]\n"                    // load current config  
        "bic r2, r2, #0x80\n"               // clear OD; output disable
        "orr r2, r2, #0x40\n"               // set IE; enable input buffer
        "str r2, [r3]\n"                    // store updated config  
        ///  io_bank0_hw->io[LED_PIN].ctrl = GPIO_FUNC_SIO << IO_BANK0_GPIO0_CTRL_FUNCSEL_LSB;
        "ldr r3, =0x40028084\n"             // &io_bank0_hw->io[16].ctrl; p603, p637
        "ldr r2, [r3]\n"                    // load current config 
        "bic r2, r2, #0x1f\n"               // clear FUNCSEL bits [4:0]
        "orr r2, r2, #5\n"                  // set FUNCSEL = 5 (SIO)
        "str r2, [r3]\n"                    // store updated config
        ///  hw_clear_bits(&pads_bank0_hw->io[gpio], PADS_BANK0_GPIO0_ISO_BITS);
        "ldr r3, =0x40038044\n"             // &pads_bank0_hw->io[16]; p785, p796
        "ldr r2, [r3]\n"                    // load current config  
        "bic r2, r2, #0x100\n"              // clear ISO bit (bit 8) un‑isolate pad  
        "str r2, [r3]\n"                    // store updated config  

        //   gpio_set_dir(LED_PIN, GPIO_OUT);
        ///  gpioc_bit_oe_put(LED_PIN, GPIO_OUT);
        "movs r4, #0x10\n"                  // GPIO16
        "movs r5, #0x01\n"                  // bit 1; used for OUT/OE writes
        "mcrr p0, #4, r4, r5, c4\n"         // gpioc_bit_oe_put(16, 1); p102   

        //   while (true)
        "1:\n"                              // loop start  

        //   gpio_put(LED_PIN, 1);
        ///  gpioc_bit_out_put(LED_PIN, 1);
        "movs r4, #0x10\n"                  // GPIO16
        "movs r5, #0x01\n"                  // bit 1; used for OUT/OE writes
        "mcrr p0, #4, r4, r5, c0\n"         // gpioc_bit_out_put(16, 1) 
        //   sleep_ms(500);
        ///  sleep_us(500 * 1000ull);
        "ldr r2, =0x17D7840\n"              // r2 = ~8.4M cycles  
        "2:\n"                              // delay loop  
        "subs r2, r2, #1\n"                 // decrement counter  
        "bne 2b\n"                          // repeat until zero 
        
        //   gpio_put(LED_PIN, 1);
        ///  gpioc_bit_out_put(LED_PIN, 1);
        "movs r4, #0x10\n"                  // GPIO16
        "movs r5, #0x00\n"                  // bit 0; used for OUT/OE writes
        "mcrr p0, #4, r4, r5, c0\n"         // gpioc_bit_out_put(16, 0) 
        //   sleep_ms(500);
        ///  sleep_us(500 * 1000ull);
        "ldr r2, =0x17D7840\n"              // r2 = ~8.4M cycles 
        "3:\n"                              // delay loop  
        "subs r2, r2, #1\n"                 // decrement counter 
        "bne 3b\n"                          // repeat until zero

        //    jmp
        "b 1b\n"                            // repeat forever  
    );  
}

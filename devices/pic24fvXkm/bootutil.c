void initOsc(void){
    CLKDIV = 0;
    
    /* Disable nested interrupts */
    INTCON1bits.NSTDIS = 1;

    return;
}
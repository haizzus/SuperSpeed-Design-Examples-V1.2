The source code of the CPLD projects is included, each in a separate folder.

All projects use the common CPLDBoard.ucf file that is in the CPLD Projects folder

The Xilinx tools generate a great deal of other files and I have not included these (to save download time)

To use the I2C circuitry in the CPLD use: I2C_Slave.xsvf, I2C_Slave_And_SPI.xsvf or Counter1_I2C_Slave.xsvf
To use the SPI circuitry in the CPLD use: I2C_Slave_And_SPI.xsvf
In Chapter 9, page 193 for FifoSlave use: SlaveFIFO.xsvf (I should have called it CPLDasFifoMaster.xsvf in the book, sorry)
In Chapter 9, page 206, for FifoMaster use: FIFO_SLAVE.xsvf (I should have called it CPLDasFifoSlave.xsvf in the book, sorry)
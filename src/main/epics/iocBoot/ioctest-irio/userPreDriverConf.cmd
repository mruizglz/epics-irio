############################################################################
## User provided PLC or fast controller driver pre configuration
############################################################################

# Enable parallel callback threads to improve 'I/O Intr' record scanning
#    see https://bugzilla.iter.org/codac/show_bug.cgi?id=10413
callbackParallelThreads
nirioinit("RIO_0","0x0177A2AD","PXIe-7966R","FlexRIOMod5761_7966v1_1","V1.1",1)
#- End-of-file marker - do not delete or add lines below!
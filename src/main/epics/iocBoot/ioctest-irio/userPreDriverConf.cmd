############################################################################
## User provided PLC or fast controller driver pre configuration
############################################################################

# Enable parallel callback threads to improve 'I/O Intr' record scanning
#    see https://bugzilla.iter.org/codac/show_bug.cgi?id=10413
callbackParallelThreads
asynSetTraceMask RIO_0 -1 0x08
nirioinit("RIO_0","0x01A34CC7","PXIe-7966R","FlexRIOMod5761_7966v1_1","V1.1",1)
asynSetTraceMask RIO_1 -1 0x08
nirioinit("RIO_1","0x0177A2AD","PXIe-7966R","FlexRIOMod5761_7966v1_1","V1.1",1)
#- End-of-file marker - do not delete or add lines below!

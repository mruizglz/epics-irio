############################################################################
## DB loading
############################################################################

#cd "$(TOP)/db"
#dbLoadRecords("xxx.db","user=ruizmHost")
#dbLoadTemplate(xxx.substitutions)

# PLC communication 
#cd "$(EPICS_ROOT)/db"
#dbLoadRecords("s7plccom.db", "CBS1=XXXX, CBS2=XXXX, CTRLTYPE=P, IDX=XX, IOCTYPE=XXXX, FOOTER_OFFSET=XXXX")
#dbLoadRecords("s7plccmdcom.db", "CBS1=XXXX, CBS2=XXXX, CTRLTYPE=P, IDX=XX, IOCTYPE=XXXX, FOOTER_OFFSET=XXXX")

# IOC monitor
#cd "$(EPICS_ROOT)/db"
# The macro CTRLTYPE identifies the controller; P for (PLC), H for PSH and F for PCF and S for SERVER
# The macro IDX is the controller's index
# The macro IOCTYPE is IOCtype; CORE, SYSM, PLC
# Uncomment below statement if iocmon feature is required and substitue XXXX with proper values
#dbLoadRecords("iocmon.db","CBS1=XXXX, CBS2=XXXX, CTRLTYPE=X, IDX=X, IOCTYPE=XXXX")

cd $(TOP)/iocBoot/$(IOC)
dbLoadTemplate("PCF0-rio-module.substitution")
dbLoadTemplate("PCF0-rio-module2.substitution")
dbLoadTemplate("PCF0-rio-AO.substitution")
dbLoadTemplate("PCF0-rio-data-acq.substitution")
dbLoadTemplate("PCF0-rio-sg.substitution")
dbLoadTemplate("PCF0-rio-auxAI.substitution")
dbLoadTemplate("PCF0-rio-auxAO.substitution")
dbLoadTemplate("PCF0-rio-auxDI.substitution")
dbLoadTemplate("PCF0-rio-auxDO.substitution")
dbLoadTemplate("PCF0-rio-wf.substitution")

#- End-of-file marker - do not delete or add lines below!

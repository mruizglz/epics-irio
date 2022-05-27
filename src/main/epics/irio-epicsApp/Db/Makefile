TOP=../..
include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE

#----------------------------------------------------
#  Optimization of db files using dbst (DEFAULT: NO)
#DB_OPT = YES

#----------------------------------------------------
# Create and install (or just install)
# databases, templates, substitutions like this
DB += rio-ai.template
DB += rio-ao.template
DB += rio-di.template
DB += rio-do.template
DB += rio-auxai.template
DB += rio-auxao.template
DB += rio-auxdi.template
DB += rio-auxdo.template
DB += rio-module.template
DB += rio-dataacquisition.template
DB += rio-sg.template
DB += rio-wf.template
DB += rio-pointbypoint.template
DB += rio-imagedataacquisition.template
DB += rio-imagewf.template 
#----------------------------------------------------
# If <anyname>.db template is not named <anyname>*.template add
# <anyname>_TEMPLATE = <templatename>

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

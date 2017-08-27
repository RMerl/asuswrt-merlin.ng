#!/usr/bin/env python 
# -*- Mode: python -*-
#======================================================================
# FILE: Gauge.py
# CREATOR: mtearle 
#
# DESCRIPTION:
#   
#
#  $Id: Gauge.py,v 1.2 2002-07-08 17:56:11 acampi Exp $
#  $Locker:  $
#
# (C) COPYRIGHT 2001, Eric Busboom <eric@softwarestudio.org>
# (C) COPYRIGHT 2001, Patrick Lewis <plewis@inetarena.com>  
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of either: 
#
#    The LGPL as published by the Free Software Foundation, version
#    2.1, available at: http://www.fsf.org/copyleft/lesser.html
#
#  Or:
#
#    The Mozilla Public License Version 1.0. You may obtain a copy of
#    the License at http://www.mozilla.org/MPL/
#======================================================================

from LibicalWrap import *
from Error import LibicalError
from Component import Component

class Gauge:
    """ 
    Base class for gauge
    """

    class ConstructorFailedError(LibicalError):
        "Failed to create a Guage "
    
    class CloneFailedError(LibicalError):
        "Failed to clone a component given Gauge "
    
    class CompareFailedError(LibicalError):
        "Failed to compare a component given Gauge "

    def __init__(self,ref=None,sql=None,expand=0):
	if ref != None:
	    self._ref = ref
	elif sql != None:
            s = str(sql)
	    self._ref = icalgauge_new_from_sql(s,expand)
	else:
	    Gauge.ConstructorFailedError("No SQL Specified")

    def __del__(self):
	if self._ref != None:
	    icalgauge_free(self._ref)
	    self._ref = None

    def ref(self):
	return self._ref

    def compare(self, comp):
	if not isinstance(comp,Component):
	    raise Gauge.CompareFailedError("Argument is not a component")

	if comp.ref() == None:
	    raise Gauge.CompareFailedError("Argument is not a component")

	return icalgauge_compare(self._ref, comp.ref())

    # Pending Implementation
    #def as_sql_string(self):
    #	return self.__str__()

    #def __str__(self):
    #	return icalgauge_as_sql(self._ref)

    #def clone(self, comp):
#	if not isinstance(comp,Component):
#	    raise Gauge.CloneFailedError("Argument is not a component")
#
#        comp_ref = icalgauge_new_clone(self._ref, comp)
#
#	if comp_ref == None:
#	    return None
#
#	return Component(ref=comp_ref)
